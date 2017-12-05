#include <string>
#include <memory>

#include "NSWConfiguration/NSWConfigRc.h"

void nsw::NSWConfigRc::configure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Config");

    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";

    reader = std::make_unique<nsw::ConfigReader>("json://" + base_folder + "dummy_config.json");
    sender = std::make_unique<nsw::ConfigSender>();

    auto config = reader->readConfig();

    auto vmmconfig0 = reader->readConfig("A01.VMM_L01_M01_00");

    ERS_INFO("TTC: Start Configuration Mode");

    nsw::VMMConfig vmm0(vmmconfig0);

    sender->sendVmmConfig(vmm0);  // Get opc address and send config.
}

void nsw::NSWConfigRc::prepareForRun(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("PrepareForRun");

    ERS_INFO("TTC: Stop Configuration Mode");
}

void nsw::NSWConfigRc::stopRecording(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Stop");
}
