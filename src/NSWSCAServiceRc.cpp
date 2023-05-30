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
#include "NSWConfiguration/IsUtility.h"
#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfigurationDal/NSWSCAServiceApplication.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/RcUtility.h"

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
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(false));
    if (commandName == nsw::commands::RECOVER_OPC_MESSAGE) {
      m_sectorControllerSender.send(nsw::commands::SCA_DISCONNECTED);
    }
  };
  const auto checkErrorCounter = [this, &notifyScaUnavailable]() {
    const auto failedFraction = m_NSWConfig->getFractionFailed();
    if (failedFraction == 0.) {
      ERS_LOG("All devices configured correctly");
    } else if (failedFraction <= m_errorThresholdContinue) {
      ers::warning(NSWConfigurationError(ERS_HERE, failedFraction, "Continuing anyways"));
    } else if (failedFraction <= m_errorThresholdRecover) {
      ers::warning(NSWConfigurationError(ERS_HERE, failedFraction, "Trying to recover OPC"));
      notifyScaUnavailable();
    } else {
      ers::fatal(NSWConfigurationError(ERS_HERE, failedFraction, "Failing"));
    }
  };
  if (commandName == nsw::commands::CONFIGURE) {
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(true));
    m_NSWConfig->configureRc();
    checkErrorCounter();
  } else if (commandName == nsw::commands::CONNECT) {
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(true));
    m_NSWConfig->connectRc();
    checkErrorCounter();
  } else if (commandName == nsw::commands::START) {
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(true));
    m_NSWConfig->startRc();
  } else if (commandName == nsw::commands::STOP) {
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(true));
    m_NSWConfig->stopRc();
    checkErrorCounter();
  } else if (commandName == nsw::commands::ENABLE_VMM) {
    m_isDictionary->checkin(buildScaAvailableKey(m_isDbName, m_sectorId), ISInfoBool(true));
    m_NSWConfig->enableVmmCaptureInputs();
    checkErrorCounter();
  } else if (commandName == nsw::commands::RECOVER_OPC or
             commandName == nsw::commands::RECOVER_OPC_MESSAGE) {
    notifyScaUnavailable();
  } else if (commandName == nsw::commands::RECONNECT_OPC) {
    if (m_NSWConfig->recoverOpc()) {
      m_isDictionary->checkin(fmt::format("{}.{}.{}", m_isDbName, m_sectorId, "scaAvailable"),
                              ISInfoBool(true));
      m_sectorControllerSender.send(nsw::commands::SCA_RECONNECTED);
    }
  } else if (commandName == nsw::commands::MONITOR) {
    try {
      if (std::size(usrCmd.commandParameters()) != 1) {
        ers::warning(nsw::NSWInvalidCommand(
          ERS_HERE,
          fmt::format("Monitor command must have one argument. Recieved {} commands ({}).",
                      std::size(usrCmd.commandParameters()),
                      usrCmd.toString())));
      }
      if (m_monitoringIsServerName.empty()) {
        ers::warning(
          nsw::NSWConfigIssue(ERS_HERE, "Requested monitoring but did not set IS server before"));
      } else {
        m_NSWConfig->monitor(
          usrCmd.commandParameters().at(0), m_isDictionary.get(), m_monitoringIsServerName);
      }
    } catch (const OpcReadWriteIssue&) {
      notifyScaUnavailable();
    } catch (const OpcConnectionIssue&) {
      notifyScaUnavailable();
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
}

void nsw::NSWSCAServiceRc::subTransition(const daq::rc::SubTransitionCmd& cmd)
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
    const auto* app = rcBase.cast<dal::NSWSCAServiceApplication>();
    m_isDbName = app->get_dbISName();
    m_sectorId = extractSectorIdFromApp(app->UID());

    // Retrieve the ipc partition
    m_ipcpartition = rcSvc.getIPCPartition();

    // Get the IS dictionary for the current partition
    m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

    m_errorThresholdContinue = app->get_errorThresholdContinue();
    m_errorThresholdRecover = app->get_errorThresholdRecover();
    m_simulation = simulationFromIS();

    m_NSWConfig = std::make_unique<NSWConfig>(m_simulation);
    m_NSWConfig->readConf(app);
    m_NSWConfig->setCommandSender(
      {app->UID(), std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID())});
    m_sectorControllerSender =
      CommandSender(findSegmentSiblingApp("NSWSectorControllerApplication"),
                    std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
    m_NSWConfig->readConfigurationResource();
    
    ERS_INFO("End Configure SubTransition");
  }
}
