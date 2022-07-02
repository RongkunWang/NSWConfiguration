#ifndef NSWCONFIGURATION_NSWSCASERVICERC_H
#define NSWCONFIGURATION_NSWSCASERVICERC_H

#include <memory>

#include <ipc/partition.h>
#include <is/infodictionary.h>

#include <RunControl/RunControl.h>

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/NSWConfig.h"

namespace daq::rc {
  class SubTransitionCmd;
  class TransitionCmd;
  class UserCmd;
}  // namespace daq::rc

namespace nsw {
  /**
   * \brief SCA service for one sector
   *
   * Recieves commands from configuration, monitoring, and sector controller. Reports OPC issues to
   * sector controller. Holds all HWIs.
   */
  class NSWSCAServiceRc : public daq::rc::Controllable
  {
  public:
    /**
     * \brief Read OKS database and create command senders, create HWIs
     */
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Delete HWIs
     */
    void unconfigure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Receive user commands
     *
     * \param cmd User command
     */
    void user(const daq::rc::UserCmd& cmd) override;

    /**
     * \brief Get simulation switch from IS
     *
     * \return true simulation mode, do not talk to HW
     * \return false real mode, do talk to HW
     */
    bool simulationFromIS();

  private:
    std::unique_ptr<NSWConfig> m_NSWConfig{};
    CommandSender m_sectorControllerSender{};
    std::string m_monitoringIsServerName;
    double m_errorThresholdContinue{};
    double m_errorThresholdRecover{};
    bool m_simulation{false};

    std::string m_isDbName;  //!< Name of the IS server to retrieve NSW parameters from

    IPCPartition m_ipcpartition;
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
  };
}  // namespace nsw

#endif
