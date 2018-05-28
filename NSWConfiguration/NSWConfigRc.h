
#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <string>
#include <vector>
#include <memory>

#include "RunControl/RunControl.h"
#include "RunControl/Common/RunControlCommands.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"

using boost::property_tree::ptree;

namespace nsw {
class NSWConfigRc: public daq::rc::Controllable {
 public:
    // override only the needed methods
    NSWConfigRc() {}
    virtual ~NSWConfigRc() noexcept {}
    void configure(const daq::rc::TransitionCmd& cmd) override;
    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;
    void stopRecording(const daq::rc::TransitionCmd& cmd) override;
    // void unconfigure(const daq::rc::TransitionCmd& cmd) override;
    void user(const daq::rc::UserCmd& cmd) override;
    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd&) override;

 private:
    void configureROCs();
    void configureVMMs();

    std::unique_ptr<nsw::ConfigReader> m_reader;
    std::unique_ptr<nsw::ConfigSender> m_sender;

    std::vector<std::string> m_roc_names;
    std::vector<std::string> m_vmm_names;

    // Run the program in simulation mode, don't send any configuration
    bool m_simulation;
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
