#include "NSWConfiguration/hw/ART.h"

#include <stdexcept>

nsw::hw::ART::ART(const nsw::ADDCConfig& config, const std::size_t numArt) :
  m_config(config.getART(numArt)),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_numArt(numArt)
{}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::ART::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::ART::writeConfiguration() const
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::ART::writeRegister([[maybe_unused]] const std::uint8_t regAddress,
                                 [[maybe_unused]] const std::uint32_t value) const
{
  throw std::logic_error("Not implemented");
}

std::vector<std::uint8_t> nsw::hw::ART::readRegister(
  [[maybe_unused]] const std::uint8_t regAddress) const
{
  throw std::logic_error("Not implemented");
}
