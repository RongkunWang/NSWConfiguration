#include "NSWConfiguration/hw/TPCarrier.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>

nsw::hw::TPCarrier::TPCarrier(OpcManager& manager, const boost::property_tree::ptree& config) :
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_busAddress(fmt::format("{}.I2C_0.bus0", getScaAddress())),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress()))
{
}

void nsw::hw::TPCarrier::writeConfiguration() const
{
  const auto& regs = nsw::carrier::REGS;
  // FIXME
  writeRegister("RJOUT_SEL", RJOutSel());
  // writeAndReadbackRegister(regs.at("gbtPhaseAdjust"), m_config.get("gbtPhaseAdjust", 7)); // TODO: wait for Nathan to find the best value
}

std::map<std::string_view, std::uint32_t> nsw::hw::TPCarrier::readConfiguration() const
{
  std::map<std::string_view, std::uint32_t> result;
  for (const auto& reg: nsw::carrier::REGS) {
    if (reg == "FIXME_gbtPhaseAdjust") {
      continue;
    }
    result.emplace(reg, readRegister(reg));
  }
  return result;
}

void nsw::hw::TPCarrier::writeRegister(std::string_view regAddress,
                                       const std::uint32_t value) const
{
  nsw::hw::SCAX::writeRegister(getConnection(),
      regAddress,
      value);
}

std::uint32_t nsw::hw::TPCarrier::readRegister(std::string_view regAddress) const
{
  return nsw::hw::SCAX::readRegister(getConnection(),
      regAddress
      );
}

void nsw::hw::TPCarrier::writeAndReadbackRegister(std::string_view regAddress,
                                                  const std::uint32_t value) const
{
  ERS_LOG(fmt::format("{}: writing to address {} with message 0x{:08x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(),
      regAddress,
      value);
}

std::uint32_t nsw::hw::TPCarrier::RJOutSel() const {
  if (m_config.get("RJOutSelSTGC", false)) {
    return nsw::carrier::RJOUT_SEL_STGC;
  } else if (m_config.get("RJOutSelMM", false)) {
    return nsw::carrier::RJOUT_SEL_MM;
  } else {
    return m_config.get("RJOutSel", nsw::carrier::RJOUT_SEL_STGC);
  }
}
