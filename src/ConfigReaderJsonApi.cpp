#include "NSWConfiguration/ConfigReaderJsonApi.h"

#include <stdexcept>
#include <regex>

#include <ers/ers.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>


#include <fmt/core.h>

#include "NSWConfiguration/OKSDeviceHierarchy.h"
#include "NSWConfiguration/Utility.h"


using boost::property_tree::ptree;

JsonApi::JsonApi(std::string file_path, nsw::DeviceMap devices) :
  m_file_path(std::move(file_path)), m_devices(std::move(devices)), m_config(read()) {
    validateDeviceMap();
}

JsonApi::JsonApi(const ptree& tree) :
  m_config(tree) {
}

std::set<std::string> JsonApi::getAllElementNames() const {
    if (not m_devices.empty()) {
        return nsw::oks::getAllDeviceNames(m_devices);
    }
    return getAllElementNamesFromJson();
}

std::set<std::string> JsonApi::getAllElementNamesFromJson() const {
    return nsw::matchRegexpInPtree("MMFE8.*|PFEB.*|SFEB.*|ADDC.*|PadTriggerSCA.*|Router.*|TPCarrier.*|MMTP.*|STGCTP.*|L1DDC.*|.*RimL1DDC.*", m_config);
}


ptree JsonApi::readTPCarrier(const std::string& element) const {
    ERS_LOG("Reading configuration for TP Carrier: " << element);
    ptree tree = m_config.get_child(element);
    return tree;
}

ptree JsonApi::readTP(const std::string& element) const {
    ERS_LOG("Reading configuration for TP: " << element);
    ptree tree = m_config.get_child(element);
    // TODO: Notify TP of disabled ARTs
    return tree;
}

void JsonApi::mergeI2cMasterTree(ptree & specific, ptree & common) const {
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

void JsonApi::mergeVMMTree(ptree & specific, ptree & common) const {
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

ptree JsonApi::readFEB(const std::string& element, size_t nvmm, size_t ntds, size_t vmm_start,
    size_t tds_start) const {
    ptree feb = m_config.get_child(element);
    ptree roc_common = m_config.get_child("roc_common_config");

    // ROC
    if (not m_devices.empty() and not findInTree(m_devices.at("FEB").at(element), [](const std::string& deviceName) {
          return (deviceName.find("ROC") != std::string::npos) and
                 (deviceName.find("sROC") == std::string::npos);
        })) {
        throw std::runtime_error("Do not disable the ROC without disabling the board.");
    }
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
        // VMM disabled
        std::string vmmname = "vmm" + std::to_string(i);
        if (not m_devices.empty() and
            not findInTree(m_devices.at("FEB").at(element), [i](const std::string& deviceName) {
              return (deviceName.find(fmt::format("VMM{}", i)) != std::string::npos) and (deviceName.find("_DeviceID") == std::string::npos);
            })) {
            ERS_LOG("Removing VMM " << vmmname);
            if (feb.get_child_optional(vmmname)) {
                feb.erase(vmmname);
            }
            continue;
        }
        ptree vmm_common = m_config.get_child("vmm_common_config");
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
        if (not m_devices.empty() and
            not findInTree(m_devices.at("FEB").at(element), [i, ntds](const std::string& deviceName) {
              if (ntds == 1) {
                return deviceName.find("TDS") != std::string::npos;
              }
              return deviceName.find(fmt::format("TDS{}", i)) != std::string::npos;
            })) {
            if (feb.get_child_optional(name)) {
                ERS_LOG("Removing TDS " << name);
                feb.erase(name);
            }
            continue;
        }
        ptree specific;
        ptree tds_common = m_config.get_child("tds_common_config");
        if (feb.get_child_optional(name)) {  // If node exists
            specific = feb.get_child(name);
        }
        mergeI2cMasterTree(specific, tds_common);
        feb.put_child(name, tds_common);
    }

    if (not m_devices.empty()) {
        adjustRocConfig(feb, m_devices.at("FEB").at(element));
    }

    return feb;
}

ptree JsonApi::readL1DDC(const std::string& element) const {
    // Read an L1DDC branch from the configuration ptree
    ERS_LOG(fmt::format("JsonApi::readL1DDC, element={}.",element));

    // get configuration for specific board
    ptree feb = m_config.get_child(element);
    const std::string boardType = feb.get<std::string>("boardType", "none");
    if (boardType=="none"){
        nsw::ConfigIssue issue(ERS_HERE, "boardType is unspecified in L1DDC configuration. Check the JSON.");
        ers::fatal(issue);
        throw issue;
    }

    // check for common L1DDC configuration
    const auto common = m_config.get_child_optional("l1ddc_common_config");
    if (!common){
        nsw::ConfigIssue issue(ERS_HERE, "l1ddc_common_config is unspecified in configuration. Check the JSON.");
        ers::fatal(issue);
        throw issue;
    }

    // get the possible GBTx configurations. These don't need to all be provided
    const auto mmg_gbtx0  = common.get().get_child_optional("mmg_gbtx0");
    const auto mmg_gbtx1  = common.get().get_child_optional("mmg_gbtx1");
    const auto mmg_gbtx2  = common.get().get_child_optional("mmg_gbtx2");
    const auto sfeb_gbtx0 = common.get().get_child_optional("sfeb_gbtx0");
    const auto sfeb_gbtx1 = common.get().get_child_optional("sfeb_gbtx1");
    const auto pfeb_gbtx0 = common.get().get_child_optional("pfeb_gbtx0");
    const auto pfeb_gbtx1 = common.get().get_child_optional("pfeb_gbtx1");
    const auto rim_gbtx0 = common.get().get_child_optional("rim_gbtx0");
    const auto rim_gbtx1 = common.get().get_child_optional("rim_gbtx1");

    if (boardType=="mmg"){
        // Check that the required common configuration exists
        if (!mmg_gbtx0){
            nsw::ConfigIssue issue(ERS_HERE, "mmg_gbtx0 is unspecified in common configuration, but there is a mmg type L1DDC to configure. Check the JSON.");
            ers::fatal(issue);
            throw issue;
        }
        feb.put_child("mmg_gbtx0",mmg_gbtx0.get());
    }
    else if (boardType=="rim"){
        // Check that the required common configuration exists
        if (!rim_gbtx0){
            nsw::ConfigIssue issue(ERS_HERE, "rim_gbtx0 is unspecified in common configuration, but there is a rim type L1DDC to configure. Check the JSON.");
            ers::fatal(issue);
            throw issue;
        }
        feb.put_child("rim_gbtx0",rim_gbtx0.get());
    }
    else if (boardType=="sfeb"){
        if (!sfeb_gbtx0||!sfeb_gbtx1){
            nsw::ConfigIssue issue(ERS_HERE, "sfeb_gbtx0 is unspecified in common configuration, but there is a sfeb type L1DDC to configure. Check the JSON.");
            ers::fatal(issue);
            throw issue;
        }
        feb.put_child("sfeb_gbtx0",sfeb_gbtx0.get());
        feb.put_child("sfeb_gbtx1",sfeb_gbtx1.get());
    }
    else if (boardType=="pfeb"){
        if (!pfeb_gbtx0||!pfeb_gbtx1){
            nsw::ConfigIssue issue(ERS_HERE, "pfeb_gbtx0 is unspecified in common configuration, but there is a pfeb type L1DDC to configure. Check the JSON.");
            ers::fatal(issue);
            throw issue;
        }
        feb.put_child("pfeb_gbtx0",pfeb_gbtx0.get());
        feb.put_child("pfeb_gbtx1",pfeb_gbtx1.get());
    }
    else if (boardType=="rim_stgc"){
        nsw::ConfigIssue issue(ERS_HERE, "rim_stgc configuration not implemented yet");
        ers::fatal(issue);
        throw issue;
    }
    else{
        nsw::ConfigIssue issue(ERS_HERE, fmt::format("boardType={} is invalid. Check the JSON.",boardType).c_str());
        ers::fatal(issue);
        throw issue;
    }

    return feb;
}

ptree JsonApi::readADDC(const std::string& element, size_t nart) const {
    // how to dump a json to screen:
    // std::stringstream ss;
    // boost::property_tree::json_parser::write_json(ss, m_config);
    // std::cout << ss.str() << std::endl;

    ptree feb = m_config.get_child(element);

    for ( size_t i = 0; i < nart; i++ ) {
        std::string name = "art" + std::to_string(i);
        if (not m_devices.empty() and
            not findInTree(m_devices.at("ADDC").at(element), [i](const std::string& deviceName) {
              return deviceName.find(fmt::format("ART{}", i)) != std::string::npos;
            })) {
            if (feb.get_child_optional(name)) {
                ERS_LOG("Removing ART " << name);
                feb.erase(name);
            }
            continue;
        }

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

ptree JsonApi::readPadTriggerSCA(const std::string& element) const {
    constexpr std::string_view name{"padtriggerfpga"};
    ptree device = m_config.get_child(element);
    ptree common = m_config.get_child("padtriggerfpga_common_config");
    ptree specific;
    if (device.get_child_optional(std::string(name))) {
      specific = device.get_child(std::string(name));
    }
    mergeI2cMasterTree(specific, common);
    device.put_child(std::string(name), common);
    return device;
}

ptree JsonApi::readRouter(const std::string& element) const {
    //
    // Need to add functionality to overwrite values!
    //
    ptree feb = m_config.get_child(element);
    return feb;
}

ptree JsonApi::read() {
    std::string s = "Reading json configuration from " + m_file_path;
    ERS_LOG(s);

    // temporary objects for reading in JSON file for cleaning
    std::stringstream jsonStringStream;
    std::ifstream inputJSONFile(m_file_path.c_str() );
    std::string line;

    // Clean input JSON file
    while (std::getline(inputJSONFile, line)) {
        // Skip whitespace starting a line
        const auto found = line.find_first_not_of(" \t");
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
        ptree config;
        boost::property_tree::read_json(jsonStringStream, config);
        return config;
    } catch(std::exception & e) {
        nsw::ConfigIssue issue(ERS_HERE, e.what());
        ers::fatal(issue);
        throw issue;
    }
}

ptree& JsonApi::getConfig() {
    return m_config;
}

const ptree& JsonApi::getConfig() const {
    return m_config;
}

void JsonApi::validateDeviceMap() const {
    const auto devicesInMap = getAllElementNames();
    const auto devicesInJson = getAllElementNamesFromJson();
    // Basically any_of but I need the index of the failing test
    for (const auto& name : devicesInMap) {
        if (devicesInJson.find(name) == std::cend(devicesInJson)) {
            throw std::runtime_error(fmt::format("Requested device {} is not contained in JSON", name));
        }
    }
}
