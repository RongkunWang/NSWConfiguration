#include "NSWConfiguration/ConfigReaderApi.h"

#include "NSWConfiguration/Utility.h"

#include <memory>
#include <vector>
#include <exception>
#include <regex>

#include "ers/ers.h"

#include "boost/foreach.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/xml_parser.hpp"

using boost::property_tree::ptree;

ptree ConfigReaderApi::read(const std::string& element) {
    const std::string type = nsw::getElementType(element);
    ERS_DEBUG(2, "==> Reading element=" << element << " as type=" << type);
    if (type == "MMFE8") {
        return readMMFE8(element);
    } else if (type == "PFEB") {
        return readPFEB(element);
    } else if (type == "SFEB_old") {
        return readSFEB(element, 3);
    } else if (type == "SFEB") {
        ERS_LOG("WARNING!! You are using deprecated SFEB type. Please switch to use SFEB8_XXX instead of " << element);
        return readSFEB(element, 4);
    } else if (type == "SFEB8") {
        return readSFEB(element, 4);
    } else if (type == "SFEB6") {
        return readSFEB6(element);
    } else if (type == "TPCarrier") {
        return readTPCarrier(element);
    } else if (type == "TP") {
        return readTP(element);
    } else if (type == "L1DDC") {
        return readL1DDC(element);
    } else if (type == "ADDC") {
        return readADDC(element, 2);
    } else if (type == "PadTriggerSCA") {
        return readPadTriggerSCA(element);
    } else if (type == "Router") {
        return readRouter(element);
    }
}

std::set<std::string> ConfigReaderApi::getAllElementNames() {
    if (m_config.empty()) {
      read();
    }

    return nsw::matchRegexpInPtree("MMFE8.*|PFEB.*|SFEB.*|ADDC.*|PadTriggerSCA.*|Router.*|TPCarrier.*|MMTP.*|STGCTP.*|L1DDC.*", m_config);
}

std::set<std::string> ConfigReaderApi::getElementNames(const std::string& regexp) {
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

ptree ConfigReaderApi::readTPCarrier(const std::string& element) const {
    ERS_LOG("Reading configuration for TP Carrier: " << element);
    ptree tree = m_config.get_child(element);
    return tree;
}

ptree ConfigReaderApi::readTP(const std::string& element) const {
    ERS_LOG("Reading configuration for TP: " << element);
    ptree tree = m_config.get_child(element);
    return tree;
}

void ConfigReaderApi::mergeI2cMasterTree(ptree & specific, ptree & common) const {
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

void ConfigReaderApi::mergeVMMTree(ptree & specific, ptree & common) const {
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
            if (registername.find("channel_") == 0) {
              ptree temp = iter_registers->second;
              common.put_child(registername, temp);
            } else {
              common.put(registername, iter_registers->second.data());
            }
        }
    }
}

ptree ConfigReaderApi::readFEB(const std::string& element, size_t nvmm, size_t ntds, size_t vmm_start,
    size_t tds_start) const {
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
    // If the configuation has more than expected vmms, remove them
    for (size_t i = 0; i < nsw::MAX_NUMBER_OF_VMM; i++) {
        if (i >= vmm_start && i < nvmm) continue;
        std::string vmmname = "vmm" + std::to_string(i);
        if (feb.get_child_optional(vmmname)) {  // If node exists
            nsw::ConfigIssue issue(ERS_HERE, "Too many vmm instances in the configuration, ignoring!");
            ers::warning(issue);
            feb.erase(vmmname);
        }
    }

    // fill for the remaining vmms
    for (size_t i = vmm_start; i < nvmm; i++) {
        ptree vmm_common = m_config.get_child("vmm_common_config");
        std::string vmmname = "vmm" + std::to_string(i);
        ptree specific;
        if (feb.get_child_optional(vmmname)) {  // If node exists
            specific = feb.get_child(vmmname);
        }
        mergeVMMTree(specific, vmm_common);
        feb.put_child(vmmname, vmm_common);
    }

    // TDS
    // If the configuation has more than expected tds, remove them
    for (size_t i = 0; i < nsw::MAX_NUMBER_OF_TDS; i++) {
        if (i >= tds_start && i < ntds) continue;
        std::string tdsname = "tds" + std::to_string(i);
        if (feb.get_child_optional(tdsname)) {  // If node exists
            nsw::ConfigIssue issue(ERS_HERE, "Too many tds instances in the configuration, ignoring!");
            ers::warning(issue);
            feb.erase(tdsname);
        }
    }
    for (size_t i = tds_start; i < ntds; i++) {
        std::string name = "tds" + std::to_string(i);
        ptree specific;
        ptree tds_common = m_config.get_child("tds_common_config");
        if (feb.get_child_optional(name)) {  // If node exists
            specific = feb.get_child(name);
        }
        mergeI2cMasterTree(specific, tds_common);
        feb.put_child(name, tds_common);
    }


    return feb;
}

ptree ConfigReaderApi::readL1DDC(const std::string& element) const {
    // Read an L1DDC branch from the configuration ptree
    ERS_LOG("ConfigReaderApi::readL1DDC, element=" << element);
    // ptree with configuration for just this element
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree ConfigReaderApi::readADDC(const std::string& element, size_t nart) const {
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
        for (auto iter : specific) {
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

ptree ConfigReaderApi::readPadTriggerSCA(const std::string& element) const {
    //
    // Need to add functionality to overwrite values!
    //
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree ConfigReaderApi::readRouter(const std::string& element) const {
    //
    // Need to add functionality to overwrite values!
    //
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree & JsonApi::read() {
    std::string s = "Reading json configuration from " + m_file_path;
    ERS_LOG(s);

    // temporary objects for reading in JSON file for cleaning
    std::stringstream jsonStringStream;
    std::ifstream inputJSONFile(m_file_path.c_str() );
    std::string line;
    int found;

    // Clean input JSON file
    while (std::getline(inputJSONFile, line)) {
        // Skip whitespace starting a line
        found = line.find_first_not_of(" \t");
        // Remove lines that start with a "/" or "#"
        if (found != std::string::npos && line[found] == '/' ) continue;
        if (found != std::string::npos && line[found] == '#' ) continue;
        jsonStringStream << line << "\r\n";
    }

    // Converting to string for trans-line cleaning
    std::string jsonString(jsonStringStream.str());

    // Removing comments that come after a comma and whitespace
    jsonString = std::regex_replace(jsonString, std::regex("(\\S)\\s*\\/\\/.*"), "$1");
    jsonString = std::regex_replace(jsonString, std::regex("(\\S)\\s*#.*"), "$1");

    // Removing any commas that are followed by white space and then either a } or ]
    jsonString = std::regex_replace(jsonString, std::regex(",(?=\\s*[}\\]])"), "");

    // Converting back to a stringstream to make read_json() happy
    jsonStringStream.str(jsonString);

    try {
        boost::property_tree::read_json(jsonStringStream, m_config);
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

OracleApi::OracleApi(const std::string& configuration) :
  m_db_connection(getDbConnectionString(configuration)),
  m_config_set(getConfigSet(configuration)),
  m_occi_env(oracle::occi::Environment::createEnvironment()),
  m_occi_con(m_occi_env->createConnection(m_db_user_name,
                                          m_db_password,
                                          m_db_connection),
             OcciConnectionDeleter{m_occi_env}),
  m_device_hierarchy(buildHierarchyTree()) {}

void OracleApi::testConfigurationString(const std::string& configuration) {
  if (configuration.find('|') == std::string::npos) {
    nsw::ConfigIssue issue(
      ERS_HERE,
      "DB configuration string does not contain '|' seperator between DB "
      "connection and config set. Format should be <connection>|<config set>.");
    ers::fatal(issue);
    throw issue;
  }
}

std::string OracleApi::getDbConnectionString(const std::string& configuration) {
  testConfigurationString(configuration);
  return configuration.substr(0, configuration.find('|'));
}

std::string OracleApi::getConfigSet(const std::string& configuration) {
  testConfigurationString(configuration);
  return configuration.substr(configuration.find('|') + 1);
}

std::vector<OracleApi::DeviceHierarchyTable> OracleApi::getDevices(
  const std::string& query) {
      // TODO
      return executeQuery<OracleApi::DeviceHierarchyTable>(query);
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

ptree & PtreeApi::read() {
  return m_config;
}
