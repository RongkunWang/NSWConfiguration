#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_HELPERS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_HELPERS_H

#include <execution>

#include <is/infodictionary.h>

#include "NSWConfiguration/Concepts.h"
#include "NSWConfiguration/monitoring/IsPublisher.h"

namespace nsw::mon::internal {
  /**
   * \brief Loop over all devices and publish result to IS
   *
   * \tparam T HW interface type
   * \param hwis Set of devices to be monitored
   * \param isDict IS dictionary (from partition)
   * \param serverName Name of the IS monitoring server
   * \param groupName Name of the monitoring group
   * \param func Function that fills the IS object for each device
   */
  template<nsw::HWI T>
  void monitorAndPublish(const std::vector<T>& hwis,
                         ISInfoDictionary* isDict,
                         const std::string_view serverName,
                         const std::string_view groupName,
                         const std::regular_invocable<T> auto& func)
  {
    std::for_each(
      std::execution::par, std::begin(hwis), std::end(hwis), [&isDict, &func, &serverName, &groupName](const auto& device) {
        try {
          const auto values = func(device);
          ISPublisher::publish(
            isDict, serverName, groupName, nsw::getElementType(device.getScaAddress()), device.getScaAddress(), values);
        } catch (const std::exception& ex) {
          ERS_LOG("Monitoring failed due to " << ex.what());
        }
      });
  }
}  // namespace nsw::mon::internal

#endif
