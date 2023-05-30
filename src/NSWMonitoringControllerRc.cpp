#include "NSWConfiguration/NSWMonitoringControllerRc.h"

#include <chrono>
#include <cstdint>
#include <iterator>
#include <stop_token>
#include <utility>
#include <string>
#include <memory>
#include <ranges>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <is/infodynany.h>
#include <is/infoT.h>
#include <ers/ers.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/RcUtility.h"
#include "NSWConfiguration/monitoring/Config.h"
#include "NSWConfiguration/monitoring/IsPublisher.h"
#include "NSWConfiguration/monitoring/Utility.h"
#include "NSWConfigurationDal/NSWMonitoringControllerApplication.h"
#include "NSWConfigurationDal/NSW_MonitoringGroup.h"
#include "NSWConfigurationIs/Statistics.h"

using namespace std::chrono_literals;

nsw::NSWMonitoringControllerRc::NSWMonitoringControllerRc(std::string partitionName) :
  m_partitionName{std::move(partitionName)}
{}

void nsw::NSWMonitoringControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWMonitoringControllerRc::user(const daq::rc::UserCmd& usrCmd)
{
  ERS_LOG("User command received: " << usrCmd.commandName());
  if (usrCmd.commandName() == nsw::commands::START) {
    ERS_INFO("Starting Monitoring");
    startMonitoringAll();
  } else if (usrCmd.commandName() == nsw::commands::STOP) {
    ERS_INFO("Stopping Monitoring");
    stopMonitoringAll();
  } else {
    ers::warning(nsw::NSWUnkownCommand(ERS_HERE, usrCmd.commandName()));
  }
}

void nsw::NSWMonitoringControllerRc::startMonitoringAll()
{
  stopMonitoringAll();
  m_threads = mon::startMonitoring(
    m_configs,
    [this](const std::string& name) { m_scaServiceSender.send(nsw::commands::MONITOR, {name}, 0); },
    m_isDictionary.get(),
    m_monitoringIsServerName,
    m_app->UID());
}

void nsw::NSWMonitoringControllerRc::stopMonitoringAll()
{
  for (auto& thread : m_threads) {
    thread.second.request_stop();
  }
  for (auto& thread : m_threads) {
    thread.second.join();
  }
  m_threads.clear();
}


void nsw::NSWMonitoringControllerRc::subTransition(const daq::rc::SubTransitionCmd& cmd)
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
    m_ipcpartition = rcSvc.getIPCPartition();
    m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);
    const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
    m_app = rcBase.cast<dal::NSWMonitoringControllerApplication>();

    m_monitoringIsServerName = m_app->get_monitoringIsServerName();

    m_scaServiceSender =
      CommandSender(findSegmentSiblingApp("NSWSCAServiceApplication"),
                    std::make_unique<daq::rc::CommandSender>(m_ipcpartition, m_app->UID()));

    m_configs = mon::parseMonitoringGroups(m_partitionName, m_app->get_monitoringGroupSetName());
    m_scaServiceSender.send(nsw::commands::MON_IS_SERVER_NAME, {m_monitoringIsServerName});
    
    ERS_INFO("End Configure SubTransition");
  }
}
