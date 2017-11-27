
#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include "RunControl/RunControl.h"

namespace nsw {
class NSWConfigRc: public daq::rc::Controllable {
 public:
    // override only the needed methods
    virtual ~NSWConfigRc() noexcept {}
    void configure(const daq::rc::TransitionCmd& cmd) override;
    // void prepareForRun(const daq::rc::TransitionCmd& cmd) override;
    void stopRecording(const daq::rc::TransitionCmd& cmd) override;
    // void unconfigure(const daq::rc::TransitionCmd& cmd) override;
    // void user(const daq::rc::UserCmd& cmd) override;
    // void onExit(daq::rc::FSM_STATE) noexcept override;
 private:
    // std::vector<nsw::ConfigSender>;  /// One Sender for each OpcServer
};
};  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
