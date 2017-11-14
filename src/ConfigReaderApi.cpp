#include <memory>
#include <vector>
#include <exception>
#include <string>

#include "NSWConfiguration/ConfigReaderApi.h"

ptree ConfigReaderApi::read(std::string element) {
    ptree tree;
    std::cout << "Reading configuration for " << element << std::endl;

    // First put common config, then override with vmm specific config
    tree = m_config.get_child("common_config");

    ptree temp = m_config.get_child(element);
    for (ptree::iterator iter = temp.begin(); iter != temp.end(); iter++) {
        std::string name = iter->first;
        std::cout << name << ", ";

        // if no child, put as data, otherwise add child
        if (iter->second.empty()) {
            std::cout << "data";
            tree.put(name, iter->second.data());
        } else {  // An array, instead of data points
            // TODO(cyildiz): Check if name is one of channel registers:
            // {"sc", "sl", "st", "sth", "sm", "smx", "sd", "sz10b", "sz8b", "sz6b"}
            tree.erase(name);  // Remove sd, as it will be added as a child tree
            tree.add_child(name, iter->second);
            std::cout << "tree";
        }
        std::cout << "\n";
    }

    return tree;
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
