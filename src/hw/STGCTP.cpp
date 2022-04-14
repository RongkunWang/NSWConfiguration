#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::STGCTP::STGCTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_scaAddressFPGA(fmt::format("{}.I2C_0.bus0", getScaAddress())),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress()))
{
}

void nsw::hw::STGCTP::writeConfiguration() const
{
  writeAndReadbackRegister(nsw::stgctp::REG_SECTOR, getSector());
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::STGCTP::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  for (const auto& [name, reg]: nsw::stgctp::REGS) {
    result.emplace(reg, readRegister(reg));
  }
  return result;
}

void nsw::hw::STGCTP::writeRegister(const std::uint32_t regAddress,
                                    const std::uint32_t value) const
{
  nsw::hw::SCAX::writeRegister(getConnection(), m_scaAddressFPGA, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::uint32_t regAddress) const
{
  return nsw::hw::SCAX::readRegister(getConnection(), m_scaAddressFPGA, regAddress);
}

void nsw::hw::STGCTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                               const std::uint32_t value) const
{
  ERS_LOG(fmt::format("{}: writing to {:#02x} with {:#08x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), m_scaAddressFPGA, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::getSector() const
{
  return m_config.get<std::uint32_t>("sector");
}
