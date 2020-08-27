#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/RouterConfig.h"

#include "boost/property_tree/json_parser.hpp"

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
  // Router_XYY_LZ -> X
  auto addr = getAddress();
  auto result = addr.substr(7, 1);
  if (result != "A" && result != "C")
    id_crash();
  return result == "A" ? 0 : 1;
}

uint8_t nsw::RouterConfig::id_sector() const {
  // Router_XYY_LZ -> YY
  auto addr = getAddress();
  auto result = std::stoi(addr.substr(8, 2));
  if (result < 1 || result > 16)
    id_crash();
  return (uint8_t)(result - 1);
}

uint8_t nsw::RouterConfig::id_layer() const {
  // Router_XYY_LZ -> Z
  auto addr = getAddress();
  auto result = std::stoi(addr.substr(12, 1));
  if (result < 0 || result > 7)
    id_crash();
  return (uint8_t)(result);
}

void nsw::RouterConfig::id_check() const {
  auto addr = getAddress();
  if (addr.length() != convention.length())
    id_crash();
  if (addr.substr(0, 7) != "Router_")
    id_crash();
  if (addr.substr(11, 1) != "L")
    id_crash();
}

void nsw::RouterConfig::id_crash() const {
  auto addr = getAddress();
  auto msg = name_error + " (" + convention + "): " + addr;
  throw std::runtime_error(msg);
}

void nsw::RouterConfig::dump() {
    // std::cout << std::endl;
}

