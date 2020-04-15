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

using boost::property_tree::ptree;

namespace nsw {
class ConfigReader {
 private:
  std::string m_connection_string;
  std::vector<std::string> m_components;  /// Components to read configuration
  std::unique_ptr<ConfigReaderApi> m_api;

 public:
  ConfigReader(const std::string connection_string, const std::vector<std::string> components);
  explicit ConfigReader(const std::string connection_string);
  ConfigReader(const ptree tree);
  ~ConfigReader();

  ptree readConfig() {
    return m_api->read();
    // TODO(cyildiz): Verify there is "global config" and at least one VMM/FE instance
    // TODO(cyildiz): if (!m_components.empty()) Get Only relevant components config
  }

  ptree readConfig(std::string element_name) {
    return m_api->read(element_name);
  }

  //! Get names of all Front end elements in the config database
  std::set<std::string> getAllElementNames() {
    return m_api->getAllElementNames();
  }

  //! Get names of all Front end elements that match with regular expression
  std::set<std::string> getElementNames(std::string regexp){
    return m_api->getElementNames(regexp);
  }
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_CONFIGREADER_H_
