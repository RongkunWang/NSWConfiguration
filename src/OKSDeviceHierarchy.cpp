#include "NSWConfiguration/OKSDeviceHierarchy.h"

#include <algorithm>
#include <iomanip>
#include <stdexcept>

#include <boost/property_tree/json_parser.hpp>
#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"

#include <NSWConfigurationDal/NSW_MMFE8.h>
#include <NSWConfigurationDal/NSW_pFEB.h>
#include <NSWConfigurationDal/NSW_sFEB.h>
#include <NSWConfigurationDal/NSW_TDS.h>
#include <NSWConfigurationDal/NSW_PadTrigger.h>
#include <NSWConfigurationDal/NSW_TP.h>
#include <NSWConfigurationDal/NSW_TPCarrier.h>
#include <NSWConfigurationDal/NSW_Router.h>
#include <NSWConfigurationDal/NSW_L1DDC.h>
#include <NSWConfigurationDal/NSW_L1DDC_MM.h>
#include <NSWConfigurationDal/NSW_L1DDC_pFEB.h>
#include <NSWConfigurationDal/NSW_L1DDC_sFEB.h>
#include <NSWConfigurationDal/NSW_RimL1DDC.h>
#include <NSWConfigurationDal/NSW_ADDC.h>

#include <dal/Partition.h>

void nsw::oks::parseDeviceMap(DeviceMap& container,
                              const std::vector<const daq::core::ResourceBase*>& contains,
                              const daq::core::Partition* partition,
                              const bool json)
{
  boost::property_tree::ptree parentTree;
  for (const auto* element : contains) {
    // Skip if device is disabled
    if (partition != nullptr) {
      if (element->disabled(*partition)) {
        continue;
      }
    }

    // Get device name, id and type. If it is not of interest return ""
    const auto [deviceName, deviceId, deviceType] =
      [element, &json]() -> std::tuple<std::string, std::string, std::string> {
      const auto& className = element->class_name();
      const auto getData = [&json](const auto* pointer) {
        const auto preprocessName = [&json](const auto& name) {
          if (json) {
            constexpr std::size_t INDEX_DASH = 3;
            constexpr std::size_t NUM_CHARS_PREFIX = 8;
            if (name.at(NUM_CHARS_PREFIX - 1) != '_' or name.at(INDEX_DASH) != '-') {
              throw std::logic_error(
                fmt::format("Attempted to remove sector prefix from device "
                            "{} but eighth character was not '_' or the fourth was not '-'",
                            name));
            }
            return name.substr(NUM_CHARS_PREFIX);
          }
          return name;
        };
        return std::pair{preprocessName(pointer->UID()), std::to_string(pointer->get_DeviceID())};
      };
      if (className == "NSW_MMFE8") {
        const auto* device = element->cast<nsw::dal::NSW_MMFE8>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "FEB"};
      }
      if (className == "NSW_sFEB") {
        const auto* device = element->cast<nsw::dal::NSW_sFEB>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "FEB"};
      }
      if (className == "NSW_pFEB") {
        const auto* device = element->cast<nsw::dal::NSW_pFEB>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "FEB"};
      }
      if (className == "NSW_L1DDC") {
        const auto* device = element->cast<nsw::dal::NSW_L1DDC>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "L1DDC"};
      }
      if (className == "NSW_L1DDC_MM") {
        const auto* device = element->cast<nsw::dal::NSW_L1DDC_MM>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "L1DDC"};
      }
      if (className == "NSW_L1DDC_pFEB") {
        const auto* device = element->cast<nsw::dal::NSW_L1DDC_pFEB>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "L1DDC"};
      }
      if (className == "NSW_L1DDC_sFEB") {
        const auto* device = element->cast<nsw::dal::NSW_L1DDC_sFEB>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "L1DDC"};
      }
      if (className == "NSW_RimL1DDC") {
        const auto* device = element->cast<nsw::dal::NSW_RimL1DDC>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "RimL1DDC"};
      }
      if (className == "NSW_ADDC") {
        const auto* device = element->cast<nsw::dal::NSW_ADDC>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "ADDC"};
      }
      if (className == "NSW_PadTrigger") {
        const auto* device = element->cast<nsw::dal::NSW_PadTrigger>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "PadTrigger"};
      }
      if (className == "NSW_TP") {
        const auto* device = element->cast<nsw::dal::NSW_TP>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "TP"};
      }
      if (className == "NSW_TPCarrier") {
        const auto* device = element->cast<nsw::dal::NSW_TPCarrier>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "TPCarrier"};
      }
      if (className == "NSW_Router") {
        const auto* device = element->cast<nsw::dal::NSW_Router>();
        const auto [deviceNameLocal, deviceIdLocal] = getData(device);
        return {deviceNameLocal, deviceIdLocal, "Router"};
      }
      return {"", "", ""};
    }();

    ERS_DEBUG(5, fmt::format("Analyzing device {} {} {}", deviceName, deviceId, deviceType));

    // Create ptree object for device if relation matches
    boost::property_tree::ptree elementTree;
    if (not deviceType.empty()) {
      ERS_DEBUG(5, fmt::format("Adding name {} and ID {} to tree ", deviceName, deviceId));
      elementTree.put("device_name", deviceName);
      elementTree.put("device_id", deviceId);
    }

    // Get the value of an attribute
    const auto getDeviceId = [element](const std::string& name) {
      auto configObject = element->config_object();
      std::uint64_t id{0};
      configObject.get(name, id);
      return std::to_string(id);
    };

    boost::property_tree::ptree childrenTree;
    if (deviceType == "FEB") {
      // Get names for pseudodevices
      static constexpr std::array names{"VMM0_DeviceID", "VMM1_DeviceID", "TDC_DeviceID"};

      // Add to tree
      for (const auto& name : names) {
        boost::property_tree::ptree pseudodeviceTree;
        pseudodeviceTree.put("device_name", name);
        pseudodeviceTree.put("device_id", getDeviceId(name));
        childrenTree.push_back(std::make_pair("", pseudodeviceTree));
      }

    }
    // Get VMMs, ROCs, sROCs, TDSs for FEBs and ARTs for ADDCs
    if (deviceType == "FEB" or deviceType == "ADDC") {
      const auto getDeviceNames = [&]() {
        const std::string key{"DeviceNames"};
        auto names = std::vector<std::string>{};
        auto configObject = element->config_object();
        configObject.get(key, names);
        return names;
      };
      const auto getDeviceIds = [&]() {
        const std::string key{"DeviceIds"};
        auto ids = std::vector<std::uint64_t>{};
        auto configObject = element->config_object();
        configObject.get(key, ids);
        return ids;
      };
      const auto subDeviceNames = getDeviceNames();
      const auto subDeviceIds = getDeviceIds();

      if (std::size(subDeviceIds) != std::size(subDeviceNames)) {
        throw std::runtime_error(fmt::format(
          "Device names and IDs for device {} do not have the same length", deviceName));
      }

      // Add to tree
      for (std::size_t c = 0; c < std::size(subDeviceNames); c++) {
        boost::property_tree::ptree pseudodeviceTree;
        pseudodeviceTree.put("device_name", subDeviceNames.at(c));
        pseudodeviceTree.put("device_id", subDeviceIds.at(c));
        childrenTree.push_back(std::make_pair("", pseudodeviceTree));
      }

      elementTree.add_child("children", childrenTree);
    }

    // Iterate through children
    const auto* pointerForContains = element->cast<daq::core::ResourceSet>();
    // If it is a nullptr it does not have a contains
    if (pointerForContains != nullptr) {
      const auto& children = pointerForContains->get_Contains();
      parseDeviceMap(container, children, partition, json);
    }

    // Add device to device hierarchy container if it does not already exist
    if (container.find(deviceType) != std::end(container)) {
      if (container.at(deviceType).find(deviceName) == std::end(container.at(deviceType))) {
        ERS_DEBUG(5, "Adding " << deviceName << " to container");
        container.at(deviceType).emplace(deviceName, elementTree);
      }
    }
  }
}

[[nodiscard]] nsw::DeviceMap nsw::oks::initDeviceMap()
{
  return {{"FEB", {}},
          {"RimL1DDC", {}},
          {"L1DDC", {}},
          {"ADDC", {}},
          {"Router", {}},
          {"PadTrigger", {}},
          {"TP", {}},
          {"TPCarrier", {}}};
}

std::set<std::string> nsw::oks::getAllDeviceNames(const nsw::DeviceMap& deviceMap)
{
  std::set<std::string> devices;
  for (const auto& [type, map] : deviceMap) {
    std::transform(std::cbegin(map),
                   std::cend(map),
                   std::inserter(devices, std::end(devices)),
                   [](const auto& pair) { return pair.first; });
  }
  return devices;
}

void nsw::oks::printDeviceMap(const nsw::DeviceMap& devices)
{
  ERS_INFO("Hierarchy");
  for (const auto& [type, map] : devices) {
    ERS_INFO("Type: " << type);
    for (const auto& [name, tree] : map) {
      ERS_INFO("Name: " << name);
      std::stringstream stream;
      boost::property_tree::json_parser::write_json(stream, tree);
      ERS_INFO("Tree: " << stream.str());
    }
  }
}