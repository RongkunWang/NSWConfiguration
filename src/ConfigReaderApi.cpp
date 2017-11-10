#include <memory>
#include <vector>
#include <exception>
#include <string>

#include "NSWConfiguration/ConfigReaderApi.h"

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
