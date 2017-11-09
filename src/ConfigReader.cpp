#include <memory>
#include <exception>
#include <string>

#include "NSWConfiguration/ConfigReader.h"

ConfigReader::ConfigReader(const std::string connection_string): m_connection_string(connection_string) {
    // Open db or json file
    if (m_connection_string.find("json://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("json://").length());
        m_api = std::make_unique<JsonApi> (file_path);
    } else  if (m_connection_string.find("xml://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("xml://").length());
        m_api = std::make_unique<XmlApi> (file_path);
    } else if (m_connection_string.find("oksconfig:") == 0) {
        m_api = std::make_unique<OksApi> (m_connection_string);
    } else if (m_connection_string.find("oracle:") == 0) {
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

ptree & OksApi::read() {
    std::string s = "Reading oks configuration from " + m_file_path;
    std::cout << s << std::endl;

    try {
      std::cout << "read oks dummy " << std::endl;
    } catch(std::exception & e) {
      std::cout << "Failed: " << e.what() <<  std::endl;
      throw;
    }
  return m_config;
}

ptree OksApi::read(std::string element) {
    ptree tree;
    return tree;
}
