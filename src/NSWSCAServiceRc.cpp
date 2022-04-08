#include "NSWConfiguration/NSWSCAServiceRc.h"

#include <utility>
#include <string>
#include <memory>

#include <fmt/core.h>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>
#include <RunControl/FSM/FSMStates.h>

#include <is/infoT.h>
#include <is/infodynany.h>

#include <ers/ers.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfigurationDal/NSWSCAServiceApplication.h"
#include "NSWConfiguration/Issues.h"

bool nsw::NSWSCAServiceRc::simulationFromIS()
{
  // Grab the simulation bool from IS
  // Can manually write to this variable from the command line:
  const auto paramName = fmt::format("{}.simulation", m_isDbName);
  if (m_isDictionary->contains(paramName)) {
    ISInfoBool value;
    m_isDictionary->getValue(paramName, value);
    ERS_INFO("Simulation from IS: " << value);
    return value.getValue();
  }
  const auto is_cmd =
    fmt::format("is_write -p ${{TDAQ_PARTITION}} -n {} -t Boolean -v 1 -i 0", paramName);
  ERS_INFO(fmt::format("'simulation' not found in IS. Perhaps you forgot: {}", is_cmd));
  return false;
}

void nsw::NSWSCAServiceRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<dal::NSWSCAServiceApplication>();
  m_isDbName = app->get_dbISName();

  // Retrieve the ipc partition
  m_ipcpartition = rcSvc.getIPCPartition();

  // Get the IS dictionary for the current partition
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

  m_simulation = simulationFromIS();

  m_NSWConfig = std::make_unique<NSWConfig>(m_simulation);
  m_NSWConfig->readConf(app);
  m_NSWConfig->setCommandSender(
    {app->UID(), std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID())});
  m_sectorControllerSender =
    CommandSender(app->get_sectorControllerName(),
                  std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
  m_NSWConfig->readConfigurationResource();
  ERS_LOG("End");
}

void nsw::NSWSCAServiceRc::unconfigure(const daq::rc::TransitionCmd& /*cmd*/)
{
  m_NSWConfig->unconfigureRc();
}

void nsw::NSWSCAServiceRc::user(const daq::rc::UserCmd& usrCmd)
{
  const auto commandName = usrCmd.commandName();
  ERS_LOG("User command received: " << commandName);
  const auto notifyScaUnavailable = [this, &commandName]() {
    ERS_INFO("SCA unavailable");
    m_isDictionary->checkin(fmt::format("{}.{}", m_isDbName, "scaAvailable"), ISInfoBool(false));
    if (commandName == nsw::commands::RECOVER_OPC_MESSAGE) {
      m_sectorControllerSender.send(nsw::commands::SCA_DISCONNECTED);
    }
  };
  try {
    if (commandName == nsw::commands::CONFIGURE) {
      m_NSWConfig->configureRc();
    } else if (commandName == nsw::commands::START) {
      m_NSWConfig->startRc();
    } else if (commandName == nsw::commands::STOP) {
      m_NSWConfig->stopRc();
    } else if (commandName == nsw::commands::ENABLE_VMM) {
      m_NSWConfig->enableVmmCaptureInputs();
    } else if (commandName == nsw::commands::RECOVER_OPC or
               commandName == nsw::commands::RECOVER_OPC_MESSAGE) {
      notifyScaUnavailable();
    } else if (commandName == nsw::commands::RECONNECT_OPC) {
      if (m_NSWConfig->recoverOpc()) {
        m_isDictionary->checkin(fmt::format("{}.{}", m_isDbName, "scaAvailable"), ISInfoBool(true));
        m_sectorControllerSender.send(nsw::commands::SCA_RECONNECTED);
      }
    } else if (commandName == nsw::commands::MONITOR) {
      if (std::size(usrCmd.commandParameters()) != 1) {
        ers::warning(nsw::NSWInvalidCommand(
          ERS_HERE,
          fmt::format("Monitor command must have one argument. Recieved {} commands ({}).",
                      std::size(usrCmd.commandParameters()),
                      usrCmd.toString())));
      }
      if (m_monitoringIsServerName.empty()) {
        ers::warning(nsw::NSWConfigIssue(ERS_HERE, "Requested monitoring but did not set IS server before"));
      } else {
        m_NSWConfig->monitor(
          usrCmd.commandParameters().at(0), m_isDictionary.get(), m_monitoringIsServerName);
      }
    } else if (commandName == nsw::commands::MON_IS_SERVER_NAME) {
      if (std::size(usrCmd.commandParameters()) != 1) {
        ers::warning(nsw::NSWInvalidCommand(
          ERS_HERE, "Recieved command to set monitoring IS server name but did not get a name"));
      } else {
        m_monitoringIsServerName = usrCmd.commandParameters().at(0);
      }
    } else {
      ers::warning(nsw::NSWUnkownCommand(ERS_HERE, commandName));
    }
  } catch (const OpcReadWriteIssue&) {
    notifyScaUnavailable();
  } catch (const OpcConnectionIssue&) {
    notifyScaUnavailable();
  } catch (const NSWConfigurationOpcError&) {
    notifyScaUnavailable();
  }
}
