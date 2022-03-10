#include "NSWConfiguration/hw/TP.h"

#include <stdexcept>

nsw::hw::TP::TP(OpcManager& manager, const TPConfig& config) :
  m_opcManager{manager},
  m_config(config),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress())
{}

void nsw::hw::TP::writeConfiguration() const
{
  throw std::logic_error("Not implemented");
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::TP::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::TP::writeRegister([[maybe_unused]] const std::uint8_t regAddress,
                                [[maybe_unused]] const std::uint32_t value) const
{
  throw std::logic_error("Not implemented");
}

std::vector<std::uint8_t> nsw::hw::TP::readRegister(
  [[maybe_unused]] const std::uint8_t regAddress) const
{
  throw std::logic_error("Not implemented");
}
