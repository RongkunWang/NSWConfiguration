#ifndef NSWCONFIGURATION_HW_SCASTATUS_H
#define NSWCONFIGURATION_HW_SCASTATUS_H

#include <stdexcept>
#include <string>

namespace nsw::hw::ScaStatus {
  enum ScaStatus {
    REACHABLE,
    UNREACHABLE,
    SERVER_OFFLINE,
  };

  inline std::string getRepresentation(ScaStatus status)
  {
    switch (status) {
    case REACHABLE:
      return "reachable";
    case UNREACHABLE:
      return "SCA unreachable";
    case SERVER_OFFLINE:
      return "OPC server offline";
    default:
      throw std::logic_error("Unknown option for enum ScaStatus");
    }
  }
}  // namespace nsw::hw::ScaStatus

#endif