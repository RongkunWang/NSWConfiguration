#include "NSWConfiguration/NSWConfigurationControllerRc.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/RcUtility.h"
#include "NSWConfigurationDal/NSWConfigurationControllerApplication.h"

#include <ers/ers.h>

void nsw::NSWConfigurationControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWConfigurationControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  ERS_LOG("User command received: " << usrCmd.commandName());
  const std::vector<std::string> args{usrCmd.currentFSMState()};
  if (usrCmd.commandName() == nsw::commands::CONFIGURE) {
    m_scaServiceSender.send(nsw::commands::CONFIGURE, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::CONNECT) {
    m_scaServiceSender.send(nsw::commands::CONNECT, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::RESET_STGCTP) {
    m_scaServiceSender.send(nsw::commands::RESET_STGCTP, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::UNCONFIGURE) {
    m_scaServiceSender.send(nsw::commands::UNCONFIGURE, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::START) {
    m_scaServiceSender.send(nsw::commands::START, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::STOP) {
    m_scaServiceSender.send(nsw::commands::STOP, args, 0);
  } else if (usrCmd.commandName() == nsw::commands::ENABLE_VMM) {
    m_scaServiceSender.send(nsw::commands::ENABLE_VMM, args, 0);
  } else {
    ers::warning(nsw::NSWUnkownCommand(ERS_HERE, usrCmd.commandName()));
  }
}

void nsw::NSWConfigurationControllerRc::subTransition(const daq::rc::SubTransitionCmd& cmd)
{
  auto main_transition = cmd.mainTransitionCmd();
  auto sub_transition = cmd.subTransition();

  ERS_LOG(fmt::format(
    "Sub transition received: {} (mainTransition: {})", sub_transition, main_transition));

  if (sub_transition == "FIXME_CONFIG")
  {
    ERS_INFO("Start Configure SubTransition");

    // Retrieving the configuration db
    daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
    const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
    const auto* app = rcBase.cast<dal::NSWConfigurationControllerApplication>();

    m_ipcpartition = rcSvc.getIPCPartition();
    m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

    m_scaServiceSender =
      CommandSender(findSegmentSiblingApp("NSWSCAServiceApplication"),
                    std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));

    ERS_INFO("End Configure SubTransition");
  }
}
