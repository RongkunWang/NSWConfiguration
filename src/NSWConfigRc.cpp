#include <string>
#include <memory>

#include "NSWConfiguration/NSWConfigRc.h"

void nsw::NSWConfigRc::configure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Config");

    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";

    m_reader = std::make_unique<nsw::ConfigReader>("json://" + base_folder + "integration_config.json");
    m_sender = std::make_unique<nsw::ConfigSender>();

    auto config = m_reader->readConfig();

    m_roc_names = {"A01.ROC_L01_M01"};
    m_vmm_names = {"A01.VMM_L01_M01_00", "A01.VMM_L01_M01_01"};

    configureROCs();
}

void nsw::NSWConfigRc::prepareForRun(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("PrepareForRun");
}

void nsw::NSWConfigRc::stopRecording(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Stop");
}

void nsw::NSWConfigRc::user(const daq::rc::UserCmd& usrCmd) {
  ERS_LOG("User command received: " << usrCmd.commandName());
}

void nsw::NSWConfigRc::subTransition(const daq::rc::SubTransitionCmd& cmd) {
    auto main_transition = cmd.mainTransitionCmd();
    auto sub_transition = cmd.subTransition();

    ERS_LOG("Sub transition received: " << sub_transition << " (mainTransition: " << main_transition << ")");

    // This part should be in sync with NSWTTCConfig application. Some of this steps can also be a regular
    // state transition instead of a subTransition. SubTransitions are called before the main transition
    // Configuration steps are
    // - Configure ROC (NSWConfiguration)
    // - send soft-reset (NSWTTCConfig)
    // - Configure VMM (NSWConfiguration)
    // - send ocr (NSWTTCConfig)
    if (sub_transition == "CONFIGURE_ROC") {
      configureROCs();
    } else if (sub_transition == "CONFIGURE_VMM") {
      configureVMMs();
    } else {
      ERS_LOG("Nothing to do for subTransition");
    }
}

void nsw::NSWConfigRc::configureVMMs() {
    ERS_LOG("Configuring VMMs");

    for (auto vmm_name : m_vmm_names) {
        auto vmmconfig = m_reader->readConfig(vmm_name);
        nsw::VMMConfig vmm(vmmconfig);
        ERS_LOG("Sending config to: " << vmm_name);
        // m_sender->sendVmmConfig(vmm);
    }
}

void nsw::NSWConfigRc::configureROCs() {
    ERS_LOG("Configuring ROCs");
    for (auto roc_name : m_roc_names) {
        auto rocconfig = m_reader->readConfig(roc_name);
        nsw::ROCConfig roc(rocconfig);
        ERS_LOG("Sending config to: " << roc_name);
        // m_sender->sendRocConfig(roc);
    }
}
