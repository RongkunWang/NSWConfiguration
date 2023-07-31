#include "NSWConfiguration/hw/GBTxI2c.h"

#include <algorithm>
#include <iterator>
#include <thread>
#include <vector>

#include <fmt/core.h>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/OpcManager.h"

nsw::hw::GBTxI2c::GBTxI2c(OpcManager& manager, const L1DDCConfig& config, const std::size_t gbtxId) :
  ScaAddressBase(config.getOpcNodeId()),
  OpcConnectionBase(manager,
                    config.getOpcServerIp(),
                    fmt::format("{0}.gbtx{1}.gbtx{1}", config.getOpcNodeId(), gbtxId + 1)),
  m_config(config.getGBTx(gbtxId)),
  m_name(config.getName()),
  m_gbtxId(gbtxId),
  m_chunkSize(static_cast<std::size_t>(config.i2cBlockSize())),
  m_delay(std::chrono::microseconds{config.i2cDelay()}),
  m_invalid{validateConfig(config, gbtxId)}
{}

void nsw::hw::GBTxI2c::writeConfiguration() const
{
  writeConfiguration(m_config);
}

void nsw::hw::GBTxI2c::writeConfiguration(const GBTxConfig& config) const
{
  if (m_invalid) {
    nsw::NSWGBTxIssue issue(
      ERS_HERE, fmt::format("Attempt to write configuration to invalid GBTx {}", m_name));
    ers::error(issue);
    return;
  }

  const auto data = config.configAsVector();
  m_tracker.update(data);

  // Use std::views::chunk with c++23
  std::vector<std::vector<std::uint8_t>> frames{};
  frames.reserve(std::size(data) / m_chunkSize + 1);
  auto iter = std::cbegin(data);
  for (std::size_t reg = 0; reg != std::size(data);) {
    std::vector<std::uint8_t> frame{
      {static_cast<std::uint8_t>(reg & 0xffU), static_cast<std::uint8_t>(reg >> 8U)}};
    auto const numElements = std::min(std::size(data) - reg, m_chunkSize);
    frame.reserve(numElements + std::size(frame));
    std::copy_n(iter, numElements, std::back_inserter(frame));
    std::advance(iter, numElements);
    reg += numElements;
    frames.push_back(std::move(frame));
  }

  for (const auto& frame : frames) {
    ERS_DEBUG(5, fmt::format("==> SENDING: {:02x}", fmt::join(data, "")));
    SCA::sendI2c(getConnection(), getScaAddress(), frame);
    nsw::snooze(m_delay);
  }
}

std::vector<std::uint8_t> nsw::hw::GBTxI2c::readConfiguration() const
{
  if (m_invalid) {
    nsw::NSWGBTxIssue issue(
      ERS_HERE, fmt::format("Attempt to write configuration to invalid GBTx {}", m_name));
    ers::error(issue);
    return {};
  }

  std::vector<std::uint8_t> data{};
  data.reserve(nsw::NUM_GBTX_WRITABLE_REGISTERS);
  const std::size_t nChunks = (nsw::NUM_GBTX_WRITABLE_REGISTERS) / m_chunkSize + 1;
  for (std::size_t reg = 0; reg < nChunks * m_chunkSize; reg += m_chunkSize) {
    // Set address to read
    const auto address =
      std::to_array({static_cast<std::uint8_t>(reg & 0xffU), static_cast<std::uint8_t>(reg >> 8U)});
    SCA::sendI2cRaw(getConnection(), getScaAddress(), address.data(), address.size());
    nsw::snooze(m_delay);
    // Read back required number of bytes
    const std::size_t nBytesRead = std::min(nsw::NUM_GBTX_WRITABLE_REGISTERS - reg, m_chunkSize);
    const std::vector<uint8_t> readback =
      SCA::readI2c(getConnection(), getScaAddress(), nBytesRead);
    nsw::snooze(m_delay);
    ERS_DEBUG(5, fmt::format("==> RECEIVED: {:02x}", fmt::join(readback, "")));
    data.insert(std::end(data), std::cbegin(readback), std::cend(readback));
  }
  m_tracker.validate(data);
  return data;
}

bool nsw::hw::GBTxI2c::hasConfigurationErrors() const
{
  return not m_tracker.getErrors().empty();
}

bool nsw::hw::GBTxI2c::validateConfig(const L1DDCConfig& config, std::size_t gbtxId)
{
  if (gbtxId >= config.getNumberGBTx()) {
    nsw::NSWGBTxIssue issue(ERS_HERE,
                            fmt::format("Only {} GBTx's exist. You requested to configure {}",
                                        config.getNumberGBTx(),
                                        gbtxId));
    ers::error(issue);
    return false;
  }
  constexpr static std::size_t GBTX_ID_IC{0};
  if (gbtxId == GBTX_ID_IC) {
    nsw::NSWGBTxIssue issue(ERS_HERE,
                            fmt::format("GBTx {} cannot be configured through I2C", GBTX_ID_IC));
    ers::error(issue);
    return false;
  }
  return true;
}
