#ifndef NSWCONFIGURATION_CONFIGREADERORACLEAPI_H_
#define NSWCONFIGURATION_CONFIGREADERORACLEAPI_H_

#include <array>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <occi.h>

#include "dal/ResourceBase.h"
#include "ers/Issue.h"
#include "ers/ers.h"

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Types.h"

class OracleApi : public ConfigReaderApi {
  private:
  struct OcciEnvironmentDeleter {
    void operator()(oracle::occi::Environment* env) const {
      oracle::occi::Environment::terminateEnvironment(env);
    }
  };
  using OcciEnv =
    std::unique_ptr<oracle::occi::Environment, OcciEnvironmentDeleter>;
  struct OcciConnectionDeleter {
    const OcciEnv& m_env;
    void           operator()(oracle::occi::Connection* con) const {
      m_env->terminateConnection(con);
    }
  };
  using OcciCon =
    std::unique_ptr<oracle::occi::Connection, OcciConnectionDeleter>;

  struct ParamNameTable {
    std::string                  param_name;
    constexpr static std::size_t num_entries{1};
  };

  struct ValueTable {
    std::string                  param_id;
    std::string                  param_value;
    constexpr static std::size_t num_entries{2};
  };

  struct DeviceTypeTable {
    std::string                  device_type;
    std::string                  device_subtype;
    constexpr static std::size_t num_entries{2};
  };

  /**
   * @brief Get the DB connection information from the passed string
   * 
   * @param configuration configuration string passed to constructor
   * @return std::string DB connection string
   */
  [[nodiscard]] static std::string getDbConnectionString(
    const std::string& configuration);

  /**
   * @brief Confirm that the configuration string passed to constructor is valid
   *
   * The configuration string should have the format <connection>|<config set>
   *
   * @param configuration configuration string passed to constructor
   * \throws nsw::ConfigIssue invalid format
   */
  static void testConfigurationString(const std::string& configuration);

  /**
   * @brief Get the config set name from the passed string
   * 
   * @param configuration configuration string passed to constructor
   * @return std::string config set name
   */
  [[nodiscard]] static std::string getConfigSet(
    const std::string& configuration);

  /**
   * @brief Execute a query and save data in table
   *
   * The query string should be a valid SQL query. Data (e.g. for WHERE statements)
   * are passed in via placeholders to prevent SQL injection attacks. Placeholders
   * are defined by a ':' followed by the a number starting at 0 for the first
   * placeholder (e.g. ':45' for the 46th placeholder in a query).
   * Values should be passed in containers. The order has to match the order of the
   * numbers in the placeholder string. Multiple containers can be passed and their
   * values will be substituted sequentially. For example, if the first container
   * has 3 elements and the second one 2 placeholders ':0', ':1', ':2' will be
   * substituted with the values from the first container and ':3', ':4' with the
   * values from the second container.
   *
   * The result will be stored in a map to a vector of tables. The key of the map
   * corresponds to the first variable in the query. The other variables from the
   * query are mapped to the table. All entries matching the key of the map are
   * stored in a vector.
   *
   * @tparam Table Table to save query result
   * @tparam ValueContainers Types of values to be substituted into query
   * @param query SQL query string (with placeholders)
   * @param valueContainers Values to be substituted into query
   * @return std::map<std::string, std::vector<Table>> Result of the query
   */
  template<typename Table, typename... ValueContainers>
  [[nodiscard]] std::map<std::string, std::vector<Table>> executeQuery(
    const std::string& query,
    const ValueContainers&... valueContainers) {
    auto* statement = m_occi_con->createStatement(query);

    // Set values for placeholders
    int counter = 1;
    (
      [statement, &counter](const auto& values) {
        for (const auto& value : values) {
          statement->setString(counter++, value);
        }
      }(valueContainers),
      ...);

    auto* result_set = statement->executeQuery();
    // First column will become the key of the map. Hence + 1
    if (result_set->getColumnListMetaData().size() != Table::num_entries + 1) {
      nsw::ConfigIssue issue(
        ERS_HERE,
        ("Number of columns in query does not match expectation " +
         std::to_string(Table::num_entries + 1))
          .c_str());
      ers::fatal(issue);
      throw issue;
    }
    std::map<std::string, std::vector<Table>> table;
    while (result_set->next()) {
      // Get ID
      if (result_set->isNull(1)) {
        throw std::runtime_error("First column of query is null.\n" + query);
      }
      const auto key = result_set->getString(1);

      // Get values (start counting at 1 and first argument is key -> + 2)
      std::array<std::string, Table::num_entries> row;
      for (std::size_t i = 0; i < Table::num_entries; i++) {
        if (result_set->isNull(i + 2)) {
          row[i] = "NULL";
        } else {
          row[i] = result_set->getString(i + 2);
        }
      }
      if (table.find(key) != std::end(table)) {
        table.try_emplace(key);
      }
      table.at(key).push_back(init_table<Table>(row));
    }
    statement->closeResultSet(result_set);
    m_occi_con->terminateStatement(statement);
    return table;
  }

  /**
   * @brief Initialize a table object with an array of values (internal)
   *
   * Internal function. Not to be called standalone.
   *
   * @tparam Result Type of the table
   * @tparam Array Type of the value array
   * @tparam I Index sequence of size of array
   * @param values Values to initialize the table
   * @return Result Initialized table
   */
  template<typename Result, typename Array, std::size_t... I>
  static Result init_table_impl(const Array& values,
                                std::index_sequence<I...>) {
    return Result{values[I]...};
  }

  /**
   * @brief Initialize a table object with an array of values
   * 
   * @tparam Result Type of the table
   * @tparam ValueType Type of the entries in the array
   * @tparam Indices Index sequence of size of array
   * @param values Values to initialize the table
   * @return Result Initialized table
   */
  template<typename Result,
           typename ValueType,
           typename Indices = std::make_index_sequence<Result::num_entries>>
  static Result init_table(
    const std::array<ValueType, Result::num_entries>& values) {
    return init_table_impl<Result>(values, Indices{});
  }

  /**
   * @brief Get the list of all device IDs from hierarchy object
   * 
   * @return std::set<std::string> Set of all device IDs
   */
  [[nodiscard]] std::set<std::string>        getAllDeviceIds() const;

  /**
   * @brief Get all unique device types 
   * 
   * @param deviceTypes Result table from DB query
   * @return std::set<std::string> Unique device types
   */
  [[nodiscard]] static std::set<std::string> getAllDeviceTypes(
    const std::map<std::string, DeviceTypeTable>& deviceTypes);

  /**
   * @brief Get all unique device sub-types
   * 
   * @param deviceSubtypes Result table from DB query
   * @return std::set<std::string> Unique device sub-types
   */
  [[nodiscard]] static std::set<std::string> getAllDeviceSubtypes(
    const std::map<std::string, DeviceTypeTable>& deviceSubtypes);

  /**
   * @brief Get all unique parameter IDs
   * 
   * @param table Result table from DB query
   * @return std::set<std::string> Unique parameter IDs
   */
  [[nodiscard]] static std::set<std::string> getAllParamIds(
    const std::map<std::string, std::vector<OracleApi::ValueTable>>& table);

  /**
   * @brief Query DB for all device types
   *
   * Since every device has exactly one type the vector inside the query result
   * have size 1, the map is flattened.
   *
   * @param deviceIds List of all device IDs
   * @param placeholder Placeholder string for device IDs in WHERE ... IN () clause
   * @return std::map<std::string, DeviceTypeTable> Map of device ID to type
   */
  [[nodiscard]] std::map<std::string, DeviceTypeTable> getDeviceTypes(
    const std::set<std::string>& deviceIds,
    const std::string&           placeholder);

  /**
   * @brief Query DB for sub-type defaults
   *
   * @param subtypes List of all sub-type IDs
   * @return std::map<std::string, std::vector<OracleApi::ValueTable>> Map of sub-type to defaults
   */
  [[nodiscard]] std::map<std::string, std::vector<OracleApi::ValueTable>>
  getSubtypeDefaults(const std::set<std::string>& subtypes);

  /**
   * @brief Query DB for type defaults
   * 
   * @param types List of all type IDs
   * @return std::map<std::string, std::vector<OracleApi::ValueTable>> Map of type to defaults
   */
  [[nodiscard]] std::map<std::string, std::vector<OracleApi::ValueTable>>
  getTypeDefaults(const std::set<std::string>& types);

  /**
   * @brief Query DB for parameter values
   * 
   * @param deviceIds List of all device IDs
   * @param placeholder Placeholder string for device IDs in WHERE ... IN () clause
   * @return std::map<std::string, std::vector<OracleApi::ValueTable>> Map of device ID to
   * parameter values
   */
  [[nodiscard]] std::map<std::string, std::vector<OracleApi::ValueTable>>
  getParamValues(const std::set<std::string>& deviceIds,
                 const std::string&           placeholder);

  /**
   * @brief Query DB for parameter names
   *
   * Since every device has exactly one type the vector inside the query result
   * have size 1, the map is flattened.
   * 
   * @param paramIds List of all parameter IDs
   * @return std::map<std::string, OracleApi::ParamNameTable> Map of parameter ID to name
   */
  [[nodiscard]] std::map<std::string, OracleApi::ParamNameTable> getParamNames(
    const std::set<std::string>& paramIds);

  /**
   * @brief Generate a comma separated string of placeholders for a given number of parameters
   *
   * The first placeholder will be number 0.
   *
   * @param num Number of placeholders
   * @return std::string Comma separated string of placeholders starting at 0
   */
  [[nodiscard]] static std::string generatePlaceholderString(std::size_t num);

  /**
   * @brief Populates the device hierarchy with value ptrees
   *
   * Result will be a map of device type to a map of device name to a value-based ptree.
   *
   * @param values Parameter values/defaults
   * @param paramNameMapping Mapping between param ID and name
   * @param deviceTypeMapping Mapping between device types and IDs
   * @return DeviceHierarchy Hierarchy object with value-based ptrees
   */
  [[nodiscard]] DeviceHierarchy buildValueTree(
    const std::map<std::string, std::vector<ValueTable>>&    values,
    const std::map<std::string, OracleApi::ParamNameTable>&  paramNameMapping,
    const std::map<std::string, OracleApi::DeviceTypeTable>& deviceTypeMapping)
    const;

  /**
   * @brief Merges the specific \ref DeviceHierarchy into the common one
   * 
   * @param specific Specific tree to be merged into common
   * @param common Common tree object (is modified!)
   */
  void mergeTrees(const DeviceHierarchy& specific, DeviceHierarchy& common) const;

  /**
   * @brief Transforms the pseudodevice names into the ones used in the value-based representation
   * 
   * @param pseudodevice Name of the pseudodevice
   * @return std::string Name in the value-based ptree (see \ref ConfigTranslationMap)
   */
  static std::string renamePseudodevice(const std::string& pseudodevice);

  /**
   * @brief Merge params for each channels into a vector
   * 
   * @param deviceTrees Hierarchy object with value-based ptrees
   */
  static void postprocessVmmTrees(DeviceHierarchy& deviceTrees);

  /**
   * @brief Flatten result of query if it yields only a single entry per map key
   * 
   * @tparam Table Type of the table
   * @param withVector Result of the query (vectors of size 1)
   * @return std::map<std::string, Table> Result of query without vector
   */
  template<typename Table>
  std::map<std::string, Table> flattenTableMap(
    const std::map<std::string, std::vector<Table>>& withVector) {
    std::map<std::string, Table> result;
    std::transform(std::begin(withVector),
                   std::end(withVector),
                   std::inserter(result, std::end(result)),
                   [](const auto& pair) -> std::pair<std::string, Table> {
                     const auto& key    = pair.first;
                     const auto& vector = pair.second;
                     if (vector.size() != 1) {
                       throw std::runtime_error(
                         "Query must yield exactly one result");
                     }
                     return {key, vector[0]};
                   });
    return result;
  }

  public:
  /**
   * @brief Construct a new Oracle Api object
   * 
   * @param configuration string of the format <connection>|<config set>
   * @param devices Set of devices to be configured
   */
  explicit OracleApi(const std::string& configuration,
                     DeviceHierarchy    devices = {});

  /**
   * @brief TODO
   * 
   * @return boost::property_tree::ptree& TODO
   */
  boost::property_tree::ptree& read() override;

  /**
   * @brief Parse device hierarchy from OKS
   * 
   * @param container Hierarchy object that is filled
   * @param contains Result of get_Contains of the parent OKS object
   * @param parentType Type of the parent OKS object
   * @return boost::property_tree::ptree TODO
   */
  [[nodiscard]] static boost::property_tree::ptree parseDeviceHierarchy(
    DeviceHierarchy&                                   container,
    const std::vector<const daq::core::ResourceBase*>& contains,
    const std::string&                                 parentType);

  /**
   * @brief Init hierarchy struct
   * 
   * @return DeviceHierarchy Empty hierarchy struct
   */
  [[nodiscard]] static DeviceHierarchy initDeviceHierarchy();

  private:
  std::string     m_db_user_name{"admin"};  ///< Name of the DB user
  std::string     m_db_password{"mysecurepassword123!"};  ///< Password of the DB user
  std::string     m_db_connection;  ///< Oracle DB connection string
  std::string     m_config_set;  ///< Name of the config set
  DeviceHierarchy m_devices;  ///< Devices to be configured
  // std::set<std::string>                  m_deviceIds;
  // std::string                            m_devicesPlaceholderString;
  OcciEnv m_occi_env;  ///< OCCI environment
  OcciCon m_occi_con;  ///< OCCI connection to Oracle DB
  // std::map<std::string, DeviceTypeTable> m_deviceTypes;
};

#endif