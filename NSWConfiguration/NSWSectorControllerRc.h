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
     * \brief Ask monitoring controller to stop
     */
    void stopROIB(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Ask configuration controller to stop
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
                  const std::uint64_t numAttempts) const
    {
      for (std::uint64_t counter = 0; counter < numAttempts; ++counter) {
        func();
        if (opcConnected()) {
          break;
        }
        if (counter == numAttempts - 1) {
          const auto issue = NSWOpcRetryLimitReached(ERS_HERE, numAttempts);
          if (m_ignoreOpcRetryError) {
            ers::error(issue);
            ers::warning(NSWOpcErrorIgnored(ERS_HERE));
          } else {
            ers::fatal(issue);
          }
        }
        if (not recoverOpc(timeout)) {
          const auto issue = NSWConfigIssue(ERS_HERE, "Cannot recover");
          if (m_ignoreOpcTimeoutError) {
            ers::error(issue);
            ers::warning(NSWOpcErrorIgnored(ERS_HERE));
          } else {
            ers::fatal(issue);
          }
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
    std::string m_sectorId{};
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
    std::atomic<bool> m_scaAvailable{true};
    std::future<void> m_reconnect;
    std::chrono::seconds m_opcReconnectTimeoutConfigure{};
    std::uint64_t m_opcReconnectAttemptLimitConfigure{};
    std::chrono::seconds m_opcReconnectTimeoutConnect{};
    std::uint64_t m_opcReconnectAttemptLimitConnect{};
    std::chrono::seconds m_opcReconnectTimeoutStart{};
    std::uint64_t m_opcReconnectAttemptLimitStart{};
    bool m_ignoreOpcTimeoutError{};
    bool m_ignoreOpcRetryError{};
  };
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
