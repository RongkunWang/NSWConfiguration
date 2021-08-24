#ifndef NSWCONFIGURATION_OKSDEVICEHIERARCHY_H
#define NSWCONFIGURATION_OKSDEVICEHIERARCHY_H

#include <set>
#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include "NSWConfiguration/Types.h"
#include "dal/ResourceBase.h"

namespace nsw::oks {
  /**
   * \brief Parse device hierarchy from OKS
   *
   * \param container Hierarchy object that is filled
   * \param contains Result of get_Contains of the parent OKS object
   * \param parentType Type of the parent OKS object
   * \return boost::property_tree::ptree TODO
   */
  [[nodiscard]] boost::property_tree::ptree parseDeviceMap(
    DeviceMap&                                         container,
    const std::vector<const daq::core::ResourceBase*>& contains,
    const std::string&                                 parentType);

  /**
   * \brief Init hierarchy struct
   *
   * \return DeviceMap Empty hierarchy struct
   */
  [[nodiscard]] DeviceMap initDeviceMap();

  /**
   * @brief Get all device names from a device map
   * 
   * @param deviceMap The device map
   * @return std::set<std::string> Names of all devices
   */
  std::set<std::string> getAllDeviceNames(const DeviceMap& deviceMap);
}  // namespace nsw::oks

#endif