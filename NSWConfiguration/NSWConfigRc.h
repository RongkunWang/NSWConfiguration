
#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <memory>

#include "RunControl/RunControl.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"

namespace nsw {
class NSWConfigRc: public daq::rc::Controllable {
 public:
    // override only the needed methods
    virtual ~NSWConfigRc() noexcept {}
    void configure(const daq::rc::TransitionCmd& cmd) override;
    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;
    void stopRecording(const daq::rc::TransitionCmd& cmd) override;
    // void unconfigure(const daq::rc::TransitionCmd& cmd) override;
    // void user(const daq::rc::UserCmd& cmd) override;
    // void onExit(daq::rc::FSM_STATE) noexcept override;
 private:
    std::unique_ptr<nsw::ConfigReader> reader;
    std::unique_ptr<nsw::ConfigSender> sender;
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
