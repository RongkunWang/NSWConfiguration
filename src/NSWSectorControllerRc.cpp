#include "NSWConfiguration/NSWSectorControllerRc.h"

#include <utility>
#include <string>
#include <memory>
#include <chrono>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/IsUtility.h"
#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfiguration/RcUtility.h"
#include "NSWConfigurationDal/NSWSectorControllerApplication.h"

#include <fmt/core.h>

#include <ers/ers.h>

using namespace std::chrono_literals;

void nsw::NSWSectorControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::connect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<nsw::dal::NSWSectorControllerApplication>();

  m_opcReconnectTimeoutConnect = std::chrono::seconds{app->get_opcReconnectTimeoutConnect()};
  m_opcReconnectAttemptLimitConnect = app->get_opcReconnectAttemptsConnect();
  retryOpc([this]() { m_configurationControllerSender.send(nsw::commands::CONNECT, 0); },
           m_opcReconnectTimeoutConnect,
           m_opcReconnectAttemptLimitConnect);
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::prepareForRun(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_configurationControllerSender.send(nsw::commands::START, 0);
  if (not opcConnected()) {
    ers::warning(
      NSWConfigIssue(ERS_HERE,
                     "OPC server disconnected while starting. Data taking is most likely "
                     "impossible until reconfiguration."));
  }
  if (opcConnected()) {
    m_monitoringControllerSender.send(nsw::commands::START, 0);
  }
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::stopRecording(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_configurationControllerSender.send(nsw::commands::STOP, 0);
  if (not opcConnected()) {
    ers::info(NSWConfigIssue(ERS_HERE, "OPC server disconnected while stopping."));
  }
  m_monitoringControllerSender.send(nsw::commands::STOP, 0);
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::disconnect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::unconfigure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  const auto commandName = usrCmd.commandName();
  ERS_LOG(fmt::format("User command received: {}", commandName));
  if (commandName == nsw::commands::ENABLE_VMM) {
    retryOpc([this]() { m_configurationControllerSender.send(nsw::commands::ENABLE_VMM, 0); },
             m_opcReconnectTimeoutStart,
             m_opcReconnectAttemptLimitStart);
  } else if (commandName == nsw::commands::SCA_DISCONNECTED) {
    if (m_scaAvailable) {
      m_scaAvailable = false;
      const auto currentState = daq::rc::FSMStates::stringToState(usrCmd.currentFSMState());
      if (currentState == daq::rc::FSM_STATE::RUNNING) {
        m_monitoringControllerSender.send(nsw::commands::STOP);
        m_reconnect = std::async(std::launch::async, [this]() {
          constexpr static auto TIMEOUT{24h};
          if (not recoverOpc(TIMEOUT)) {
            const auto issue = NSWConfigIssue(ERS_HERE, "Cannot recover");
            ers::error(issue);
            throw issue;
          }
        });
      }
    }
  } else if (commandName == nsw::commands::SCA_RECONNECTED) {
    m_scaAvailable = true;
    if (daq::rc::FSMStates::stringToState(usrCmd.currentFSMState()) ==
        daq::rc::FSM_STATE::RUNNING) {
      ers::warning(NSWConfigIssue(ERS_HERE,
                                  "OPC server restarted while running. Data taking is most likely "
                                  "impossible until reconfiguration."));
    }
  }
}

void nsw::NSWSectorControllerRc::subTransition(const daq::rc::SubTransitionCmd& cmd)
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
    const auto* app = rcBase.cast<nsw::dal::NSWSectorControllerApplication>();

    m_isDbName = app->get_dbISName();
    m_sectorId = extractSectorIdFromApp(app->UID());
    m_ipcpartition = rcSvc.getIPCPartition();
    m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

    m_scaServiceSender =
      nsw::CommandSender(findSegmentSiblingApp("NSWSCAServiceApplication"),
                        std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
    m_configurationControllerSender =
      nsw::CommandSender(findSegmentSiblingApp("NSWConfigurationControllerApplication"),
                        std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
    m_monitoringControllerSender =
      nsw::CommandSender(findSegmentSiblingApp("NSWMonitoringControllerApplication"),
                        std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));

    m_opcReconnectTimeoutConfigure = std::chrono::seconds{app->get_opcReconnectTimeoutConfigure()};
    m_opcReconnectAttemptLimitConfigure = app->get_opcReconnectAttemptsConfigure();
    m_opcReconnectTimeoutStart = std::chrono::seconds{app->get_opcReconnectTimeoutStart()};
    m_opcReconnectAttemptLimitStart = app->get_opcReconnectAttemptsStart();
    m_ignoreOpcTimeoutError = app->get_ignoreOpcTimeoutError();
    m_ignoreOpcRetryError = app->get_ignoreOpcRetryLimitError();

    retryOpc([this]() { m_configurationControllerSender.send(nsw::commands::CONFIGURE, 0); },
            m_opcReconnectTimeoutConfigure,
            m_opcReconnectAttemptLimitConfigure);

    ERS_INFO("End Configure SubTransition");
  }
  else if (sub_transition == "FIXME_STGCTP_RESET")
  {
    ERS_INFO("Start STGC TP RESET SubTransition");

    // Retrieving the configuration db
    daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
    const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
    const auto* app = rcBase.cast<nsw::dal::NSWSectorControllerApplication>();

    m_opcReconnectTimeoutConnect = std::chrono::seconds{app->get_opcReconnectTimeoutConnect()};
    m_opcReconnectAttemptLimitConnect = app->get_opcReconnectAttemptsConnect();
    retryOpc([this]() { m_configurationControllerSender.send(nsw::commands::RESET_STGCTP, 0); },
            m_opcReconnectTimeoutConnect,
            m_opcReconnectAttemptLimitConnect);

    ERS_INFO("End STGC TP RESET SubTransition");
  }
}

bool nsw::NSWSectorControllerRc::recoverOpc(const std::chrono::seconds timeout) const
{
  const auto start = std::chrono::high_resolution_clock::now();
  ERS_LOG("Start pinging OPC server");
  constexpr static auto INTERVAL{5s};
  while (not opcConnected() and std::chrono::high_resolution_clock::now() - start < timeout) {
    m_scaServiceSender.send(nsw::commands::RECONNECT_OPC);
    std::this_thread::sleep_for(INTERVAL);
  }
  return opcConnected();
}

bool nsw::NSWSectorControllerRc::opcConnected() const
{
  const auto key = buildScaAvailableKey(m_isDbName, m_sectorId);
  if (m_isDictionary->contains(key)) {
    ISInfoBool value{false};
    m_isDictionary->getValue(key, value);
    return value.getValue();
  }
  return true;
}
