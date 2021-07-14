#include "NSWConfiguration/DBToValueMapping.h"
#include "NSWConfiguration/Constants.h"
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace nsw::Oracle {
  std::unordered_map<std::string, std::string> transform(
    const std::string& type,
    const std::string& name,
    const std::string& value) {
    if (type == "ROC") {
      return Internal::transformRoc(name, value);
    }
    if (type == "ROC_VMM") {
      return Internal::transformRocVmm(name, value);
    }
    if (type == "ROC_TDC") {
      return Internal::transformRocTdc(name, value);
    }
    throw std::runtime_error("Unkown type " + type);
  }

  namespace Internal {
    std::unordered_map<std::string, std::string> transformRoc(
      const std::string& name,
      const std::string& value) {
      const std::string& newName = ROC_MAPPING.at(name);
      return [&newName,
              &value,
              &name]() -> std::unordered_map<std::string, std::string> {
        if (name == "phase40MHz" or name == "phase160MHz") {
          // triplicate
          return {{newName + "_0", value},
                  {newName + "_1", value},
                  {newName + "_2", value}};
        }
        // if (name.find("vmmConnections") != std::string::npos) {
        //   // split VMMs
        //   std::unordered_map<std::string, std::string> result;
        //   std::generate_n(std::inserter(result, std::begin(result)),
        //                   nsw::MAX_NUMBER_OF_VMM,
        //                   [&newName, &value, count = 0u]() mutable {
        //                     const std::string key =
        //                       newName + ".vmm" + std::to_string(count);
        //                     const std::string newValue =
        //                       std::to_string((std::stoul(value) >> count) & 1u);
        //                     count++;
        //                     return std::make_pair(key, newValue);
        //                   });
        //   return result;
        // }
        return {{newName, value}};
      }();
    }

    std::unordered_map<std::string, std::string> transformRocVmm(
      const std::string& name,
      const std::string& value) {
      return {{ROC_VMM_MAPPING.at(name), value}};
    }

    std::unordered_map<std::string, std::string> transformRocTdc(
      const std::string& name,
      const std::string& value) {
      return {{ROC_TDC_MAPPING.at(name), value}};
    }
  }  // namespace Internal
}  // namespace nsw::Oracle