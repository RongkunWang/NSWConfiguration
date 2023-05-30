#ifndef NSWCONFIGURATION_NSWCONFIGURATIONCONTROLLERRC_H
#define NSWCONFIGURATION_NSWCONFIGURATIONCONTROLLERRC_H

#include "NSWConfiguration/CommandSender.h"
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
   * \brief Configuration Controller of one sector
   *
   * Recieves commands from sector controller and sends commands to SCA service. Controlls the SCA
   * based monitoring of devices.
   */
  class NSWConfigurationControllerRc : public daq::rc::Controllable
  {
  public:
    /**
     * \brief Read OKS database and create command senders
     */
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    /**
     * \brief Receive user commands
     *
     * \param cmd User command
     */
    void user(const daq::rc::UserCmd& cmd) override;
    
    /**
     * \brief Used to replace main Config sequence
     *
     * \param cmd : sub transition command
     */
    void subTransition(const daq::rc::SubTransitionCmd& /*cmd*/) override;

  private:
    nsw::CommandSender m_scaServiceSender;  //!< Command sender to SCA service application
    IPCPartition m_ipcpartition;
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
  };
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
