#include "NSWConfiguration/hw/ADDC.h"

#include <stdexcept>

#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/hw/ART.h"
#include "NSWConfiguration/hw/OpcManager.h"

nsw::hw::ADDC::ADDC(nsw::OpcManager& manager, const nsw::ADDCConfig& config) :
  ScaAddressBase(config.getAddress()),
  OpcConnectionBase(manager, config.getOpcServerIp(), config.getAddress())
{
  for (std::size_t iart = 0; iart < nsw::NUM_ART_PER_ADDC; ++iart) {
    m_arts.emplace_back(manager, config, iart);
  }
}

nsw::hw::ADDC::ADDC(nsw::OpcManager& manager, const boost::property_tree::ptree& config) : ADDC(manager, nsw::ADDCConfig(config)) { }

void nsw::hw::ADDC::writeConfiguration() const {
  ERS_INFO(fmt::format("Configuring ADDC: {}, all ARTs", getScaAddress()));

  for (const auto & art : m_arts) {
    art.writeConfiguration();
  }

  ERS_LOG(fmt::format("{} configuration done.", getScaAddress()));
}

void nsw::hw::ADDC::writeConfiguration(const std::size_t i_art) const {
  ERS_INFO(fmt::format("Configuring ADDC: {}, art index ", getScaAddress(), i_art));

  m_arts.at(i_art).writeConfiguration();

  ERS_LOG(fmt::format("{} configuration done.", getScaAddress()));
}
