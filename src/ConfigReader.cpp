#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/ConfigReaderJsonApi.h"
#include "NSWConfiguration/ConfigReaderOracleApi.h"

#include <exception>

#include "ers/ers.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/xml_parser.hpp"

using boost::property_tree::ptree;

nsw::ConfigReader::ConfigReader(const std::string& connection_string,
                                const std::vector<std::string>& components) :
  m_connection_string(connection_string),
  m_components(components),
  m_api(getApi(connection_string, {})) {}

nsw::ConfigReader::ConfigReader(const std::string& connection_string) :
  m_connection_string(connection_string),
  m_api(getApi(connection_string, {})) {}

nsw::ConfigReader::ConfigReader(const ptree& tree) :
  m_api(std::make_unique<JsonApi>(tree)) {}

nsw::ConfigReader::ConfigReader(const std::string& connection_string,
                                const DeviceMap&   devices) :
  m_connection_string(connection_string),
  m_api(getApi(connection_string, devices)) {}

std::unique_ptr<ConfigReaderApi> nsw::ConfigReader::getApi(
  const std::string&     connection_string,
  const DeviceMap& devices) {
  // Open db, json file or oracle db
  if (connection_string.find("json://") == 0) {
    const std::string file_path =
      connection_string.substr(std::string("json://").length());
    return std::make_unique<JsonApi>(file_path, devices);
  }
  if (connection_string.find("oracle:") == 0) {
    return std::make_unique<OracleApi>(connection_string, devices);
  }
  std::stringstream ss;
  ss << "Problem accessing the configuration in any of the supported formats.";
  ss << " The string has to be preceed by file type (e.g. json://).";
  nsw::ConfigIssue issue(ERS_HERE, ss.str().c_str());
  throw issue;
}