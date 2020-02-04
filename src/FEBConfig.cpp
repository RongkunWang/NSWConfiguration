#include <string>

#include "boost/optional.hpp"

#include "NSWConfiguration/FEBConfig.h"


nsw::FEBConfig::FEBConfig(ptree config):
        SCAConfig(config),
        m_roc_analog(config.get_child(ROC_ANALOG_NAME), ROC_ANALOG_NAME, ROC_ANALOG_REGISTERS),
        m_roc_digital(config.get_child(ROC_DIGITAL_NAME), ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS) {
    /// A FE can have up to 8 vmms, the config ptree should be constructed with correct number vmms
    for (int i = 0; i < 8; i++) {
        std::string vmmname = "vmm" + std::to_string(i);
        if (config.find(vmmname) != config.not_found()) {
            ERS_DEBUG(3, "VMM id:" << vmmname);
            m_vmms.emplace_back(config.get_child(vmmname));
            m_vmms.back().setName(vmmname);
        }
    }

    /// A FE can have up to 4 tdss, the config ptree should be constructed with correct number vmms
    for (int i = 0; i < 4; i++) {
        std::string tdsname = "tds" + std::to_string(i);
        if (config.find(tdsname) != config.not_found()) {
            ERS_DEBUG(3, "TDS id:" << tdsname);
            m_tdss.emplace_back(config.get_child(tdsname), tdsname, TDS_REGISTERS);
        }
    }
}

void nsw::FEBConfig::dump() {
    m_roc_analog.dump();
    m_roc_digital.dump();
    std::cout << "Number of TDS: " << m_tdss.size() << std::endl;
    for (auto tds : m_tdss) {tds.dump();}
    std::cout << "Number of VMMs: " << m_vmms.size() << std::endl;
}
