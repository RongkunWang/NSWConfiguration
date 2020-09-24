// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Use bridge design pattern to implement multiple implementations?

#ifndef NSWCONFIGURATION_CONFIGREADER_H_
#define NSWCONFIGURATION_CONFIGREADER_H_

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

namespace nsw {
class ConfigReader {
 private:
  std::string m_connection_string;
  std::vector<std::string> m_components;  /// Components to read configuration
  std::unique_ptr<ConfigReaderApi> m_api;

 public:
  ConfigReader(const std::string& connection_string, const std::vector<std::string>& components);
  explicit ConfigReader(const std::string& connection_string);
  explicit ConfigReader(const ptree& tree);
  ~ConfigReader();

  ptree readConfig() {
    return m_api->read();
    // TODO(cyildiz): Verify there is "global config" and at least one VMM/FE instance
    // TODO(cyildiz): if (!m_components.empty()) Get Only relevant components config
  }

  ptree readConfig(const std::string& element_name) {
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
    nsw::ConfigReader reader1(cfg);
    try {
      auto config1 = reader1.readConfig();
    }
    catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
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
      names = reader1.getAllElementNames();
    }

    // make objects
    std::vector<T> configs;
    std::cout << "Adding:" << std::endl;
    for (auto & nm : names) {
      try {
        if (nsw::getElementType(nm) == element_type) {
          configs.emplace_back(reader1.readConfig(nm));
          std::cout << " " << nm;
          if (configs.size() % 4 == 0)
            std::cout << std::endl;
        }
      }
      catch (std::exception & e) {
        std::cout << nm << " - ERROR: Skipping this FE!"
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
      }
    }
    std::cout << std::endl;

    return configs;
  }
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_CONFIGREADER_H_
