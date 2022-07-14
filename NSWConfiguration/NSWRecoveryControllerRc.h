#ifndef NSWCONFIGURATION_NSWRECOVERYCONTROLLERRC_H
#define NSWCONFIGURATION_NSWRECOVERYCONTROLLERRC_H

#include <memory>
#include <thread>
#include <unordered_set>

#include "NSWConfiguration/recovery/ElinkAnalyzer.h"

#include <ipc/partition.h>
#include <is/infodictionary.h>

#include <dal/ResourceBase.h>

#include <RunControl/RunControl.h>

namespace daq::rc {
  class SubTransitionCmd;
  class TransitionCmd;
  class UserCmd;
}  // namespace daq::rc

namespace nsw {
  /**
   * \brief Controller stopless removal and recovery
   *
   * TODO
   */
  class NSWRecoveryControllerRc : public daq::rc::Controllable
  {
  public:
    NSWRecoveryControllerRc(std::string partitionName, std::string segmentName);
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

  private:
    void loop(std::stop_token stopToken) const;
    void analyze(const std::string& name) const;
    void removeLinks(const std::unordered_set<std::uint64_t>& fids) const;

    std::string m_partitionName{};
    std::string m_segmentName{};
    mutable std::map<std::string, ElinkAnalyzer> m_analyzers{};
    std::map<std::uint64_t, std::string> m_fidToElinkIdMap{};
    std::map<std::string, std::string> m_elinkToAppMap{};
    std::map<std::string, std::string> m_robToAppMap{};
    std::map<std::string, const daq::core::ResourceBase*> m_elinkToObjectMap{};
    std::vector<std::string> m_swRodRobNames{};
    std::jthread m_worker;
    std::string m_isNameSwrod;  //!< Name of the IS server to retrieve NSW parameters from
    std::chrono::seconds m_interval{};
    double m_percentageThreshold{};
    std::uint64_t m_minimumThreshold{};
    const daq::core::Partition* m_partition = nullptr;
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
    std::unique_ptr<Configuration> m_oksDb;
  };
}  // namespace nsw

#endif
