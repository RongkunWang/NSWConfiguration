#include "NSWConfiguration/MMFE8Config.h"

nsw::MMFE8Config::MMFE8Config(ptree config):
        FEConfig(config),
        roc_analog(config.get_child(ROC_ANALOG_NAME), ROC_ANALOG_NAME, ROC_ANALOG_REGISTERS),
        roc_digital(config.get_child(ROC_DIGITAL_NAME), ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS) {
    for (int i = 0; i < 8; i++) {
        std::string vmmname = "vmm" + std::to_string(i);
        vmms.emplace_back(config.get_child(vmmname));
        vmms.back().setName(vmmname);
    }
}
