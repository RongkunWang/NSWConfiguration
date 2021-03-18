#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <memory>

#include "RunControl/RunControl.h"
#include "NSWConfiguration/NSWConfig.h"

class daq::rc::SubTransitionCmd;
class daq::rc::TransitionCmd;
class daq::rc::UserCmd;

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

    void connect(const daq::rc::TransitionCmd& cmd) override;

    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;

    void stopRecording(const daq::rc::TransitionCmd& cmd) override;

    void unconfigure(const daq::rc::TransitionCmd& cmd) override;

    void disconnect(const daq::rc::TransitionCmd& cmd) override;

    void user(const daq::rc::UserCmd& cmd) override;

    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd&) override;

 private:
    std::unique_ptr<NSWConfig> m_NSWConfig;
    bool m_simulation;

};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
