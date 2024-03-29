#include "NSWConfiguration/ConfigReaderApi.h"

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <memory>
#include <vector>
#include <exception>
#include <regex>

#include <ers/ers.h>

#include <fmt/core.h>

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
    } else if (type == "STGCTP") {
        return readSTGCTP(element);
    } else if (type == "TP") {
        return readTP(element);
    } else if (type == "L1DDC" || type == "RimL1DDC") {
        return readL1DDC(element);
    } else if (type == "ADDC") {
        return readADDC(element, 2);
    } else if (type == "PadTrigger") {
        return readPadTrigger(element);
    } else if (type == "Router") {
        return readRouter(element);
    }
    throw std::runtime_error(fmt::format("Do not know devices of type {}", element));
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

void ConfigReaderApi::adjustRocConfig(boost::property_tree::ptree& config, const boost::property_tree::ptree& devices) {
    const auto type = nsw::getElementType(config.get<std::string>("OpcNodeId"));

    // Disable VMMs
    for (std::size_t ivmm = 0; ivmm < nsw::MAX_NUMBER_OF_VMM; ivmm++) {
        if (not findInTree(devices, [ivmm](const std::string& name) {
              // Psuedodevice VMM0/1_DeviceID
              return (name.find(fmt::format("VMM{}", ivmm)) != std::string::npos) and (name.find("_DeviceID") == std::string::npos);
            })) {
            const auto rocVmmId = getRocVmmId(type, ivmm);
            config.put<unsigned int>(fmt::format("rocCoreDigital.reg008vmmEnable.vmm{}", rocVmmId), 0);
            for (unsigned int isroc = 0; isroc < nsw::roc::NUM_SROCS; isroc++) {
                constexpr unsigned int firstReg = 2;
                config.put<unsigned int>(fmt::format("rocCoreDigital.reg00{}sRoc{}VmmConnections.vmm{}", isroc + firstReg, isroc, rocVmmId), 0);
            }
        }
    }

    // Disable sROCs
    for (unsigned int isroc = 0; isroc < nsw::roc::NUM_SROCS; isroc++) {
        if (not findInTree(devices, [isroc](const std::string& name) {
              return name.find(fmt::format("sROC{}", isroc)) != std::string::npos;
            })) {
            // Do not disable sROC to prevent noisy elinks. I leave the code here if we decide to adjust this later
            // config.put<unsigned int>(fmt::format("rocCoreDigital.reg007sRocEnable.enableSROC{}", isroc), 0);
            for (std::size_t ivmm = 0; ivmm < nsw::MAX_NUMBER_OF_VMM; ivmm++) {
                constexpr unsigned int firstReg = 2;
                config.put<unsigned int>(fmt::format("rocCoreDigital.reg00{}sRoc{}VmmConnections.vmm{}", isroc + firstReg, isroc, ivmm), 0);
            }
        }
    }
}

std::size_t ConfigReaderApi::getRocVmmId(const std::string_view type, const std::size_t ivmm)
{
    if (type == "SFEB6" or type == "SFEB8" or type == "PFEB") {
        switch (ivmm) {
            case 0: return 2;
            case 1: return 3;
            case 2: return 0;
            case 3: return 1;
            case 4: return 5;
            case 5: return 4;
            default: return ivmm;
        }
    }
    return ivmm;
}
