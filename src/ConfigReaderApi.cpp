#include <memory>
#include <vector>
#include <exception>
#include <string>
#include <regex>
#include <set>

#include "boost/foreach.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Utility.h"

ptree ConfigReaderApi::read(std::string element) {
    if (nsw::getElementType(element) == "VMM") {
        return readVMM(element);
    } else if (nsw::getElementType(element) == "ROC") {
        return readROC(element);
    } else if (nsw::getElementType(element) == "TDS") {
        return readTDS(element);
    } else if (nsw::getElementType(element) == "MMFE8") {
        return readMMFE8(element);
    } else if (nsw::getElementType(element) == "PFEB") {
        return readPFEB(element);
    } else if (nsw::getElementType(element) == "SFEB") {
        return readSFEB(element);
    } else if (nsw::getElementType(element) == "TP") {
        return readTP(element);
    } else if (nsw::getElementType(element) == "ADDC") {
        return readADDC(element, 2);
    }
}

std::set<std::string> ConfigReaderApi::getAllElementNames() {
    if (m_config.empty()) {
      read();
    }

    return nsw::matchRegexpInPtree("MMFE8.*|PFEB.*|SFEB.*|ADDC.*", m_config);
}

std::set<std::string> ConfigReaderApi::getElementNames(std::string regexp) {
    std::set<std::string> result;
    std::regex re(regexp);
    auto all = getAllElementNames();
    for (auto el : all) {
        if (std::regex_match(el, re)) {
          result.emplace(el);
        }
    }
    return result;
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

ptree ConfigReaderApi::readTP(std::string element) {
    ERS_LOG("Reading configuration for TP: " << element);
    ptree tree = m_config.get_child(element);

    //for (ptree::iterator iter = registers.begin(); iter != registers.end(); iter++) {
    //  std::cout << iter->first << "\t" << (iter->second).data() << std::endl;
    //}

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

void ConfigReaderApi::mergeI2cMasterTree(ptree & specific, ptree & common) {
    // Loop over I2c addresses within specific tree
    for (ptree::iterator iter_addresses = specific.begin();
        iter_addresses != specific.end(); iter_addresses++) {
        std::string address = iter_addresses->first;
        ptree addresstree = iter_addresses->second;

        // Iterate over registers in I2c address
        for (ptree::iterator iter_registers = addresstree.begin();
            iter_registers != addresstree.end(); iter_registers++) {
            std::string registername = iter_registers->first;
            std::string node = address + "." + registername;

            //  Check if node exists in specific tree, and replace the value from common
            if (!common.get_optional<std::string>(node).is_initialized()) {
                nsw::ConfigBadNode issue(ERS_HERE, node, "i2c element");
                ers::error(issue);
                // throw issue;  // TODO(cyildiz): throw or just error?
            } else {
                common.put(node, iter_registers->second.data());
            }
        }
    }
}

void ConfigReaderApi::mergeVMMTree(ptree & specific, ptree & common) {
    // Iterate over registers in I2c address
    for (ptree::iterator iter_registers = specific.begin();
        iter_registers != specific.end(); iter_registers++) {
        std::string registername = iter_registers->first;

        //  Check if node exists in specific tree, and replace the value from common
        if (!common.get_optional<std::string>(registername).is_initialized()) {
            nsw::ConfigBadNode issue(ERS_HERE, registername, "vmm");
            ers::error(issue);
            // throw issue;  // TODO(cyildiz): throw or just error?
        } else {
            if (registername.find("channel_" == 0)) {
              ptree temp = iter_registers->second;
              common.put_child(registername, temp);
            } else {
              common.put(registername, iter_registers->second.data());
            }
        }
    }
}

ptree ConfigReaderApi::readFEB(std::string element, size_t nvmm, size_t ntds) {
    ptree feb = m_config.get_child(element);
    ptree roc_common = m_config.get_child("roc_common_config");

    // ROC
    for ( auto name : {"rocPllCoreAnalog", "rocCoreDigital" } ) {
        ptree specific;
        if (feb.get_child_optional(name)) {  // If node exists
            specific = feb.get_child(name);
        }
        ptree common = roc_common.get_child(name);
        mergeI2cMasterTree(specific, common);
        feb.put_child(name, common);
    }

    // VMM
    for (size_t i = 0; i < nvmm; i++) {
        ptree vmm_common = m_config.get_child("vmm_common_config");
        std::string vmmname = "vmm" + std::to_string(i);
        ptree specific;
        if (feb.get_child_optional(vmmname)) {  // If node exists
            specific = feb.get_child(vmmname);
        }
        mergeVMMTree(specific, vmm_common);
        vmm_common.put("OpcServerIp", "none");  // TODO(cyildiz): Remove
        vmm_common.put("OpcNodeId", "none");   // TODO(cyildiz): Remove
        feb.put_child(vmmname, vmm_common);
    }

    // If the configuation has more than expected vmms, remove them
    for (size_t i = nvmm; i < 8; i++) {
        std::string vmmname = "vmm" + std::to_string(i);
        ptree vmm;
        if (feb.get_child_optional(vmmname)) {  // If node exists
            nsw::ConfigIssue issue(ERS_HERE, "Too many vmm instances in the configuration, ignoring!");
            ers::warning(issue);
            feb.erase(vmmname);
        }
    }

    for ( int i = 0; i < ntds; i++ ) {
        std::string name = "tds" + std::to_string(i);
        ptree specific;
        ptree tds_common = m_config.get_child("tds_common_config");
        if (feb.get_child_optional(name)) {  // If node exists
            specific = feb.get_child(name);
        }
        mergeI2cMasterTree(specific, tds_common);
        feb.put_child(name, tds_common);
    }

    for (int i = ntds; i < 3; i++) {
        std::string tdsname = "tds" + std::to_string(i);
        ptree tds;
        if (feb.get_child_optional(tdsname)) {  // If node exists
            nsw::ConfigIssue issue(ERS_HERE, "Too many tds instances in the configuration, ignoring!");
            ers::warning(issue);
            feb.erase(tdsname);
        }
    }

    return feb;
}

ptree ConfigReaderApi::readTDS(std::string element) {
    ptree tree = m_config.get_child("tds_common_config");
    ptree temp = m_config.get_child(element);  // tds specific config
    std::string type;

    for (ptree::iterator iter = temp.begin(); iter != temp.end(); iter++) {
        std::string name = iter->first;
        // Put Opc related FE configuration
        if (name.find("Opc") != std::string::npos) {
            tree.put(name, iter->second.data());
        } else if (name == "tds") {
            ptree i2ctree = iter->second;

            // Loop over I2c addresses within tds
            for (ptree::iterator iter_addresses = i2ctree.begin();
                iter_addresses != i2ctree.end(); iter_addresses++) {
                std::string address = iter_addresses->first;
                ptree addresstree = iter_addresses->second;

                for (ptree::iterator iter_registers = addresstree.begin();
                    iter_registers != addresstree.end(); iter_registers++) {
                    std::string registername = iter_registers->first;
                    std::string node = name + "." + address + "." + registername;

                    if (!tree.get_optional<std::string>(node).is_initialized()) {  // Check if node exists
                        nsw::TDSConfigBadNode issue(ERS_HERE, node.c_str());
                        ers::error(issue);
                        // throw issue;  // TODO(cyildiz): throw or just error
                    } else {
                        tree.put(node, iter_registers->second.data());
                    }
                }
            }
        } else {
            ERS_LOG("Unknown element in TDS config: " << name);
            // TODO(cyildiz): Handle exception
        }
    }

    return tree;
}

ptree ConfigReaderApi::readADDC(std::string element, size_t nart) {

    // how to dump a json to screen:
    // std::stringstream ss;
    // boost::property_tree::json_parser::write_json(ss, m_config);
    // std::cout << ss.str() << std::endl;

    ptree feb = m_config.get_child(element);
    ptree art_common = m_config.get_child("art_common_config");
    art_common.put("OpcServerIp", "none");  // TODO(tuna): Do what Cenk does for VMM
    art_common.put("OpcNodeId",   "none");  // TODO(tuna): Do what Cenk does for VMM

    for ( size_t i = 0; i < nart; i++ ) {
        std::string name = "art" + std::to_string(i);
        ptree specific;
        if (feb.get_child_optional(name)) {  // If node exists
            specific = feb.get_child(name);
        }
        mergeI2cMasterTree(specific, art_common);
        feb.put_child(name, art_common);
    }

    return feb;
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
