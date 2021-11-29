#include "NSWConfiguration/hw/GBTx.h"

#include <algorithm>
#include <iterator>
#include <thread>

#include <fmt/core.h>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/OpcManager.h"

nsw::hw::GBTx::GBTx(const L1DDCConfig& config, const std::size_t gbtxId) :
  m_config(config.getGBTxConfig(gbtxId)),
  m_gbtxId(gbtxId),
  m_portIn(config.getPortToGbtx()),
  m_portOut(config.getPortFromGbtx()),
  m_elinkId(config.getElinkId()),
  m_felixIp(config.getFelixServerIp()),
  m_opcserverIp(config.getOpcServerIp()),
  m_opcNodeId(fmt::format("{0}.gbtx{1}.gbtx{1}", config.getOpcNodeId(), gbtxId + 1)),
  m_chunkSize(static_cast<std::size_t>(config.i2cBlockSize())),
  m_delay(std::chrono::microseconds{config.i2cDelay()}),
  m_name(config.getName())
{
  if (gbtxId > 2) {
    nsw::NSWGBTxIssue issue(
      ERS_HERE, fmt::format("Only 3 GBTx's exist. You requested to configure {}", gbtxId));
    ers::error(issue);
    throw issue;
  }
}

void nsw::hw::GBTx::writeConfiguration() const
{
  writeConfiguration(m_config);
}

std::vector<std::uint8_t> nsw::hw::GBTx::readConfiguration() const
{
  if (m_config.getMode() == nsw::GBTxConfig::Mode::IC) {
    return readIcConfiguration();
  }
  return readI2cConfiguration();
}

bool nsw::hw::GBTx::confirmConfiguration(const GBTxConfig& config) const
{
  const auto data = readConfiguration();
  const auto reference = config.configAsVector();
  ERS_DEBUG(2, "\n==> Configuration, read from GBTx, after uploading new configuration:");
  ERS_DEBUG(2, nsw::getPrintableGbtxConfig(data) << '\n');

  // Check readback
  if (reference.size() != data.size()) {
    ERS_LOG("WARNING: readback size not expected");
    ERS_LOG("expected=" << reference.size() << " recieved=" << data.size() << '\n');
    return false;
  }

  // Some of the readback registers are not writable registers. Only check the writable registers
  constexpr std::size_t maxConfigRegister = 365;
  const auto size = static_cast<long>(std::min(std::size(reference), maxConfigRegister));

  return std::equal(std::cbegin(reference),
                    std::cbegin(reference) + size,
                    std::cbegin(data),
                    std::cbegin(data) + size);
}

void nsw::hw::GBTx::train() const
{
  auto trainConfig(m_config);
  trainConfig.setResetChannelsOn();
  trainConfig.setTrainingRegistersOn();
  writeConfiguration(trainConfig);
  std::this_thread::sleep_for(nsw::l1ddc::GBTX_TRAINING_TIME);
  writeConfiguration(m_config);
}

void nsw::hw::GBTx::writeConfiguration(const GBTxConfig& config) const
{
  const auto data = config.configAsVector();

  // Try sending configuration and check the readback
  // If the readback doesn't match, for nTries, raise error
  auto nTries = MAX_ATTEMPTS;
  while (nTries > 0) {
    if (m_config.getMode() == nsw::GBTxConfig::Mode::IC) {
      writeIcConfiguration(data);
    } else {
      writeI2cConfiguration(data);
    }
    if (confirmConfiguration(config)) {
      return;
    }
    nTries--;
    ERS_LOG("Retrying configuration. Remaining tries: " << nTries << " for " << m_name);
  }
  nsw::NSWGBTxIssue issue(
    ERS_HERE,
    fmt::format(
      "Unable to read back correct configuration within {} tries for {}", MAX_ATTEMPTS, m_name));
  ers::error(issue);
  throw issue;
}

void nsw::hw::GBTx::writeIcConfiguration(const std::vector<std::uint8_t>& data) const
{
  IChandler ich(m_felixIp,
                static_cast<unsigned int>(m_portIn),
                static_cast<unsigned int>(m_portOut),
                static_cast<unsigned int>(m_elinkId));
  ich.sendCfg(data);
}

std::vector<std::uint8_t> nsw::hw::GBTx::readIcConfiguration() const
{
  IChandler ich(m_felixIp,
                static_cast<unsigned int>(m_portIn),
                static_cast<unsigned int>(m_portOut),
                static_cast<unsigned int>(m_elinkId));

  auto data = ich.readCfg();

  // TODO: Remove this once fixed upstream
  constexpr int NBYTES = 1;
  std::rotate(std::rbegin(data), std::rbegin(data) + NBYTES, std::rend(data));  // rotate right
  std::fill_n(std::begin(data), NBYTES, 0);                                     // zero first bytes

  return data;
}

void nsw::hw::GBTx::writeI2cConfiguration(const std::vector<std::uint8_t>& data) const
{
  const auto& opcConnection = nsw::OpcManager::getConnection(m_opcserverIp);
  std::vector<uint8_t> frame{};
  frame.reserve(3);
  for (std::size_t reg = 0; reg < data.size(); reg++) {
    if (frame.empty()) {
      frame.push_back(static_cast<std::uint8_t>(reg & 0xffU));
      frame.push_back(static_cast<std::uint8_t>(reg >> 8U));
    }
    frame.push_back(data.at(reg));
    if (frame.size() == m_chunkSize || reg + 1 == data.size()) {
      SCA::sendI2c(opcConnection, m_opcNodeId, frame);
      nsw::snooze(m_delay);
      frame.clear();
    }
  }
}

std::vector<std::uint8_t> nsw::hw::GBTx::readI2cConfiguration() const
{
  std::vector<uint8_t> data{};
  const std::size_t nChunks = (nsw::NUM_GBTX_WRITABLE_REGISTERS + m_chunkSize) / m_chunkSize;
  const auto& opcConnection = nsw::OpcManager::getConnection(m_opcserverIp);
  for (std::size_t reg = 0; reg < nChunks * m_chunkSize; reg += m_chunkSize) {
    // Set address to read
    const std::array<std::uint8_t, 2> address{static_cast<std::uint8_t>(reg & 0xffU),
                                              static_cast<std::uint8_t>(reg >> 8U)};
    const std::size_t nBytesRead = (reg + m_chunkSize > nsw::NUM_GBTX_WRITABLE_REGISTERS)
                                     ? nsw::NUM_GBTX_WRITABLE_REGISTERS - reg
                                     : m_chunkSize;
    const auto readback = SCA::readI2cAtAddress(
      opcConnection, m_opcNodeId, address.data(), address.size(), m_delay, nBytesRead);
    nsw::snooze(m_delay);
    data.insert(std::end(data), std::cbegin(readback), std::cend(readback));
  }
  return data;
}
