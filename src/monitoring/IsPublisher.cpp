#include "NSWConfiguration/monitoring/IsPublisher.h"

#include <fmt/core.h>

std::string nsw::mon::ISPublisher::buildKey(const std::string_view serverName,
                                            const std::string_view groupName,
                                            const std::string_view deviceType,
                                            const std::string_view deviceName)
{
  return fmt::format("{}.{}.{}.{}", serverName, deviceType, groupName, deviceName);
}
