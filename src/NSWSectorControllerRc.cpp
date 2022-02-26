#include "NSWConfiguration/NSWSectorControllerRc.h"

#include <utility>
#include <string>
#include <memory>
#include <chrono>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>

#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfigurationDal/NSWSectorControllerApplication.h"
#include "fmt/core.h"

#include <ers/ers.h>

using namespace std::chrono_literals;

void nsw::NSWSectorControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<nsw::dal::NSWSectorControllerApplication>();

  m_isDbName = app->get_dbISName();
  m_ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);

  m_scaServiceSender =
    nsw::CommandSender(app->get_scaServiceName(),
                       std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
  m_configurationControllerSender =
    nsw::CommandSender(app->get_configurationControllerName(),
                       std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));
  m_monitoringControllerSender =
    nsw::CommandSender(app->get_monitoringControllerName(),
                       std::make_unique<daq::rc::CommandSender>(m_ipcpartition, app->UID()));

  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::connect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  for (int counter = 0; counter < NUM_CONFIG_RETRIES; ++counter) {
    m_configurationControllerSender.send("configure", 0);
    constexpr static auto TIMEOUT{1min};
    if (opcConnected()) {
      break;
    }
    if (not recoverOpc(TIMEOUT)) {
      const auto issue = NSWConfigIssue(ERS_HERE, "Cannot recover");
      ers::error(issue);
      throw issue;
    }
  }
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::prepareForRun(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_configurationControllerSender.send("start", 0);
  m_monitoringControllerSender.send("start", 0);
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::stopRecording(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_configurationControllerSender.send("stop", 0);
  m_monitoringControllerSender.send("stop", 0);
  ERS_LOG("End");
}

void nsw::NSWSectorControllerRc::disconnect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  m_configurationControllerSender.send("unconfigure", 0);
  ERS_INFO("End");
}

void nsw::NSWSectorControllerRc::unconfigure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_INFO("Start");
  ERS_INFO("End");
}

void nsw::NSWSectorControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  const auto commandName = usrCmd.commandName();
  ERS_LOG("User command received: " << commandName);
  if (commandName == "enableVmmCaptureInputs") {
    m_configurationControllerSender.send("enableVmmCaptureInputs", 0);
  } else if (commandName == "scaServiceUnavailable") {
    if (m_scaAvailable) {
      m_scaAvailable = false;
      const auto currentState = daq::rc::FSMStates::stringToState(usrCmd.currentFSMState());
      if (currentState == daq::rc::FSM_STATE::RUNNING) {
        m_monitoringControllerSender.send("stop");
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
  } else if (commandName == "scaServiceReconnected") {
    m_scaAvailable = true;
    if (daq::rc::FSMStates::stringToState(usrCmd.currentFSMState()) ==
        daq::rc::FSM_STATE::RUNNING) {
      ers::warning(NSWConfigIssue(ERS_HERE,
                                  "OPC server restarted while running. Data taking is most likely "
                                  "impossible until reconfiguring."));
    }
  }
}

void nsw::NSWSectorControllerRc::subTransition(const daq::rc::SubTransitionCmd& cmd)
{
  auto main_transition = cmd.mainTransitionCmd();
  auto sub_transition = cmd.subTransition();

  ERS_LOG(fmt::format(
    "Sub transition received: {} (mainTransition: {})", sub_transition, main_transition));
}

bool nsw::NSWSectorControllerRc::recoverOpc(const std::chrono::seconds timeout) const
{
  const auto start = std::chrono::high_resolution_clock::now();
  ERS_LOG("Start pinging OPC server");
  constexpr static auto INTERVAL{5s};
  while (not opcConnected() and std::chrono::high_resolution_clock::now() - start < timeout) {
    m_scaServiceSender.send("reconnect");
    std::this_thread::sleep_for(INTERVAL);
  }
  return opcConnected();
}

bool nsw::NSWSectorControllerRc::opcConnected() const
{
  const auto key = fmt::format("{}.{}", m_isDbName, "scaAvailable");
  if (m_isDictionary->contains(key)) {
    ISInfoBool value{false};
    m_isDictionary->getValue(key, value);
    return value.getValue();
  }
  return true;
}
