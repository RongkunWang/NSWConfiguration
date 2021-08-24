#include "NSWConfiguration/ConfigReaderApi.h"

#include "NSWConfiguration/Utility.h"

#include <memory>
#include <vector>
#include <exception>
#include <regex>

#include "ers/ers.h"

#include "boost/property_tree/json_parser.hpp"

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

std::set<std::string> ConfigReaderApi::getElementNames(const std::string& regexp) const {
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

void ConfigReaderApi::mergeTree(const ptree& specific, ptree& common) const {
  // Iterate over registers in I2c address
  for (const auto& [name, value] : specific) {
    //  Check if node exists in specific tree, and replace the value from common
    if (!common.get_optional<std::string>(name).is_initialized()) {
      nsw::ConfigBadNodeGeneral issue(ERS_HERE, name);
      ers::error(issue);

      std::stringstream ss;
      ss << "Problematic common ptree: \n";
      boost::property_tree::json_parser::write_json(ss, common);
      ss << "Problematic specific ptree: \n";
      boost::property_tree::json_parser::write_json(ss, specific);
      std::cerr << ss.str() << '\n';

      throw issue;
    }
    if (not value.empty() and value.data().empty()) {
      mergeTree(value, common.get_child(name));
    }
    else {
        common.put(name, value.data());
    }
  }
}
