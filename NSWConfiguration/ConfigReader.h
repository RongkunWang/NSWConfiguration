// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Use bridge design pattern to implement multiple implementations?

#include <string>
#include <memory>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include "NSWConfiguration/ConfigReaderApi.h"

using boost::property_tree::ptree;

#ifndef NSWCONFIGURATION_CONFIGREADER_H_
#define NSWCONFIGURATION_CONFIGREADER_H_

class ConfigReader {
 private:
  std::string m_connection_string;
  std::vector<std::string> m_components;  /// Components to read configuration
  std::unique_ptr<ConfigReaderApi> m_api;

 public:
  ConfigReader(const std::string connection_string, const std::vector<std::string> components);
  explicit ConfigReader(const std::string connection_string);
  ~ConfigReader();

  ptree readConfig() {
    return m_api->read();
    // TODO(cyildiz): Verify there is "global config" and at least one VMM/FE instance
  }

  ptree readConfig(std::string element_name) {
    return m_api->read(element_name);
  }
};
#endif  // NSWCONFIGURATION_CONFIGREADER_H_

