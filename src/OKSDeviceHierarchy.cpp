#include "NSWConfiguration/OKSDeviceHierarchy.h"

#include <algorithm>
#include <iomanip>

#include "NSWConfiguration/Constants.h"

#include "NSWConfigurationDal/NSW_MMFE8.h"
#include "NSWConfigurationDal/NSW_VMM.h"
#include "NSWConfigurationDal/NSW_pFEB.h"
#include "NSWConfigurationDal/NSW_sFEB.h"
#include "NSWConfigurationDal/NSW_TDS.h"
#include "NSWConfigurationDal/NSW_ART.h"
#include "NSWConfigurationDal/NSW_ROC.h"
#include "NSWConfigurationDal/NSW_PadTrigger.h"
#include "NSWConfigurationDal/NSW_TP.h"
#include "NSWConfigurationDal/NSW_TPCarrier.h"
#include "NSWConfigurationDal/NSW_Router.h"
#include "NSWConfigurationDal/NSW_L1DDC.h"
#include "NSWConfigurationDal/NSW_ADDC.h"

boost::property_tree::ptree nsw::oks::parseDeviceMap(
  DeviceMap&                                   container,
  const std::vector<const daq::core::ResourceBase*>& contains,
  const std::string&                                 parentType) {
  ERS_INFO("STARTING for parent type " << parentType);
  boost::property_tree::ptree parentTree;
  for (const auto* element : contains) {
    // Get device name, id and type. If it is not of interest return ""
    const auto [deviceName, deviceId, deviceType] =
      [element]() -> std::tuple<std::string, std::string, std::string> {
      const auto& className = element->class_name();
      const auto getData = [](const auto* pointer) {
        return std::pair{pointer->UID(),
                         std::to_string(pointer->get_DeviceID())};
      };
      if (className == "NSW_MMFE8") {
        const auto* device = element->cast<nsw::dal::NSW_MMFE8>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "FEB"};
      }
      if (className == "NSW_sFEB") {
        const auto* device                = element->cast<nsw::dal::NSW_sFEB>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "FEB"};
      }
      if (className == "NSW_pFEB") {
        const auto* device                = element->cast<nsw::dal::NSW_pFEB>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "FEB"};
      }
      if (className == "NSW_VMM") {
        const auto* device                = element->cast<nsw::dal::NSW_VMM>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "VMM"};
      }
      if (className == "NSW_TDS") {
        const auto* device                = element->cast<nsw::dal::NSW_TDS>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "TDS"};
      }
      if (className == "NSW_ART") {
        const auto* device                = element->cast<nsw::dal::NSW_ART>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "ART"};
      }
      if (className == "NSW_ROC") {
        const auto* device                = element->cast<nsw::dal::NSW_ROC>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "ROC"};
      }
      if (className == "NSW_L1DDC") {
        const auto* device                = element->cast<nsw::dal::NSW_L1DDC>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "L1DDC"};
      }
      if (className == "NSW_ADDC") {
        const auto* device                = element->cast<nsw::dal::NSW_ADDC>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "ADDC"};
      }
      if (className == "NSW_PadTrigger") {
        const auto* device                = element->cast<nsw::dal::NSW_PadTrigger>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "PadTrigger"};
      }
      if (className == "NSW_TP") {
        const auto* device                = element->cast<nsw::dal::NSW_TP>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "TP"};
      }
      if (className == "NSW_TPCarrier") {
        const auto* device                = element->cast<nsw::dal::NSW_TPCarrier>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "TPCarrier"};
      }
      if (className == "NSW_Router") {
        const auto* device                = element->cast<nsw::dal::NSW_Router>();
        const auto [deviceName, deviceId] = getData(device);
        return {deviceName, deviceId, "Router"};
      }
      return {"", "", ""};
    }();

    ERS_INFO("Analyzing device " << deviceName << ' ' << deviceId << ' '
                                 << deviceType);

    // Get relation type
    const auto relationType = [&parentType, &deviceType]() -> std::string {
      if (parentType == "FEB" and
          (deviceType == "ROC" or deviceType == "VMM" or deviceType == "TDS")) {
        return "FEB_child";
      }
      if (deviceType == "FEB" or deviceType == "TP" or deviceType == "ART" or
          deviceType == "L1DDC" or deviceType == "ADDC" or deviceType == "PadTrigger" or
          deviceType == "TP" or deviceType == "TPCarrier" or deviceType == "Router") {
        return deviceType;
      }
      return "";
    }();

    ERS_INFO("Relation type " << relationType);
    // Create ptree object for device if relation matches
    boost::property_tree::ptree elementTree;
    if (not relationType.empty()) {
      ERS_INFO("Adding name and ID to tree " << deviceName << ' ' << deviceId);
      elementTree.put("device_name", deviceName);
      elementTree.put("device_id", deviceId);
    }

    // Get the value of an attribute
    const auto getDeviceId = [element](const std::string& name) {
      auto          configObject = element->config_object();
      std::uint64_t id{0};
      configObject.get(name, id);
      return std::to_string(id);
    };

    // Create subdevices for ROC and VMM
    if (relationType == "FEB_child") {
      // Get names for pseudodevices
      std::vector<std::string> names;
      if (deviceType == "VMM") {
        std::generate_n(
          std::back_inserter(names),
          nsw::vmm::NUM_CH_PER_VMM,
          [counter = 0, prefix = "ch", suffix = "_DeviceID"]() mutable {
            std::stringstream ss;
            ss << prefix << std::setw(2) << std::setfill('0') << counter++ << suffix;
            return ss.str();
          });
      } else if (deviceType == "ROC") {
        names = {"VMM0_DeviceID", "VMM1_DeviceID", "TDC_DeviceID"};
      }

      // Add to tree
      boost::property_tree::ptree childrenTree;
      for (const auto& name : names) {
        boost::property_tree::ptree pseudodeviceTree;
        pseudodeviceTree.put("device_name", name);
        pseudodeviceTree.put("device_id", getDeviceId(name));
        childrenTree.push_back(std::make_pair("", pseudodeviceTree));
      }
      elementTree.add_child("children", childrenTree);
    }

    // Iterate through children
    const auto* pointerForContains = element->cast<daq::core::ResourceSet>();
    // If it is a nullptr it does not have a contains
    if (pointerForContains != nullptr) {
      const auto& children = pointerForContains->get_Contains();
      const boost::property_tree::ptree result =
        parseDeviceMap(container, children, deviceType);

      // If not empty add it to children of current tree
      if (not result.empty()) {
        ERS_INFO("Adding contains child");
        elementTree.add_child("children", result);
      }
    }

    // Add device to device hierarchy container if it does not already exist
    if (container.find(deviceType) != std::end(container)) {
      if (container.at(deviceType).find(deviceName) ==
          std::end(container.at(deviceType))) {
        ERS_INFO("Adding " << deviceName << " to container");
        container.at(deviceType).emplace(deviceName, elementTree);
      }
    }

    // If device is a ROC, VMM or TDS collect all trees in a list and return them.
    // The caller (FEB) will add the list to its children.
    if (relationType == "FEB_child") {
      parentTree.push_back(std::make_pair("", elementTree));
    }
  }

  return parentTree;
}

[[nodiscard]] nsw::DeviceMap nsw::oks::initDeviceMap() {
  return {{"FEB", {}}, {"TP", {}}, {"ART", {}}, {"L1DDC", {}}, {"ADDC", {}}, {"Router", {}}, {"PadTrigger", {}}, {"TP", {}}, {"TPCarrier", {}}};
}

std::set<std::string> nsw::oks::getAllDeviceNames(const nsw::DeviceMap& deviceMap) {
  std::set<std::string> devices;
  for (const auto& [type, map] : deviceMap) {
    std::transform(std::cbegin(map), std::cend(map), std::inserter(devices, std::end(devices)), [] (const auto& pair) { return pair.first; });
  }
  return devices;
}
