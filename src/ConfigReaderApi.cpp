#include <memory>
#include <vector>
#include <exception>
#include <string>

#include "ers/ers.h"

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
    ERS_LOG("Reading configuration for VMM: " << element);

    // Create tree with default values from common config
    tree = m_config.get_child("vmm_common_config");

    // Go over elements and overwrite the values from common config
    ptree temp = m_config.get_child(element);
    for (ptree::iterator iter = temp.begin(); iter != temp.end(); iter++) {
        std::string name = iter->first;
        std::string type;

        // if no child, put as data, otherwise add child
        if (iter->second.empty()) {
            type = "data";
            tree.put(name, iter->second.data());  // This overwrites the value from common config
        } else {  // This is a array, thus should be a channel register
            if (name.find("channel")!= 0) {
                throw std::runtime_error("Expecting channel register, got this instead: " + name);
            }
            tree.erase(name);  // Remove common config value for register
            tree.add_child(name, iter->second);  // Add it as a child tree
            type = "tree";
        }
        ERS_DEBUG(5, name << ", " << type );
    }

    return tree;
}

ptree ConfigReaderApi::readROC(std::string element) {
    // TODO(cyildiz): First read common config, then put element specific config
    ptree tree = m_config.get_child("roc_common_config");
    ptree roc = m_config.get_child(element);  // roc specific config

    // return tree;
    return roc;
}

ptree & JsonApi::read() {
    std::string s = "Reading json configuration from " + m_file_path;
    ERS_LOG(s);

    try {
        boost::property_tree::read_json(m_file_path, m_config);
    } catch(std::exception & e) {
        ERS_LOG("Failed: " << e.what());  // TODO(cyildiz): ers exception
        throw;
    }
  return m_config;
}

ptree & XmlApi::read() {
    std::string s = "Reading xml configuration from " + m_file_path;
    ERS_LOG(s);

    try {
        boost::property_tree::read_xml(m_file_path, m_config);
    } catch(std::exception & e) {
        ERS_LOG("Failed: " << e.what());  // TODO(cyildiz): ers exception
        throw;
    }
    return m_config;
}

ptree & OracleApi::read() {
    return m_config;
}

ptree & OksApi::read() {
    std::string s = "Reading oks configuration from " + m_file_path;
    ERS_LOG(s);

    try {
      ERS_LOG("read oks dummy ");
    } catch(std::exception & e) {
      ERS_LOG("Failed: " << e.what());
      throw;
    }
  return m_config;
}
