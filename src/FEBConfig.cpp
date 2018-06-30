#include <string>

#include "boost/optional.hpp"

#include "NSWConfiguration/FEBConfig.h"


nsw::FEBConfig::FEBConfig(ptree config):
        FEConfig(config),
        m_roc_analog(config.get_child(ROC_ANALOG_NAME), ROC_ANALOG_NAME, ROC_ANALOG_REGISTERS),
        m_roc_digital(config.get_child(ROC_DIGITAL_NAME), ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS) {
    /// A FE can have up to 8 vmms, the config ptree should be constructed with correct number vmms
    for (int i = 0; i < 8; i++) {
        std::string vmmname = "vmm" + std::to_string(i);
        if (config.find(vmmname) != config.not_found()) {
            m_vmms.emplace_back(config.get_child(vmmname));
            m_vmms.back().setName(vmmname);
        }
    }

    /// A FE can have up to 3 tdss, the config ptree should be constructed with correct number vmms
    for (int i = 0; i < 3; i++) {
        std::string tdsname = "tds" + std::to_string(i);
        if (config.find(tdsname) != config.not_found()) {
            m_tdss.emplace_back(config.get_child(tdsname), tdsname, TDS_REGISTERS);
        }
    }
}
