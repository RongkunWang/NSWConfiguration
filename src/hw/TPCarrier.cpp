#include "NSWConfiguration/hw/TPCarrier.h"

#include <stdexcept>

nsw::hw::TPCarrier::TPCarrier(const TPCarrierConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{}

void nsw::hw::TPCarrier::writeConfiguration() const
{
  throw std::logic_error("Not implemented");
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::TPCarrier::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::TPCarrier::writeRegister([[maybe_unused]] const std::uint8_t regAddress,
                                       [[maybe_unused]] const std::uint32_t value) const
{
  throw std::logic_error("Not implemented");
}

std::vector<std::uint8_t> nsw::hw::TPCarrier::readRegister(
  [[maybe_unused]] const std::uint8_t regAddress) const
{
  throw std::logic_error("Not implemented");
}
