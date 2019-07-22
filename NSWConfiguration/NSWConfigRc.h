
#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "RunControl/RunControl.h"
#include "RunControl/Common/RunControlCommands.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"

using boost::property_tree::ptree;

namespace nsw {
class FEBConfig;
}

namespace nsw {
class NSWConfigRc: public daq::rc::Controllable {
 public:
    // override only the needed methods
    explicit NSWConfigRc(bool simulation = false);
    virtual ~NSWConfigRc() noexcept {}

    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    void configure(const daq::rc::TransitionCmd& cmd) override;

    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;

    void stopRecording(const daq::rc::TransitionCmd& cmd) override;

    void unconfigure(const daq::rc::TransitionCmd& cmd) override;

    void user(const daq::rc::UserCmd& cmd) override;

    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd&) override;

 private:
    //! Configure ROCs in all FEBs in m_frontends
    void configureROCs();

    //! Configure VMMs in all FEBs in m_frontends
    void configureVMMs();

    //! Configure all front ends in m_frontends
    void configureFEBs();

    std::unique_ptr<nsw::ConfigReader> m_reader;
    std::unique_ptr<nsw::ConfigSender> m_sender;

    std::map<std::string, FEBConfig> m_frontends;   //! Each element is [frontend_name, frontend_config]

    // Run the program in simulation mode, don't send any configuration
    bool m_simulation;

    // Database connection string
    std::string m_dbcon;
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
