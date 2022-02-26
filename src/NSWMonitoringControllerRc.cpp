#include "NSWConfiguration/NSWMonitoringControllerRc.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfigurationDal/NSWMonitoringControllerApplication.h"

#include <ers/ers.h>

void nsw::NSWMonitoringControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<dal::NSWMonitoringControllerApplication>();

  m_ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

  m_scaServiceSender =
    CommandSender(app->get_scaServiceName(),
                  std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));

  ERS_LOG("End");
}

void nsw::NSWMonitoringControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  ERS_LOG("User command received: " << usrCmd.commandName());
  if (usrCmd.commandName() == "start") {
    ERS_INFO("Starting Monitoring");
  } else if (usrCmd.commandName() == "stop") {
    ERS_INFO("Stopping Monitoring");
  } else {
    ers::warning(nsw::NSWUnkownCommand(ERS_HERE, usrCmd.commandName()));
  }
}
