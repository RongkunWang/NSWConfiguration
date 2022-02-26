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

void nsw::NSWSCAServiceRc::configure(const daq::rc::TransitionCmd&)
{
  ERS_INFO("Start");
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

void nsw::NSWSCAServiceRc::user(const daq::rc::UserCmd& usrCmd)
{
  const auto notifyScaUnavailable = [this, &usrCmd]() {
    m_isDictionary->checkin(fmt::format("{}.{}", m_isDbName, "scaAvailable"), ISInfoBool(false));
    if (daq::rc::FSMStates::stringToState(usrCmd.currentFSMState()) != daq::rc::FSM_STATE::CONNECTED) {
      m_sectorControllerSender.send("scaServiceUnavailable");
    }
  };
  ERS_LOG("User command received: " << usrCmd.commandName());
  try {
    if (usrCmd.commandName() == "configure") {
      m_NSWConfig->configureRc();
    } else if (usrCmd.commandName() == "unconfigure") {
      m_NSWConfig->unconfigureRc();
    } else if (usrCmd.commandName() == "start") {
      m_NSWConfig->startRc();
    } else if (usrCmd.commandName() == "stop") {
      m_NSWConfig->stopRc();
    } else if (usrCmd.commandName() == "enableVmmCaptureInputs") {
      m_NSWConfig->enableVmmCaptureInputs();
    } else if (usrCmd.commandName() == "recover") {
      notifyScaUnavailable();
    } else if (usrCmd.commandName() == "reconnect") {
      if (m_NSWConfig->recoverOpc()) {
        m_isDictionary->checkin(fmt::format("{}.{}", m_isDbName, "scaAvailable"), ISInfoBool(true));
        m_sectorControllerSender.send("scaServiceReconnected");
      }
    } else {
      ers::warning(nsw::NSWUnkownCommand(ERS_HERE, usrCmd.commandName()));
    }
  } catch (const OpcReadWriteIssue&) {
    notifyScaUnavailable();
  } catch (const OpcConnectionIssue&) {
    notifyScaUnavailable();
  }
}
