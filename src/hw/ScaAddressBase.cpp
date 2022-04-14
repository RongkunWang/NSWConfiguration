#include "NSWConfiguration/hw/ScaAddressBase.h"

nsw::hw::ScaAddressBase::ScaAddressBase(std::string scaAddress) :
  m_scaAddress{std::move(scaAddress)},
  m_filenameCompatibleGeoId{replaceSlashes(m_scaAddress)},
  m_geoIdInfo{m_scaAddress}
{}

std::string nsw::hw::ScaAddressBase::replaceSlashes(const std::string& str)
{
  constexpr static char SLASH = '/';
  constexpr static char UNDERSCORE = '_';
  auto newStr{str};
  std::replace(std::begin(newStr), std::end(newStr), SLASH, UNDERSCORE);
  return newStr;
}
