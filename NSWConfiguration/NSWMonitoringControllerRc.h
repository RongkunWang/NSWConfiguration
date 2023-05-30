#ifndef NSWCONFIGURATION_NSWMONITORINGCONTROLLERRC_H
#define NSWCONFIGURATION_NSWMONITORINGCONTROLLERRC_H

#include <memory>
#include <string_view>
#include <thread>

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/monitoring/Config.h"
#include "NSWConfigurationDal/NSWMonitoringControllerApplication.h"

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
   * \brief Monitoring Controller of one sector
   *
   * Recieves commands from sector controller and sends commands to SCA service. Controls the
   * configuration of devices.
   */
  class NSWMonitoringControllerRc : public daq::rc::Controllable
  {
  public:
    /**
     * @brief Constructor
     *
     * @param partitionName Name of the partition
     */
    explicit NSWMonitoringControllerRc(std::string partitionName);

    /**
     * \brief Read OKS database and create command senders, send IS server name to SCA service
     */
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Receive user commands
     *
     * \param cmd User command
     */
    void user(const daq::rc::UserCmd& cmd) override;    /**
    
     * \brief Used to replace main Config sequence
     *
     * \param cmd : sub transition command
     */
    void subTransition(const daq::rc::SubTransitionCmd& /*cmd*/) override;

  private:
    nsw::CommandSender m_scaServiceSender;  //!< Command sender to SCA service application
    IPCPartition m_ipcpartition;
    const nsw::dal::NSWMonitoringControllerApplication* m_app{};
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
    std::string m_partitionName;
    std::string m_monitoringIsServerName;
    std::vector<nsw::mon::Config> m_configs;
    std::map<std::string, std::jthread> m_threads;

    /**
     * \brief Start moniotring threads for all groups
     */
    void startMonitoringAll();

    /**
     * \brief Stop moniotring threads for all groups
     */
    void stopMonitoringAll();
  };
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
