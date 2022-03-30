#include "NSWConfiguration/hw/TPCarrier.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>

nsw::hw::TPCarrier::TPCarrier(OpcManager& manager, const TPCarrierConfig& config):
  m_opcManager{manager},
  m_config(config),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_busAddress(fmt::format("{}.I2C_0.bus0", m_scaAddress)),
  m_name(fmt::format("{}/{}", m_opcserverIp, m_scaAddress))
{
}

void nsw::hw::TPCarrier::writeConfiguration() const
{
  const auto& regs = nsw::carrier::REGS;
  writeAndReadbackRegister(regs.at("RJOUT_SEL"), m_config.RJOutSel());
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::TPCarrier::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  for (const auto& [name, reg]: nsw::carrier::REGS) {
    result.emplace(reg, readRegister(reg));
  }
  return result;
}

void nsw::hw::TPCarrier::writeRegister(const std::uint32_t regAddress,
                                       const std::uint32_t value) const
{
  nsw::hw::SCAX::writeRegister(getConnection(), m_busAddress, regAddress, value);
}

std::uint32_t nsw::hw::TPCarrier::readRegister(const std::uint32_t regAddress) const
{
  return nsw::hw::SCAX::readRegister(getConnection(), m_busAddress, regAddress);
}

void nsw::hw::TPCarrier::writeAndReadbackRegister(const std::uint32_t regAddress,
                                                  const std::uint32_t value) const
{
  ERS_LOG(fmt::format("{}: writing to address 0x{:02x} with message 0x{:08x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), m_busAddress, regAddress, value);
}
