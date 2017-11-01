#include <memory>
#include <exception>
#include "NSWConfiguration/ConfigReader.h"

ConfigReader::ConfigReader(const std::string connection_string): m_connection_string(connection_string) {
    // Open db or json file
    auto pos = m_connection_string.find("file://");
    if (pos != std::string::npos) {
        std::string file_path = m_connection_string.substr(std::string("file://").length());
        if (file_path.substr(file_path.length()-4,4) == "json")
            m_api = std::make_unique<JsonApi> (file_path);
        else
            m_api = std::make_unique<XmlApi> (file_path);
    } else {
        m_api = std::make_unique<OracleApi> (m_connection_string);
    }
}

ConfigReader::~ConfigReader() {
  // Cleanup, disconnect from db...
}

ptree & JsonApi::read() {
    std::string s = "Reading json configuration from " + m_file_path;
    std::cout << s << std::endl;

    try {
        boost::property_tree::read_json(m_file_path, m_config);
    } catch(std::exception & e) {
      std::cout << "Failed: " << e.what() <<  std::endl;
      throw;
    }
  return m_config;
}

ptree & XmlApi::read() {
    std::string s = "Reading xml configuration from " + m_file_path;
    std::cout << s << std::endl;

    try {
        boost::property_tree::read_xml(m_file_path, m_config);
    } catch(std::exception & e) {
      std::cout << "Failed: " << e.what() <<  std::endl;
      throw;
    }
  return m_config;
}

ptree XmlApi::read(std::string element) {
  ptree tree;
  return tree;
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
