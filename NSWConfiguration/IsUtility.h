#ifndef NSWCONFIGURATION_ISUTILITY_H
#define NSWCONFIGURATION_ISUTILITY_H

#include <string>
#include <string_view>

namespace nsw {
  /**
   * \brief Build the key to publish the SCA availability status for a given sector on IS
   *
   * \param db IS DB server name
   * \param sector Sector ID
   * \return std::string IS key
   */
  [[nodiscard]] std::string buildScaAvailableKey(std::string_view db, std::string_view sector);
}  // namespace nsw

#endif  // NSWCONFIGURATION_UTILITY_H
