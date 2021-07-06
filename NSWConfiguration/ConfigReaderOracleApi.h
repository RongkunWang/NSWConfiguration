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
    std::string                  param_id;
    std::string                  param_name;
    constexpr static std::size_t num_entries{2};
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

  [[nodiscard]] static std::string getDbConnectionString(
    const std::string& configuration);
  static void testConfigurationString(const std::string& configuration);
  [[nodiscard]] static std::string getConfigSet(
    const std::string& configuration);

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

  template<typename Result, typename Array, std::size_t... I>
  static Result init_table_impl(const Array& values,
                                std::index_sequence<I...>) {
    return Result{values[I]...};
  }

  template<typename Result,
           typename ValueType,
           typename Indices = std::make_index_sequence<Result::num_entries>>
  static Result init_table(
    const std::array<ValueType, Result::num_entries>& values) {
    return init_table_impl<Result>(values, Indices{});
  }

  std::set<std::string> getAllDeviceIds() const;
  std::set<std::string> getAllDeviceTypes() const;
  std::set<std::string> getAllDeviceSubtypes() const;
  std::set<std::string> getAllParamIds() const;

  std::map<std::string, DeviceTypeTable> getDeviceTypes();
  std::map<std::string, std::vector<OracleApi::ValueTable>>
  getSubtypeDefaults();
  std::map<std::string, std::vector<OracleApi::ValueTable>> getTypeDefaults();
  std::map<std::string, std::vector<OracleApi::ValueTable>> getParamValues();

  [[nodiscard]] static std::string generatePlaceholderString(std::size_t num);

  DeviceHierarchy buildValueTree(
    const std::map<std::string, std::vector<ValueTable>>& values) const;

  public:
  explicit OracleApi(const std::string& configuration,
                     DeviceHierarchy    devices = {});
  boost::property_tree::ptree& read() override;

  //! Parse device hierarchy from OKS
  [[nodiscard]] static boost::property_tree::ptree parseDeviceHierarchy(
    DeviceHierarchy&                                   container,
    const std::vector<const daq::core::ResourceBase*>& contains,
    const std::string&                                 parentType);

  //! Init hierarchy struct
  [[nodiscard]] static DeviceHierarchy initDeviceHierarchy();

  private:
  std::string                            m_db_user_name{"admin"};
  std::string                            m_db_password{"mysecurepassword123!"};
  std::string                            m_db_connection;
  std::string                            m_config_set;
  DeviceHierarchy                        m_devices;
  std::set<std::string>                  m_deviceIds;
  std::string                            m_devicesPlaceholderString;
  OcciEnv                                m_occi_env;
  OcciCon                                m_occi_con;
  std::map<std::string, DeviceTypeTable> m_deviceTypes;
};

#endif