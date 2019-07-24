#include <string>

#include "boost/optional.hpp"

#include "NSWConfiguration/TPConfig.h"


nsw::TPConfig::TPConfig(ptree config):
        FEConfig(config)
        // ROC_ANALOG_NAME, etc need to be changed
        // m_tp_analog(config.get_child(ROC_ANALOG_NAME), ROC_ANALOG_NAME, ROC_ANALOG_REGISTERS),
        // m_tp_digital(config.get_child(ROC_DIGITAL_NAME), ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS)
        {
        std::cout << "TPConfig::TPConfig()" << std::endl;
    /// A FE can have up to 8 vmms, the config ptree should be constructed with correct number vmms
    // for (int i = 0; i < 8; i++) {
    //     std::string vmmname = "vmm" + std::to_string(i);
    //     if (config.find(vmmname) != config.not_found()) {
    //         ERS_DEBUG(3, "VMM id:" << vmmname);
    //         m_vmms.emplace_back(config.get_child(vmmname));
    //         m_vmms.back().setName(vmmname);
    //     }
    // }

    /// A FE can have up to 3 tdss, the config ptree should be constructed with correct number vmms
    for (int i = 0; i < 16; i++) {
        std::string mastername = "i2cMaster" + std::to_string(i);
        if (config.find(mastername) != config.not_found()) {
            ERS_DEBUG(3, "I2c master id:" << mastername);
            m_config_elements.emplace_back(config.get_child(mastername), mastername, TP_REGISTERS);
        }
    }
}

void nsw::TPConfig::dump() {
    // m_tp_analog.dump();
    // m_tp_digital.dump();
    // std::cout << "Number of TDS: " << m_tdss.size() << std::endl;
    for (auto config_element : m_config_elements) {config_element.dump();}
    // std::cout << "Number of VMMs: " << m_vmms.size() << std::endl;
}
