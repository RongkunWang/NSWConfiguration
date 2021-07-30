#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/Constants.h"

#include <stdexcept>

using boost::property_tree::ptree;

nsw::RouterConfig::RouterConfig(const ptree& config):
    SCAConfig(config)
{
    // std::cout << std::endl;
}

bool nsw::RouterConfig::CrashOnClkReadyFailure() const {
    return m_config.get<bool>("CrashOnClkReadyFailure");
}

bool nsw::RouterConfig::CrashOnConfigFailure() const {
    return m_config.get<bool>("CrashOnConfigFailure");
}

std::string nsw::RouterConfig::Sector() const {
    return m_config.get<std::string>("Sector");
}

uint8_t nsw::RouterConfig::id() const {
  //
  // Convert name to 8-bit ID
  // 4 bits: Sector (0x0 - 0xf)
  // 3 bits: Layer (0x0 - 0x7)
  // 1 bits: Endcap (A=0, C=1)
  // NB: "Sector" = Sector-1,
  //     since ATLAS sectors 01-16
  //
  // e.g. Router_A14_L5
  // 4 bits: 14-1 = 13 = 0b1101
  // 3 bits: L5   =  5 = 0b101
  // 1 bits: A    =  0 = 0b0
  // (13 << 4) + (5 << 1) + 1 = 218
  //
  id_check();
  return (id_sector() << 4) + (id_layer() << 1) + id_endcap();
}

uint8_t nsw::RouterConfig::id_endcap() const {
  // XYY -> X
  // A12 -> A, e.g.
  auto sect = Sector();
  auto result = sect.substr(0, 1);
  if (result != "A" && result != "C")
    id_crash();
  return result == "A" ? 0 : 1;
}

uint8_t nsw::RouterConfig::id_sector() const {
  // XYY -> YY
  // A12 -> 12, e.g.
  auto sect = Sector();
  const auto result = std::stoul(sect.substr(1, 2));
  if (result < nsw::MIN_SECTOR_ID || result > nsw::MAX_SECTOR_ID)
    id_crash();
  return static_cast<uint8_t>(result - 1);
}

uint8_t nsw::RouterConfig::id_layer() const {
  // Router_LZ -> Z
  auto addr = getAddress();
  const auto result = std::stoul(addr.substr(8, 1));
  if (result < nsw::MIN_LAYER_ID || result > nsw::MAX_LAYER_ID)
    id_crash();
  return static_cast<uint8_t>(result);
}

void nsw::RouterConfig::id_check() const {
  auto addr = getAddress();
  if (addr.length() != convention.length())
    id_crash();
  if (addr.substr(0, 7) != "Router_")
    id_crash();
  if (addr.substr(7, 1) != "L")
    id_crash();
}

void nsw::RouterConfig::id_crash() const {
  auto addr = getAddress();
  auto msg = name_error + " (" + convention + "): " + addr;
  throw std::runtime_error(msg);
}

void nsw::RouterConfig::dump() const {
    // std::cout << std::endl;
}

