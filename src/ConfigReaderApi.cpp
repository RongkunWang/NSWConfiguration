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
        ERS_DEBUG(5, name << ", " << type);
    }

    return tree;
}

ptree ConfigReaderApi::readROC(std::string element) {
    ptree tree = m_config.get_child("roc_common_config");
    ptree temp = m_config.get_child(element);  // roc specific config
    std::string type;

    for (ptree::iterator iter = temp.begin(); iter != temp.end(); iter++) {
        std::string name = iter->first;
        // Put Opc related FE configuration
        if (name.find("Opc") != std::string::npos) {
            tree.put(name, iter->second.data());
        } else if (name == "rocPllCoreAnalog" || name == "rocCoreDigital") {
            ptree i2ctree = iter->second;

            // Loop over I2c addresses within Roc analog or digital
            for (ptree::iterator iter_addresses = i2ctree.begin();
                iter_addresses != i2ctree.end(); iter_addresses++) {
                std::string address = iter_addresses->first;
                ptree addresstree = iter_addresses->second;

                for (ptree::iterator iter_registers = addresstree.begin();
                    iter_registers != addresstree.end(); iter_registers++) {
                    std::string registername = iter_registers->first;
                    std::string node = name + "." + address + "." + registername;

                    if (!tree.get_optional<std::string>(node).is_initialized()) {  // Check if node exists
                        nsw::ROCConfigBadNode issue(ERS_HERE, node.c_str());
                        ers::error(issue);
                        // throw issue;  // TODO(cyildiz): throw or just error
                    } else {
                        tree.put(node, iter_registers->second.data());
                    }
                }
            }
        } else {
            ERS_LOG("Unknown element in ROC config: " << name);
            // TODO(cyildiz): Handle exception
        }
    }

    return tree;
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
