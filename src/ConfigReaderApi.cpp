#include <memory>
#include <vector>
#include <exception>
#include <string>

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Utility.h"

ptree ConfigReaderApi::read(std::string element) {
    if (nsw::getElementType(element) == "VMM") {
        return readVMM(element);
    } else if (nsw::getElementType(element) == "ROC") {
        return readROC(element);
    }
}

ptree ConfigReaderApi::readVMM(std::string element) {
    ptree tree;
    std::cout << "Reading configuration for VMM: " << element << std::endl;

    // Create tree with default values from common config
    tree = m_config.get_child("vmm_common_config");

    // Go over elements and overwrite the values from common config
    ptree temp = m_config.get_child(element);
    for (ptree::iterator iter = temp.begin(); iter != temp.end(); iter++) {
        std::string name = iter->first;
        std::cout << name << ", ";

        // if no child, put as data, otherwise add child
        if (iter->second.empty()) {
            std::cout << "data";
            tree.put(name, iter->second.data());  // This overwrites the value from common config
        } else {  // This is a array, thus should be a channel register
            if (name.find("channel")!= 0) {
                throw std::runtime_error("Expecting channel register, got this instead: " + name);
            }
            tree.erase(name);  // Remove common config value for register
            tree.add_child(name, iter->second);  // Add it as a child tree
            std::cout << "tree";
        }
        std::cout << "\n";
    }

    return tree;
}

ptree ConfigReaderApi::readROC(std::string element) {
    ptree tree;
    // TODO(cyildiz)
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

ptree & OracleApi::read() {
    return m_config;
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
