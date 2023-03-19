#include "NSWConfiguration/NSWOutOfRunMonitoringRc.h"

#include <cstdint>
#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>

#include <ers/ers.h>

#include "NSWConfiguration/monitoring/Utility.h"

using namespace std::chrono_literals;

nsw::NSWOutOfRunMonitoringRc::NSWOutOfRunMonitoringRc(std::string partitionName) :
  m_partitionName{std::move(partitionName)}
{}

void nsw::NSWOutOfRunMonitoringRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  m_ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  m_app = rcBase.cast<dal::NSWOutOfRunMonitoringApplication>();

  m_monitoringIsServerName = m_app->get_monitoringIsServerName();

  m_configs = mon::parseMonitoringGroups(m_partitionName, m_app->get_monitoringGroupSetName());

  m_NSWConfig = std::make_unique<NSWConfig>(false);
  m_NSWConfig->readConf(m_app);
  m_NSWConfig->readConfigurationResource();

  ERS_LOG("End");
}

void nsw::NSWOutOfRunMonitoringRc::prepareForRun(const daq::rc::TransitionCmd& /*cmd*/)
{
  m_threads = mon::startMonitoring(
    m_configs,
    [this](const std::string& name) { m_NSWConfig->monitor(name, m_isDictionary.get(), m_monitoringIsServerName); },
    m_isDictionary.get(),
    m_monitoringIsServerName,
    m_app->UID());
}

void nsw::NSWOutOfRunMonitoringRc::stopRecording(const daq::rc::TransitionCmd& /*cmd*/)
{
  for (auto& thread : m_threads) {
    thread.second.request_stop();
  }
  for (auto& thread : m_threads) {
    thread.second.join();
  }
  m_threads.clear();
}
