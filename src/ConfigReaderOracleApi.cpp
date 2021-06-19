#include "NSWConfiguration/ConfigReaderOracleApi.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <string_view>

#include <boost/property_tree/json_parser.hpp>

#include "DAL_NSWConfiguration.tmp.cpp/NSW_MMFE8.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_VMM.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_pFEB.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_sFEB.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_TDS.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_ART.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_ROC.h"
#include "NSWConfiguration/Constants.h"

OracleApi::OracleApi(const std::string& configuration,
                     DeviceHierarchy    devices) :
  m_db_connection(getDbConnectionString(configuration)),
  m_config_set(getConfigSet(configuration)),
  m_devices(std::move(devices)),
  m_deviceIds(getAllDeviceIds()),
  m_placeholderString(generatePlaceholderString()),
  m_occi_env(oracle::occi::Environment::createEnvironment()),
  m_occi_con(m_occi_env->createConnection(m_db_user_name,
                                          m_db_password,
                                          m_db_connection),
             OcciConnectionDeleter{m_occi_env}) {}

void OracleApi::testConfigurationString(const std::string& configuration) {
  if (configuration.find('|') == std::string::npos) {
    nsw::ConfigIssue issue(
      ERS_HERE,
      "DB configuration string does not contain '|' seperator between DB "
      "connection and config set. Format should be <connection>|<config set>.");
    ers::fatal(issue);
    throw issue;
  }
}

std::string OracleApi::getDbConnectionString(const std::string& configuration) {
  testConfigurationString(configuration);
  return configuration.substr(0, configuration.find('|'));
}

std::string OracleApi::getConfigSet(const std::string& configuration) {
  testConfigurationString(configuration);
  return configuration.substr(configuration.find('|') + 1);
}

boost::property_tree::ptree& OracleApi::read() {
  return m_config;
}

boost::property_tree::ptree OracleApi::parseDeviceHierarchy(
  DeviceHierarchy&                                   container,
  const std::vector<const daq::core::ResourceBase*>& contains,
  const std::string&                                 parentType) {
  ERS_INFO("STARTING for parent type " << parentType);
  boost::property_tree::ptree parentTree;
  for (const auto* element : contains) {
    boost::property_tree::ptree elementTree;

    // Get device name, id and type. If it is not of interest return ""
    const auto [deviceName, deviceId, deviceType] =
      [element]() -> std::tuple<std::string, std::string, std::string> {
      const auto& className = element->class_name();
      // ERS_INFO("LOOP " << element << ' ' << className);
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
          deviceType == "L1DDC" or deviceType == "ADDC") {
        return deviceType;
      }
      return "";
    }();

    ERS_INFO("Relation type " << relationType);
    // Create ptree object for device if relation matches
    if (not relationType.empty()) {
      ERS_INFO("Adding name and ID to tree " << deviceName << ' ' << deviceId);
      elementTree.put("device_name", deviceName);
      elementTree.put("device_id", deviceId);
    }

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
          [counter = 1, prefix = "ch", suffix = "_DeviceID"]() mutable {
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << counter++;
            return prefix + ss.str() + suffix;
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
    if (pointerForContains != nullptr) {
      const auto& children = pointerForContains->get_Contains();
      //   ERS_INFO("CONTAINS SIZE  " << childs.size());
      const boost::property_tree::ptree result =
        parseDeviceHierarchy(container, children, deviceType);

      // If not empty add it to children of current tree
      if (not result.empty()) {
        ERS_INFO("Adding contains child");
        elementTree.add_child("children", result);
      }
    }

    // Add to container
    if (container.find(deviceType) != std::end(container)) {
      if (container.at(deviceType).find(deviceName) ==
          std::end(container.at(deviceType))) {
        ERS_INFO("Adding " << deviceName << " to container");
        container.at(deviceType).emplace(deviceName, elementTree);
      }
    }

    // If FEB child collect trees in parentTree
    if (relationType == "FEB_child") {
      parentTree.push_back(std::make_pair("", elementTree));
    }
  }

  return parentTree;
}

[[nodiscard]] DeviceHierarchy OracleApi::initDeviceHierarchy() {
  return {{"FEB", {}}, {"TP", {}}, {"ART", {}}, {"L1DDC", {}}, {"ADDC", {}}};
}

std::set<std::string> OracleApi::getAllDeviceIds() const {
  const std::string_view id = "device_id";
  const auto getIds         = [&id](const boost::property_tree::ptree& tree) {
    const auto getIdsImpl = [&id](const auto&                        func,
                                  const boost::property_tree::ptree& tree,
                                  std::set<std::string>& result) -> void {
      for (const auto& [name, value] : tree) {
        if (not value.empty() and value.data().empty()) {
          func(func, value, result);
        } else if (name == id) {
          result.insert(value.data());
        }
      }
    };
    std::set<std::string> result;
    getIdsImpl(getIdsImpl, tree, result);
    return result;
  };
  std::set<std::string> devices;
  for (const auto& iterType : m_devices) {
    for (const auto& iterDevice : iterType.second) {
      devices.merge(getIds(iterDevice.second));
    }
  }
}

std::map<std::string, OracleApi::DeviceTypeTable> OracleApi::getDeviceTypes() {
  std::string query = "SELECT      DEVICE_ID, DEVICE_TYPE_ID, DEVICE_SUBTYPE_ID"
                      "FROM        ATLAS_CONF_NSW.DEVICES_V"
                      "WHERE       ATLAS_CONF_NSW.DEVICES_V.DEVICE_NAME IN (" +
                      m_placeholderString + ')';
  std::map<std::string, OracleApi::DeviceTypeTable> result;
  const auto                                        resultWithVector =
    executeQuery<OracleApi::DeviceTypeTable>(query, m_deviceIds);
  std::transform(
    std::begin(resultWithVector),
    std::end(resultWithVector),
    std::inserter(result, std::end(result)),
    [](const auto& pair) -> std::pair<std::string, OracleApi::DeviceTypeTable> {
      const auto& key    = pair.first;
      const auto& vector = pair.second;
      if (vector.size() != 1) {
        throw std::runtime_error(
          "Query for device type must yield exactly one result");
      }
      return {key, vector[0]};
    });
  return result;
}

// std::map<std::string, OracleApi::DeviceTypeTable> OracleApi::getParamValues()
// {
//   std::string query = "SELECT      DEVICE_ID, PARAM_NAME, PARAM_VALUE"
//                       "FROM        ATLAS_CONF_NSW.DEVICES_V"
//                       "WHERE       ATLAS_CONF_NSW.DEVICES_V.DEVICE_NAME IN ("
//                       + m_placeholderString + ')';
//   return executeQuery<OracleApi::DeviceTypeTable>(query, m_deviceIds);
// }

std::string OracleApi::generatePlaceholderString() const {
  std::vector<int> v(m_deviceIds.size());
  std::iota(std::begin(v), std::end(v), 1);
  auto fold = [](std::string a, const int b) {
    return std::move(a) + ", :" + std::to_string(b);
  };
  return std::accumulate(std::next(v.begin()),
                         v.end(),
                         ':' + std::to_string(v[0]),  // first element
                         fold);
}

DeviceHierarchy OracleApi::buildValueTree(
  const std::map<std::string, std::vector<OracleApi::ValueTable>>& values)
  const {
  DeviceHierarchy result = initDeviceHierarchy();

  const auto fill = [&values](const boost::property_tree::ptree& tree) {
    const auto fillImpl = [&values](const auto&                        func,
                                    const boost::property_tree::ptree& tree)
      -> std::pair<std::string, boost::property_tree::ptree> {
      boost::property_tree::ptree result;
      const auto                  id = tree.get<std::string>("device_id");
      const auto deviceName          = tree.get<std::string>("device_name");
      const auto children            = tree.get_child("children");
      for (const auto& [paramName, value] : values.at(deviceName)) {
        result.put(paramName, value);
      }
      for (const auto& iter : children) {
        const auto [childName, childTree] = func(func, iter.second);
        result.add_child(childName, childTree);
      }
      return {id, result};
    };
    const auto [name, resultTree] = fillImpl(fillImpl, tree);
    boost::property_tree::ptree result;
    result.add_child(name, resultTree);
    return result;
  };

  for (const auto& [type, dict] : m_devices) {
    for (const auto& [name, tree] : dict) {
      result.at(type).emplace(name, fill(tree));
    }
  }
}