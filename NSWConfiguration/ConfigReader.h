// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Use bridge design pattern to implement multiple implementations?

#ifndef NSWCONFIGURATION_CONFIGREADER_H_
#define NSWCONFIGURATION_CONFIGREADER_H_

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <ers/ers.h>

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Utility.h"

#include "boost/property_tree/ptree_fwd.hpp"

namespace nsw {
class ConfigReader {
 private:
  std::string m_connection_string;
  std::vector<std::string> m_components;  /// Components to read configuration
  std::unique_ptr<ConfigReaderApi> m_api;

 public:
  ConfigReader(const std::string& connection_string, const std::vector<std::string>& components);
  explicit ConfigReader(const std::string& connection_string);
  explicit ConfigReader(const boost::property_tree::ptree& tree);
  explicit ConfigReader(const std::string& connection_string, const DeviceMap& devices);

  boost::property_tree::ptree readConfig() {
    return m_api->getConfig();
    // TODO(cyildiz): Verify there is "global config" and at least one VMM/FE instance
    // TODO(cyildiz): if (!m_components.empty()) Get Only relevant components config
  }

  boost::property_tree::ptree readConfig(const std::string& element_name) {
    return m_api->read(element_name);
  }

  //! Get names of all Front end elements in the config database
  std::set<std::string> getAllElementNames() {
    return m_api->getAllElementNames();
  }

  //! Get names of all Front end elements that match with regular expression
  std::set<std::string> getElementNames(const std::string& regexp) {
    return m_api->getElementNames(regexp);
  }

  //! Get vector of objects from config file and element type
  template <class T>
  static std::vector<T> makeObjects(const std::string& cfg, const std::string& element_type,
    const std::string& name = "") {

    // create config reader
    nsw::ConfigReader reader(cfg);

    // check
    if (name.find(" ") != std::string::npos) {
      const auto msg = fmt::format("'{}' has whitespace", name);
      nsw::ConfigIssue issue(ERS_HERE, msg.c_str());
      ers::fatal(issue);
    }

    // parse input names
    std::set<std::string> names;
    if (name != "") {
      if (std::count(name.begin(), name.end(), ',')) {
        std::istringstream ss(name);
        while (!ss.eof()) {
          std::string buf;
          std::getline(ss, buf, ',');
          if (buf != "")
            names.emplace(buf);
        }
      } else {
        names.emplace(name);
      }
    } else {
      names = reader.getAllElementNames();
    }

    // make objects
    std::vector<T> configs;
    ERS_LOG("Adding objects...");
    for (auto & nm : names) {
      try {
        if (nsw::getElementType(nm) == element_type) {
          configs.emplace_back(reader.readConfig(nm));
          ERS_LOG("Added " << nm);
        }
      }
      catch (std::exception & e) {
        const auto msg = fmt::format(
          "{} - WARNING: Skipping this FE! Problem constructing configuration: {}", nm, e.what());
        nsw::ConfigIssue issue(ERS_HERE, msg.c_str());
        ers::warning(issue);
      }
    }

    return configs;
  }

  private:
  [[nodiscard]] static std::unique_ptr<ConfigReaderApi> getApi(
    const std::string& connection_string,
    const DeviceMap&   devices);
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_CONFIGREADER_H_
