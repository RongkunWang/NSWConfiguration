#include "NSWConfiguration/ConfigReaderOracleApi.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <string_view>

#include <boost/property_tree/json_parser.hpp>
#include <unordered_map>

#include "DAL_NSWConfiguration.tmp.cpp/NSW_MMFE8.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_VMM.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_pFEB.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_sFEB.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_TDS.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_ART.h"
#include "DAL_NSWConfiguration.tmp.cpp/NSW_ROC.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Types.h"
#include "NSWConfiguration/DBToValueMapping.h"

OracleApi::OracleApi(const std::string& configuration,
                     DeviceHierarchy    devices) :
  m_db_connection(getDbConnectionString(configuration)),
  m_config_set(getConfigSet(configuration)),
  m_devices(std::move(devices)),
  // m_deviceIds(getAllDeviceIds()),
  // m_devicesPlaceholderString(generatePlaceholderString(m_deviceIds.size())),
  m_occi_env(oracle::occi::Environment::createEnvironment()),
  m_occi_con(m_occi_env->createConnection(m_db_user_name,
                                          m_db_password,
                                          m_db_connection),
             OcciConnectionDeleter{m_occi_env}) {
  // 1. Get all device IDs from hierarchy
  const auto uniqueDeviceIds = getAllDeviceIds();

  // 2. Get all device types and subtypes from DB
  const auto deviceIdPlaceholder =
    generatePlaceholderString(uniqueDeviceIds.size());
  const auto deviceTypes = getDeviceTypes(uniqueDeviceIds, deviceIdPlaceholder);

  // 3. Get all types and subtypes from 2
  const auto uniqueDeviceTypes   = getAllDeviceTypes(deviceTypes);
  const auto uniqueDeviceSubypes = getAllDeviceSubtypes(deviceTypes);

  // 4. Get defaults for all type and subtypes and values for all devices from
  // DB
  const auto typeDefaults    = getTypeDefaults(uniqueDeviceTypes);
  const auto subtypeDefaults = getTypeDefaults(uniqueDeviceSubypes);
  const auto paramValues = getParamValues(uniqueDeviceIds, deviceIdPlaceholder);

  // 5. Get all param IDs from type defaults
  const auto uniqueParamIds = getAllParamIds(typeDefaults);

  // 6. Get mapping param ID to param name from DB
  const auto paramNameMapping = getParamNames(uniqueParamIds);

  // 7. Generate type-default, subtype-default and value ptrees
  auto treeTypeDefault =
    buildValueTree(typeDefaults, paramNameMapping, deviceTypes);
  const auto treeSubtypeDefault =
    buildValueTree(subtypeDefaults, paramNameMapping, deviceTypes);
  const auto treeParamValues =
    buildValueTree(paramValues, paramNameMapping, deviceTypes);

  // 8. Merge subtype-default into type-default and values into that
  mergeTrees(treeSubtypeDefault, treeTypeDefault);
  mergeTrees(treeParamValues, treeTypeDefault);
  // const auto& treeMerged = treeTypeDefault;

  // 10. Convert value-based ptree into register-based ptree
}

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
  return devices;
}

std::set<std::string> OracleApi::getAllDeviceTypes(
  const std::map<std::string, DeviceTypeTable>& deviceTypes) {
  std::set<std::string> result;
  std::transform(std::begin(deviceTypes),
                 std::end(deviceTypes),
                 std::inserter(result, std::end(result)),
                 [](const auto& pair) { return pair.second.device_type; });
  return result;
}

std::set<std::string> OracleApi::getAllDeviceSubtypes(
  const std::map<std::string, DeviceTypeTable>& deviceSubtypes) {
  std::set<std::string> result;
  std::transform(std::begin(deviceSubtypes),
                 std::end(deviceSubtypes),
                 std::inserter(result, std::end(result)),
                 [](const auto& pair) { return pair.second.device_subtype; });
  return result;
}

std::set<std::string> OracleApi::getAllParamIds(
  const std::map<std::string, std::vector<OracleApi::ValueTable>>& table) {
  std::set<std::string> result;
  for (const auto& [key, vector] : table) {
    std::transform(std::begin(vector),
                   std::end(vector),
                   std::inserter(result, std::end(result)),
                   [](const auto& element) { return element.param_id; });
  }
  return result;
}

std::map<std::string, OracleApi::DeviceTypeTable> OracleApi::getDeviceTypes(
  const std::set<std::string>& deviceIds,
  const std::string&           placeholder) {
  std::string query =
    "SELECT      DEVICE_ID, DEVICE_TYPE_ID, DEVICE_SUBTYPE_ID "
    "FROM        ATLAS_CONF_NSW.DEVICES_V "
    "WHERE       ATLAS_CONF_NSW.DEVICES_V.DEVICE_NAME IN (" +
    placeholder + ')';
  return flattenTableMap(
    executeQuery<OracleApi::DeviceTypeTable>(query, deviceIds));
}

std::map<std::string, std::vector<OracleApi::ValueTable>>
OracleApi::getParamValues(const std::set<std::string>& deviceIds,
                          const std::string&           placeholder) {
  const std::string setPlaceholder = ':' + std::to_string(deviceIds.size());
  std::string       query =
    " SELECT DVALUES.DEVICE_ID, DVALUES.PARAM_ID, DVALUES.PARAM_VALUE"
    " FROM ATLAS_CONF_NSW.DEVICE_PARAM_VALUES DVALUES"
    " RIGHT JOIN ATLAS_CONF_NSW.CONFIG_ARGUMENTS DARGS"
    "   ON DARGS.VALUE_ID = DVALUES.VALUE_ID"
    " RIGHT JOIN ATLAS_CONF_NSW.CONFIG_SET_ITEMS SETITEMS"
    "   ON SETITEMS.CONFIG_ID = DARGS.CONFIG_ID"
    " LEFT JOIN ATLAS_CONF_NSW.CONFIG_SETS CSET"
    "   ON CSET.SET_ID = SETITEMS.SET_ID"
    " WHERE DVALUES.DEVICE_ID IN (" +
    placeholder + ") AND CSET.SET_NAME = " + setPlaceholder +
    " AND DARGS.UNTIL = TO_DATE('1/JAN/3000','dd/mon/yyyy')"
    " AND SETITEMS.UNTIL = TO_DATE('1/JAN/3000','dd/mon/yyyy');";
  return executeQuery<OracleApi::ValueTable>(
    query, deviceIds, std::array{setPlaceholder});
}

std::map<std::string, std::vector<OracleApi::ValueTable>>
OracleApi::getTypeDefaults(const std::set<std::string>& types) {
  const auto  placeholder = generatePlaceholderString(types.size());
  std::string query =
    " SELECT ATLAS_CONF_NSW.PARAM_DEFAULTS_V.TYPE_ID, "
    " ATLAS_CONF_NSW.PARAM_DEFAULTS_V.PARAM_ID, "
    " ATLAS_CONF_NSW.PARAM_DEFAULTS_V.DEFAULT_VALUE"
    " FROM ATLAS_CONF_NSW.PARAM_DEFAULTS_V"
    " LEFT JOIN ATLAS_CONF_NSW.DEVICE_SUBTYPES"
    "   ON ATLAS_CONF_NSW.PARAM_DEFAULTS_V.SUBTYPE_ID = "
    " ATLAS_CONF_NSW.DEVICE_SUBTYPES.SUBTYPE_ID"
    " WHERE ATLAS_CONF_NSW.PARAM_DEFAULTS_V.TYPE_ID IN (" +
    placeholder +
    ") AND ATLAS_CONF_NSW.DEVICE_SUBTYPES.SUBTYPE_NAME IS NULL"
    " AND ATLAS_CONF_NSW.PARAM_DEFAULTS_V.UNTIL = "
    " TO_DATE('1/JAN/3000','dd/mon/yyyy');";
  return executeQuery<OracleApi::ValueTable>(query, types);
}

std::map<std::string, std::vector<OracleApi::ValueTable>>
OracleApi::getSubtypeDefaults(const std::set<std::string>& subtypes) {
  const auto  placeholder = generatePlaceholderString(subtypes.size());
  std::string query =
    R"(SELECT ATLAS_CONF_NSW.PARAM_DEFAULTS.SUBTYPE_ID, ATLAS_CONF_NSW.PARAM_DEFAULTS.PARAM_ID, ATLAS_CONF_NSW.PARAM_DEFAULTS.DEFAULT_VALUE
FROM ATLAS_CONF_NSW.PARAM_DEFAULTS
WHERE ATLAS_CONF_NSW.PARAM_DEFAULTS.SUBTYPE_ID IN ()" +
    placeholder +
    R"()) AND ATLAS_CONF_NSW.PARAM_DEFAULTS.UNTIL = TO_DATE('1/JAN/3000','dd/mon/yyyy');
)";
  return executeQuery<OracleApi::ValueTable>(query, subtypes);
}

std::map<std::string, OracleApi::ParamNameTable> OracleApi::getParamNames(
  const std::set<std::string>& paramIds) {
  const auto  placeholder = generatePlaceholderString(paramIds.size());
  std::string query =
    R"(SELECT ATLAS_CONF_NSW.DEVICE_TYPE_PARAMS.PARAM_ID, ATLAS_CONF_NSW.DEVICE_TYPE_PARAMS.PARAM_NAME
FROM ATLAS_CONF_NSW.DEVICE_TYPE_PARAMS
WHERE ATLAS_CONF_NSW.DEVICE_TYPE_PARAMS.PARAM_ID IN ()" +
    placeholder +
    R"());
)";
  return flattenTableMap(
    executeQuery<OracleApi::ParamNameTable>(query, paramIds));
}

std::string OracleApi::generatePlaceholderString(const std::size_t num) {
  std::vector<int> v(num);
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
  const std::map<std::string, std::vector<OracleApi::ValueTable>>& values,
  const std::map<std::string, OracleApi::ParamNameTable>&  paramNameMapping,
  const std::map<std::string, OracleApi::DeviceTypeTable>& deviceTypeMapping)
  const {
  DeviceHierarchy result = initDeviceHierarchy();

  const auto fill = [&values, &paramNameMapping, &deviceTypeMapping](
                      const boost::property_tree::ptree& tree) {
    const auto fillImpl = [&values, &paramNameMapping, &deviceTypeMapping](
                            const auto&                        func,
                            const boost::property_tree::ptree& tree)
      -> std::pair<std::string, boost::property_tree::ptree> {
      boost::property_tree::ptree result;
      const auto                  id = tree.get<std::string>("device_id");
      const auto deviceName          = tree.get<std::string>("device_name");
      const auto children            = tree.get_child("children");
      for (const auto& [paramId, value] : values.at(deviceName)) {
        for (const auto& [key, valToPut] :
             nsw::Oracle::transform(deviceTypeMapping.at(id).device_type,
                                    paramNameMapping.at(paramId).param_name,
                                    value)) {
          result.put(key, valToPut);
        }
      }
      for (const auto& iter : children) {
        const auto [childName, childTree] = func(func, iter.second);
        result.add_child(childName, childTree);
      }
      return {renamePseudodevice(deviceName), result};
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

  return result;
}

void OracleApi::mergeTrees(const DeviceHierarchy& specific,
                           DeviceHierarchy&       common) const {
  if (specific.size() != common.size()) {
    throw std::runtime_error(
      "Can only merge tree structures with equal length");
  }
  auto itSpecific = std::begin(specific);
  auto itCommon   = std::begin(common);
  for (; itSpecific != std::end(specific); itSpecific++, itCommon++) {
    const auto& innerSpecific   = itSpecific->second;
    auto&       innerCommon     = itCommon->second;
    auto        itInnerSpecific = std::begin(innerSpecific);
    auto        itInnerCommon   = std::begin(innerCommon);
    if (innerSpecific.size() != innerCommon.size()) {
      throw std::runtime_error(
        "Can only merge tree structures with equal length");
    }
    for (; itInnerSpecific != std::end(innerSpecific);
         itInnerSpecific++, itInnerCommon++) {
      mergeTree(itInnerSpecific->second, itInnerCommon->second);
    }
  }
}

std::string OracleApi::renamePseudodevice(const std::string& pseudodevice) {
  const std::unordered_map<std::string_view, std::string> mapping{
    {{"ePLLVMM0", "ePllVmm0"},
     {"ePLLVMM1", "ePllVmm1"},
     {"ePLLTDC", "ePllTdc"}}};
  for (const auto& [id, name] : mapping) {
    if (pseudodevice.find(id) != std::string::npos) {
      return name;
    }
  }

  // ch36_VMM6_A1L3M04 -> ch36
  if (pseudodevice.find("VMMchannel") != std::string::npos) {
    return pseudodevice.substr(0, pseudodevice.find('_'));
  }
  return pseudodevice;
}

void OracleApi::postprocessVmmTrees(DeviceHierarchy& deviceTrees) {
  std::array<std::string, nsw::vmm::NUM_CH_PER_VMM> channelNames{};
  std::generate_n(std::begin(channelNames),
                  nsw::vmm::NUM_CH_PER_VMM,
                  [counter = 0, prefix = "ch"]() mutable {
                    std::stringstream ss;
                    ss << std::setw(2) << std::setfill('0') << counter++;
                    return prefix + ss.str();
                  });
  std::array paramNames{
    "sl", "st", "sth", "sm", "smx", "sd", "sz10b", "sz8b", "sz6b", "sc"};
  std::string prefix = "channel_";
  for (auto& [deviceName, tree] : deviceTrees.at("VMM")) {
    for (const auto& paramName : paramNames) {
      boost::property_tree::ptree node;
      for (const auto& channelName : channelNames) {
        boost::property_tree::ptree entry;
        entry.put("", tree.get<unsigned int>(channelName + '.' + paramName));
        node.push_back(std::make_pair("", entry));
      }
      tree.add_child("channel_" + std::string{paramName}, node);
    }
    for (const auto& channelName : channelNames) {
      tree.erase(channelName);
    }
  }
}