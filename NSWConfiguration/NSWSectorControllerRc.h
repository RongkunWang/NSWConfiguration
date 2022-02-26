#ifndef NSWCONFIGURATION_NSWSECTORCONTROLLERRC_H
#define NSWCONFIGURATION_NSWSECTORCONTROLLERRC_H

#include "NSWConfiguration/CommandSender.h"
#include <future>
#include <memory>

#include <ipc/partition.h>
#include <is/infodictionary.h>

#include <RunControl/RunControl.h>


namespace daq::rc {
  class SubTransitionCmd;
  class TransitionCmd;
  class UserCmd;
}  // namespace daq::rc

namespace nsw {
  class NSWSectorControllerRc : public daq::rc::Controllable
  {
  public:
    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    void connect(const daq::rc::TransitionCmd& /*cmd*/) override;

    void prepareForRun(const daq::rc::TransitionCmd& /*cmd*/) override;

    void stopRecording(const daq::rc::TransitionCmd& /*cmd*/) override;

    void unconfigure(const daq::rc::TransitionCmd& /*cmd*/) override;

    void disconnect(const daq::rc::TransitionCmd& /*cmd*/) override;

    void user(const daq::rc::UserCmd& cmd) override;

    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd& /*cmd*/) override;

  private:
    [[nodiscard]] bool recoverOpc(std::chrono::seconds timeout) const;
    [[nodiscard]] bool opcConnected() const;

    nsw::CommandSender m_scaServiceSender;  //!< Command sender to SCA service application
    nsw::CommandSender
      m_configurationControllerSender;  //!< Command sender to configuration controller application
    nsw::CommandSender
      m_monitoringControllerSender;  //!< Command sender to monitoring controller application
    IPCPartition m_ipcpartition;
    std::string m_isDbName;  //!< Name of the IS server to retrieve NSW parameters from
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
    std::atomic<bool> m_scaAvailable{true};
    std::future<void> m_reconnect;
    constexpr static int NUM_CONFIG_RETRIES{3};
  };
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
