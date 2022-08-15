#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"
using namespace std::chrono_literals;

nsw::hw::STGCTP::STGCTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_scaAddressFPGA(fmt::format("{}.I2C_0.bus0", getScaAddress())),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress())),
  m_skippedReg(SkipRegisters())
{
}

void nsw::hw::STGCTP::writeConfiguration() const
{
  if (getDoReset()) {
    ERS_LOG(fmt::format("{}: toggling RX resets", getName()));
    writeRegister(nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_ENABLE);
    nsw::snooze(1s);
    writeRegister(nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_DISABLE);
    nsw::snooze(2s);
    ERS_LOG(fmt::format("{}: toggling TX resets", getName()));
    writeRegister(nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_ENABLE);
    nsw::snooze(1s);
    writeRegister(nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_DISABLE);
    nsw::snooze(2s);
  }
  writeAndReadbackRegister(nsw::stgctp::REG_SECTOR, getSector());
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::STGCTP::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  for (const auto& [reg, mask]: nsw::stgctp::REGS) {
    try {
      result.emplace(reg, (m_skippedReg.contains(reg)) ? nsw::DEADBEEF : readRegister(reg) & mask);
    } catch (std::exception & ex) {
      ERS_INFO(fmt::format("{}: failed to read reg {:#04x}", m_name, reg));
      break;
    }
  }
  return result;
}

void nsw::hw::STGCTP::writeRegister(const std::uint32_t regAddress,
                                    const std::uint32_t value) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {:#04x}", m_name, regAddress));
    return;
  }
  nsw::hw::SCAX::writeRegister(getConnection(), m_scaAddressFPGA, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::uint32_t regAddress) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip reading {:#04x}, return dummy value {:#x}", m_name, regAddress, nsw::DEADBEEF));
    return nsw::DEADBEEF;
  }
  return nsw::hw::SCAX::readRegister(getConnection(), m_scaAddressFPGA, regAddress);
}

void nsw::hw::STGCTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                               const std::uint32_t value) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {:#04x}", m_name, regAddress));
    return;
  }
  ERS_LOG(fmt::format("{}: writing to {:#04x} with {:#010x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), m_scaAddressFPGA, regAddress, value);
}

std::set<std::uint8_t> nsw::hw::STGCTP::SkipRegisters() const
{
  const auto key = "SkipRegisters";
  if (m_config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(m_config, key);
}

std::uint32_t nsw::hw::STGCTP::getSector() const
{
  return m_config.get<std::uint32_t>("sector");
}

bool nsw::hw::STGCTP::getDoReset() const
{
  return m_config.get("DoReset", true);
}
