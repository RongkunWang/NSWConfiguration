#ifndef NSWCONFIGURATION_RCUTILITY_H
#define NSWCONFIGURATION_RCUTILITY_H

#include <string>

namespace nsw {
  /**
   * \brief Find an application with a given class name within the same segment as the app it is
   * called from
   *
   * \param className Class name of the app to be found
   * \return std::string Name of the application
   */
  [[nodiscard]] std::string findSegmentSiblingApp(const std::string& className);

  /**
   * @brief Extract a unique detector ID from an application name
   *
   * App name should have the format NSW-MMG-C-S01-SCAService. Everything after the last dash is
   * dropped
   *
   * @param appName Name of the application
   * @return std::string Unique sector ID
   */
  std::string extractSectorIdFromApp(const std::string& appName);
}  // namespace nsw

#endif  // NSWCONFIGURATION_UTILITY_H
