#include "NSWConfiguration/NSWConfigurationControllerRc.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfigurationDal/NSWConfigurationControllerApplication.h"

#include <ers/ers.h>

void nsw::NSWConfigurationControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<dal::NSWConfigurationControllerApplication>();

  m_ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

  m_scaServiceSender =
    CommandSender(app->get_scaServiceName(),
                  std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));

  ERS_LOG("End");
}

void nsw::NSWConfigurationControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  ERS_LOG("User command received: " << usrCmd.commandName());
  if (usrCmd.commandName() == "configure") {
    m_scaServiceSender.send("configure", 0);
  } else if (usrCmd.commandName() == "unconfigure") {
    m_scaServiceSender.send("unconfigure", 0);
  } else if (usrCmd.commandName() == "start") {
    m_scaServiceSender.send("start", 0);
  } else if (usrCmd.commandName() == "stop") {
    m_scaServiceSender.send("stop", 0);
  } else if (usrCmd.commandName() == "enableVmmCaptureInputs") {
    m_scaServiceSender.send("enableVmmCaptureInputs", 0);
  } else {
    ers::warning(nsw::NSWUnkownCommand(ERS_HERE, usrCmd.commandName()));
  }
}
