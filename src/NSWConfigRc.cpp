#include "NSWConfiguration/NSWConfigRc.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfigurationDal/NSWConfigApplication.h"

#include <ers/ers.h>

nsw::NSWConfigRc::NSWConfigRc(bool simulation) : m_simulation {simulation} {
    ERS_LOG("Constructing NSWConfigRc instance");
    if (m_simulation) {
        ERS_INFO("Running in simulation mode, no configuration will be sent");
        m_simulation_lock = true;
    }
}

bool nsw::NSWConfigRc::simulationFromIS() {
    // Grab the simulation bool from IS
    // Can manually write to this variable from the command line:
    // > is_write -p part-BB5-Calib -n Setup.NSW.simulation -t Boolean -v 1 -i 0
    if (is_dictionary->contains("Setup.NSW.simulation") ){
        ISInfoDynAny any;
        is_dictionary->getValue("Setup.NSW.simulation", any);
        auto val = any.getAttributeValue<bool>(0);
        ERS_INFO("Simulation from IS: " << val);
        return val;
    }
    return false;
}

void nsw::NSWConfigRc::configure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");
    // Retrieving the configuration db
    daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
    const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
    const nsw::dal::NSWConfigApplication* nswConfigApp = rcBase.cast<nsw::dal::NSWConfigApplication>();

    // Retrieve the ipc partition
    m_ipcpartition = rcSvc.getIPCPartition();

    // Get the IS dictionary for the current partition
    is_dictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

    if (!m_simulation_lock) {
        m_simulation = simulationFromIS();
    }

    m_NSWConfig = std::make_unique<NSWConfig>(m_simulation);
    m_NSWConfig->readConf(nswConfigApp);
    ERS_LOG("End");
}

void nsw::NSWConfigRc::connect(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");
    m_NSWConfig->configureRc();
    ERS_LOG("End");
}

void nsw::NSWConfigRc::prepareForRun(const daq::rc::TransitionCmd& cmd) {
    ERS_LOG("Start");
    m_NSWConfig->startRc();
    ERS_LOG("End");
}

void nsw::NSWConfigRc::stopRecording(const daq::rc::TransitionCmd& cmd) {
    ERS_LOG("Start");
    m_NSWConfig->stopRc();
    ERS_LOG("End");
}

void nsw::NSWConfigRc::disconnect(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");
    m_NSWConfig->unconfigureRc();
    ERS_INFO("End");
}

void nsw::NSWConfigRc::unconfigure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");
    ERS_INFO("End");
}

void nsw::NSWConfigRc::user(const daq::rc::UserCmd& usrCmd) {
  ERS_LOG("User command received: " << usrCmd.commandName());
  if (usrCmd.commandName() == "enableVmmCaptureInputs")
  {
    m_NSWConfig->enableVmmCaptureInputs();
  }
}

void nsw::NSWConfigRc::subTransition(const daq::rc::SubTransitionCmd& cmd) {
    auto main_transition = cmd.mainTransitionCmd();
    auto sub_transition = cmd.subTransition();

    ERS_LOG("Sub transition received: " << sub_transition << " (mainTransition: " << main_transition << ")");

    // This part should be in sync with NSWTTCConfig application. Some of this steps can also be a regular
    // state transition instead of a subTransition. SubTransitions are called before the main transition
    // This is not used in current software version, it may be used if one requires to configure different
    // boards at different times, instead of configuring everything at "configure" step.
    /*if (sub_transition == "CONFIGURE_ROC") {
      // configureROCs();
    } else if (sub_transition == "CONFIGURE_VMM") {
      // configureVMMs();
    } else {
      ERS_LOG("Nothing to do for subTransition" << sub_transition);
    }*/
}


