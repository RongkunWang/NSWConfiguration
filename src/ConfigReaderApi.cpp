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
    if (nsw::getElementType(element) == "MMFE8") {
        return readMMFE8(element);
    } else if (nsw::getElementType(element) == "PFEB") {
        return readPFEB(element);
    } else if (nsw::getElementType(element) == "SFEB_old") {
        return readSFEB(element, 3);
    } else if (nsw::getElementType(element) == "SFEB") {
        return readSFEB(element, 4);
    } else if (nsw::getElementType(element) == "TP") {
        return readTP(element);
    } else if (nsw::getElementType(element) == "ADDC") {
        return readADDC(element, 2);
    } else if (nsw::getElementType(element) == "PadTriggerSCA") {
        return readPadTriggerSCA(element);
    } else if (nsw::getElementType(element) == "Router") {
        return readRouter(element);
    }
}

std::set<std::string> ConfigReaderApi::getAllElementNames() {
    if (m_config.empty()) {
      read();
    }

    return nsw::matchRegexpInPtree("MMFE8.*|PFEB.*|SFEB.*|ADDC.*|PadTriggerSCA.*|Router.*|MMTP.*|STGCTP.*", m_config);
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

ptree ConfigReaderApi::readTP(std::string element) {
    ERS_LOG("Reading configuration for TP: " << element);
    ptree tree = m_config.get_child(element);

    // for (ptree::iterator iter = registers.begin(); iter != registers.end(); iter++) {
    //   std::cout << iter->first << "\t" << (iter->second).data() << std::endl;
    // }

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
                nsw::ConfigBadNode issue(ERS_HERE, node, "i2c master. See err file for details");
                ers::error(issue);

                std::stringstream ss;
                ss << "Problematic i2c common ptree: " << std::endl;
                boost::property_tree::json_parser::write_json(ss, common);
                ss << "Problematic i2c specific ptree: " << std::endl;
                boost::property_tree::json_parser::write_json(ss, specific);
                std::cerr << ss.str() << std::endl;

                throw issue;
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

            std::stringstream ss;
            ss << "Problematic vmm common ptree: " << std::endl;
            boost::property_tree::json_parser::write_json(ss, common);
            ss << "Problematic vmm specific ptree: " << std::endl;
            boost::property_tree::json_parser::write_json(ss, specific);
            std::cerr << ss.str() << std::endl;

            throw issue;
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

        ERS_DEBUG(4, "Merging " << name << " ptree");
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

ptree ConfigReaderApi::readADDC(std::string element, size_t nart) {

    // how to dump a json to screen:
    // std::stringstream ss;
    // boost::property_tree::json_parser::write_json(ss, m_config);
    // std::cout << ss.str() << std::endl;

    ptree feb = m_config.get_child(element);

    for ( size_t i = 0; i < nart; i++ ) {

        std::string name = "art" + std::to_string(i);

        // check for ART-specific configuration
        ptree specific;
        ptree common = m_config.get_child("art_common_config");
        if (feb.get_child_optional(name))
            specific = feb.get_child(name);

        // top-level registers
        // i.e. not art_core and art_ps, which are i2c master trees
        for (auto iter: specific) {
            std::string address = iter.first;
            if (address != "art_core" && address != "art_ps")
                common.put_child(address, iter.second);
        }

        // art_core and art_ps
        for ( auto name_i2c : {"art_core", "art_ps"} ) {
            ptree specific_i2c;
            ptree common_i2c = common.get_child(name_i2c);
            if (specific.get_child_optional(name_i2c))
                specific_i2c = specific.get_child(name_i2c);
            mergeI2cMasterTree(specific_i2c, common_i2c);
            common.put_child(name_i2c, common_i2c);
        }

        feb.put_child(name, common);
    }

    return feb;
}

ptree ConfigReaderApi::readPadTriggerSCA(std::string element) {
    //
    // Need to add functionality to overwrite values!
    //
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree ConfigReaderApi::readRouter(std::string element) {
    //
    // Need to add functionality to overwrite values!
    //
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree & JsonApi::read() {
    std::string s = "Reading json configuration from " + m_file_path;

    ERS_LOG(s);
    try {
        boost::property_tree::read_json(m_file_path, m_config);
    } catch(std::exception & e) {
        nsw::ConfigIssue issue(ERS_HERE, e.what());
        ers::fatal(issue);
        throw issue;
    }
  return m_config;
}

ptree & XmlApi::read() {
    std::string s = "Reading xml configuration from " + m_file_path;
    ERS_LOG(s);

    try {
        boost::property_tree::read_xml(m_file_path, m_config);
    } catch(std::exception & e) {
        nsw::ConfigIssue issue(ERS_HERE, e.what());
        ers::fatal(issue);
        throw issue;
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
      nsw::ConfigIssue issue(ERS_HERE, e.what());
      ers::fatal(issue);
      throw issue;
    }
  return m_config;
}
