#ifndef NSWCONFIGURATION_NSWSECTORCONTROLLERRC_H
#define NSWCONFIGURATION_NSWSECTORCONTROLLERRC_H

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfiguration/Issues.h"
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
  /**
   * \brief Controller for one sector
   *
   * Recieves commands from orchestrator and sends commands to SCA service, monitoring and
   * configuration controller. Controls configuration and monitoring of one sector.
   */
  class NSWSectorControllerRc : public daq::rc::Controllable
  {
  public:
    /**
     * \brief Read OKS database and create command senders
     */
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Send configure command to configuration controller
     *
     * Retries configuration if OPC server is unavailable
     */
    void connect(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Ask configuration and monitoring controller to start
     */
    void prepareForRun(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Ask configuration and monitoring controller to stop
     */
    void stopRecording(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Noop
     */
    void unconfigure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Noop
     */
    void disconnect(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Receive user commands
     *
     * \param cmd User command
     */
    void user(const daq::rc::UserCmd& cmd) override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd& /*cmd*/) override;

  private:
    /**
     * \brief Perodically send reconnect OPC commands to SCA service
     *
     * \param timeout Fail after timeout
     * \return true reconnected
     * \return false remains disconnected
     */
    [[nodiscard]] bool recoverOpc(std::chrono::seconds timeout) const;

    /**
     * \brief Retry an operation if OPC dies during the operation
     *
     * \param func Operation to be performed
     * \param timeout Timeout for OPC reconnections
     * \param retries Number of attempts
     */
    void retryOpc(const std::regular_invocable<> auto& func,
                  const std::chrono::seconds timeout,
                  const int retries) const
    {
      for (int counter = 0; counter < retries; ++counter) {
        func();
        if (opcConnected()) {
          break;
        }
        if (counter == retries - 1) {
          const auto issue = NSWOpcRetryLimitReached(ERS_HERE, retries);
          ers::error(issue);
          throw issue;
        }
        if (not recoverOpc(timeout)) {
          const auto issue = NSWConfigIssue(ERS_HERE, "Cannot recover");
          ers::error(issue);
          throw issue;
        }
      }
    }

    /**
     * \brief Is the OPC server connected
     *
     * \return true yes
     * \return false no
     */
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
