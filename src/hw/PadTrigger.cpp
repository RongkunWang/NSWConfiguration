#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <ers/ers.h>
#include <fmt/core.h>
#include <stdexcept>

nsw::hw::PadTrigger::PadTrigger(const PadTriggerSCAConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{
  m_name = fmt::format("{}/{}", m_opcserverIp, m_scaAddress);
  m_scaAddressFPGA = fmt::format("{}.fpga.fpga", m_scaAddress);
}

void nsw::hw::PadTrigger::writeConfiguration() const
{
  writeRepeatersConfiguration();
  writeVTTxConfiguration();
  writeFPGAConfiguration();
}

void nsw::hw::PadTrigger::writeRepeatersConfiguration() const
{
  if (not m_config.ConfigRepeaters()) {
    ERS_LOG(fmt::format("Skipping configuration of repeaters of {}", name()));
    return;
  }

  for (const auto& [rep, address, value]: m_repeaterSequenceOfCommands) {

    const auto gpio = fmt::format("gpio-repeaterChip{}", rep);

    // GPIO enable
    writeGPIO(gpio, true);

    // i2c write
    ERS_LOG(fmt::format("Repeater{} address {}: write {}", rep, address, value));
    writeRepeaterRegister(rep, address, value);

    // i2c readback
    const auto val = readRepeaterRegister(rep, address);
    ERS_LOG(fmt::format("Repeater{} address {}: readback {}", rep, address, val));

    // check and complain
    if (val != value) {
      const std::string msg = "Found mismatch in repeater readback";
      nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
      ers::error(issue);
    }

    // GPIO disable
    writeGPIO(gpio, false);
  }
}

void nsw::hw::PadTrigger::writeVTTxConfiguration() const
{
  if (not m_config.ConfigVTTx()) {
    ERS_LOG(fmt::format("Skipping configuration of VTTx of {}", name()));
    return;
  }

  for (const auto& vttx: m_vttxs) {

    for (const auto& vttx_data: m_vttx_datas) {

      // write
      ERS_LOG(fmt::format("VTTx{} address {}: write {}", vttx, m_vttx_addr, vttx_data));
      writeVTTxRegister(vttx, m_vttx_addr, vttx_data);

      // readback
      const auto val = readVTTxRegister(vttx, m_vttx_addr);
      ERS_LOG(fmt::format("VTTx{} address {}: readback {}", vttx, m_vttx_addr, val));

      // check and complain
      if (vttx_data != val) {
        const std::string msg = "Found mismatch in VTTx readback";
        nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
        ers::error(issue);
      }

    }

  }

}

void nsw::hw::PadTrigger::writeFPGAConfiguration() const
{
  const auto& fpga = m_config.getFpga();
  const auto& fw   = m_config.firmware_dateword();
  ERS_INFO(fmt::format("Firmware provided: {}", fw));

  for (const auto& [rname, value_str] : fpga.getBitstreamMap()) {

    // strings -> numerics
    const auto addr  = addressFromRegisterName(rname);
    const auto value = std::stoul(value_str, nullptr, nsw::BASE_BIN);

    // backwards compatibility
    if (addr > nsw::padtrigger::REG_CONTROL and fw < nsw::padtrigger::DATE_REGS_2021_10) {
      ERS_INFO(fmt::format("Older firmware, will not configure {}", rname));
      continue;
    }

    // write
    ERS_INFO(fmt::format("{}: writing to {:#02x} with {:#08x}", name(), addr, value));
    writeFPGARegister(addr, value);

    // readback
    const auto val = readFPGARegister(addr);
    ERS_INFO(fmt::format("{}: readback of {:#02x} gives {:#08x}", name(), addr, val));

    // compare
    if (val != value) {
      const std::string msg = "Found mismatch in FPGA reg readback";
      nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
      ers::error(issue);
    }

  }

}

std::map<std::uint8_t, std::uint32_t> nsw::hw::PadTrigger::readConfiguration() const
{
  std::map<std::uint8_t, std::uint32_t> result;
  result.emplace(nsw::padtrigger::REG_CONTROL, readFPGARegister(nsw::padtrigger::REG_CONTROL));
  return result;
}

void nsw::hw::PadTrigger::writeGPIO(const std::string& name, const bool value) const
{
  const auto addr = fmt::format("{}.{}.{}", m_scaAddress, "gpio", name);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendGPIO(opcConnection, addr, value);
}

void nsw::hw::PadTrigger::writeRepeaterRegister(const std::uint8_t repeater,
                                                const std::uint8_t regAddress,
                                                const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2c(opcConnection, addr, data);
}

void nsw::hw::PadTrigger::writeVTTxRegister(const std::uint8_t vttx,
                                            const std::uint8_t regAddress,
                                            const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "vttx", vttx,
                                "vttx", vttx);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2c(opcConnection, addr, data);
}

void nsw::hw::PadTrigger::writeFPGARegister(const std::uint8_t regAddress,
                                            const std::uint32_t value) const
{
  const std::vector<uint8_t> addr = { regAddress };
  const auto data = nsw::intToByteVector(value, nsw::NUM_BYTES_IN_WORD32, nsw::padtrigger::SCA_LITTLE_ENDIAN);

  std::vector<uint8_t> payload;
  payload.reserve(addr.size() + data.size());
  payload.insert(std::end(payload), std::begin(addr), std::end(addr));
  payload.insert(std::end(payload), std::begin(data), std::end(data));

  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2cRaw(opcConnection, m_scaAddressFPGA, payload.data(), payload.size());
}

bool nsw::hw::PadTrigger::readGPIO(const std::string& name) const
{
  const auto addr = fmt::format("{}.{}.{}", m_scaAddress, "gpio", name);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  return nsw::hw::SCA::readGPIO(opcConnection, addr);
}

std::uint8_t nsw::hw::PadTrigger::readRepeaterRegister(const std::uint8_t repeater,
                                                       const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto val = nsw::hw::SCA::readI2cAtAddress(opcConnection, addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint8_t nsw::hw::PadTrigger::readVTTxRegister(const std::uint8_t vttx,
                                                   const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "vttx", vttx,
                                "vttx", vttx);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto val = nsw::hw::SCA::readI2cAtAddress(opcConnection, addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint32_t nsw::hw::PadTrigger::readFPGARegister(const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = { regAddress };
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto value = nsw::hw::SCA::readI2cAtAddress(opcConnection, m_scaAddressFPGA,
                                                    data.data(), data.size(),
                                                    nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(value, nsw::padtrigger::SCA_LITTLE_ENDIAN);
}

std::uint8_t nsw::hw::PadTrigger::addressFromRegisterName(const std::string& name) const
{
  std::uint8_t addr{0};
  try {
    const auto addr_str = name.substr(std::size_t{0}, name.find("_"));
    addr = std::stoul(addr_str, nullptr, nsw::BASE_HEX);
  } catch (std::exception & ex) {
    const auto msg = fmt::format("Cannot get address from: {}", name);
    nsw::PadTriggerConfigError issue(ERS_HERE, msg.c_str());
    ers::error(issue);
    throw issue;
  }
  return addr;
}
