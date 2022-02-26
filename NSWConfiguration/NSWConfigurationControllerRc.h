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
}

namespace nsw {
class NSWConfigurationControllerRc: public daq::rc::Controllable {
 public:
    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    void configure(const daq::rc::TransitionCmd& /*cmd*/) override;

    void user(const daq::rc::UserCmd& cmd) override;

 private:
    nsw::CommandSender m_scaServiceSender;  //!< Command sender to SCA service application
    IPCPartition m_ipcpartition;
    std::unique_ptr<ISInfoDictionary> m_isDictionary;

};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
