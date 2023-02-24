#include "NSWConfiguration/IsUtility.h"

#include <string_view>

#include <fmt/core.h>

std::string nsw::buildScaAvailableKey(const std::string_view db, const std::string_view sector)
{
  constexpr static std::string_view KEY{"scaAvailable"};
  return fmt::format("{}.{}.{}", db, sector, KEY);
}
