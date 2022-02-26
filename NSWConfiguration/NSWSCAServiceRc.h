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
  class NSWSCAServiceRc : public daq::rc::Controllable
  {
  public:
    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends

    void configure(const daq::rc::TransitionCmd&) override;
    void user(const daq::rc::UserCmd& cmd) override;

    //! Handle Configuration
    bool simulationFromIS();

  private:
    std::unique_ptr<NSWConfig> m_NSWConfig{};
    CommandSender m_sectorControllerSender{};
    bool m_simulation{false};

    std::string m_isDbName;  //!< Name of the IS server to retrieve NSW parameters from

    IPCPartition m_ipcpartition;
    std::unique_ptr<ISInfoDictionary> m_isDictionary;
  };
}  // namespace nsw

#endif
