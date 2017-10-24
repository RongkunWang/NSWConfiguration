#include <memory>
#include <exception>
#include "ConfigReader.h"

ConfigReader::ConfigReader(const std::string connection_string): m_connection_string(connection_string) {
    // Open db or json file
    auto pos = m_connection_string.find("file://");
    if (pos != std::string::npos) {
        std::string file_path = m_connection_string.substr(std::string("file://").length());
        m_api = std::make_unique<JsonApi> (file_path);
    } else {
        m_api = std::make_unique<OracleApi> (m_connection_string);
    }
}

ConfigReader::~ConfigReader() {
  // Cleanup, disconnect from db...
}

ptree & JsonApi::read() {
    std::string s = "Reading configuration from " + m_file_path;

    try {
        boost::property_tree::read_json(m_file_path, m_config);
    } catch(std::exception & e) {
      std::cout << "Failed: " << e.what() <<  std::endl;
      throw;
    }
  return m_config;
}

ptree JsonApi::read(std::string element) {
  ptree tree;
  return tree;
}

ptree & OracleApi::read() {
  return m_config;
}

ptree OracleApi::read(std::string element) {
  ptree tree;
  return tree;
}
