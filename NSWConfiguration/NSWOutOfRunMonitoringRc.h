#ifndef NSWCONFIGURATION_NSWOUTOFRUNMONITORINGRC_H
#define NSWCONFIGURATION_NSWOUTOFRUNMONITORINGRC_H

#include <memory>
#include <string_view>
#include <thread>

#include <ipc/partition.h>
#include <is/infodictionary.h>
#include <RunControl/RunControl.h>

#include <NSWConfigurationDal/NSWOutOfRunMonitoringApplication.h>

#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfiguration/monitoring/Config.h"

namespace daq::rc {
  class SubTransitionCmd;
  class TransitionCmd;
  class UserCmd;
}  // namespace daq::rc

namespace nsw {
  /**
   * \brief Monitoring Controller of one sector
   *
   * Recieves commands from sector controller and sends commands to SCA service. Controls the
   * configuration of devices.
   */
  class NSWOutOfRunMonitoringRc : public daq::rc::Controllable
  {
  public:
    /**
     * @brief Constructor
     *
     * @param partitionName Name of the partition
     */
    explicit NSWOutOfRunMonitoringRc(std::string partitionName);

    /**
     * \brief Read OKS database and create command senders, send IS server name to SCA service
     */
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Start monitoring
     */
    void prepareForRun(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Stop monitoring
     */
    void stopRecording(const daq::rc::TransitionCmd& /*cmd*/) override;

  private:
    IPCPartition m_ipcpartition;
    std::unique_ptr<NSWConfig> m_NSWConfig{};
    const nsw::dal::NSWOutOfRunMonitoringApplication* m_app{};
    std::unique_ptr<ISInfoDictionary> m_isDictionary{nullptr};
    std::string m_partitionName;
    std::string m_monitoringIsServerName;
    std::vector<nsw::mon::Config> m_configs;
    std::map<std::string, std::jthread> m_threads;
  };
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
