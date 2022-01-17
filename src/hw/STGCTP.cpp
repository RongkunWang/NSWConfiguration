#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::STGCTP::STGCTP(const boost::property_tree::ptree& config):
  m_config(config),
  m_opcserverIp(config.get<std::string>("OpcServerIp")),
  m_scaAddress(config.get<std::string>("OpcNodeId")),
  m_name(fmt::format("{}/{}", m_opcserverIp, m_scaAddress))
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
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCAX::writeRegister(opcConnection, m_scaAddress, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::uint32_t regAddress) const
{
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  return nsw::hw::SCAX::readRegister(opcConnection, m_scaAddress, regAddress);
}

void nsw::hw::STGCTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                               const std::uint32_t value) const
{
  ERS_LOG(fmt::format("{}: writing to {:#02x} with {:#08x}", m_name, regAddress, value));
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCAX::writeAndReadbackRegister(opcConnection, m_scaAddress, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::getSector() const
{
  return m_config.get<std::uint32_t>("sector");
}
