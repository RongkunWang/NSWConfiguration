#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <memory>

#include <ipc/partition.h>
#include <is/infodictionary.h>

#include <RunControl/RunControl.h>

#include "NSWConfiguration/NSWConfig.h"

namespace daq::rc {
  class SubTransitionCmd;
  class TransitionCmd;
  class UserCmd;
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

    void connect(const daq::rc::TransitionCmd& cmd) override;

    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;

    void stopRecording(const daq::rc::TransitionCmd& cmd) override;

    void unconfigure(const daq::rc::TransitionCmd& cmd) override;

    void disconnect(const daq::rc::TransitionCmd& cmd) override;

    void user(const daq::rc::UserCmd& cmd) override;

    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd&) override;

    //! Handle Configuration
    bool simulationFromIS();

 private:
    std::unique_ptr<NSWConfig> m_NSWConfig;
    bool m_simulation;
    bool m_simulation_lock{false};

    std::string m_is_db_name; //!< Name of the IS server to retrieve NSW parameters from

    IPCPartition m_ipcpartition;
    std::unique_ptr<ISInfoDictionary> is_dictionary;

};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
