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

#include <dal/util.h>
#include <is/infodynany.h>
#include <is/infoT.h>
#include <ers/ers.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/monitoring/Config.h"
#include "NSWConfiguration/monitoring/IsPublisher.h"
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
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  m_ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(m_ipcpartition);
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  m_app = rcBase.cast<dal::NSWMonitoringControllerApplication>();

  m_monitoringIsServerName = m_app->get_monitoringIsServerName();

  m_scaServiceSender =
    CommandSender(m_app->get_scaServiceName(),
                  std::make_unique<daq::rc::CommandSender>(m_ipcpartition, m_app->UID()));

  m_configs = parseMonitoringGroups();
  m_scaServiceSender.send(nsw::commands::MON_IS_SERVER_NAME, {m_monitoringIsServerName});

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

std::vector<nsw::mon::Config> nsw::NSWMonitoringControllerRc::parseMonitoringGroups() const
{
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  auto& conf = rcSvc.getConfiguration();
  const auto* partition = daq::core::get_partition(conf, m_partitionName);
  if (partition == nullptr) {
    throw nsw::NSWInvalidPartition(ERS_HERE);
  }
  std::vector<const dal::NSW_MonitoringGroup*> groups{};
  conf.get(groups);
  std::vector<mon::Config> configs{};
  configs.reserve(std::size(groups));
  const auto filter = [&partition](const auto* group) { return not group->disabled(*partition); };
  std::ranges::transform(groups | std::views::filter(filter),
                         std::back_inserter(configs),
                         [](const auto* group) -> mon::Config {
                           return {group->UID(), std::chrono::seconds{group->get_frequency()}};
                         });
  return configs;
}

void nsw::NSWMonitoringControllerRc::startMonitoringAll()
{
  stopMonitoringAll();
  for (const auto& config : m_configs) {
    m_threads[config.m_name] = std::jthread{[this, &config](const std::stop_token stopToken) {
      while (not stopToken.stop_requested()) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        m_scaServiceSender.send(nsw::commands::MONITOR, {config.m_name}, 0);
        const auto timeDiff = std::chrono::high_resolution_clock::now() - startTime;
        auto stats = nsw::mon::is::Statistics{};
        stats.time = static_cast<std::uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count());
        m_isDictionary->checkin(
          fmt::format(
            "{}.{}.{}.{}", m_monitoringIsServerName, KEY_STATS, config.m_name, m_app->UID()),
          stats);
        const auto sleepTime = std::max(
          0ms,
          std::chrono::duration_cast<std::chrono::milliseconds>(config.m_frequency - timeDiff));
        if (sleepTime == 0s) {
          ers::warning(NSWMonitoringFrequency(
            ERS_HERE,
            config.m_name,
            config.m_frequency.count(),
            std::chrono::duration_cast<std::chrono::seconds>(timeDiff).count()));
        }
        std::mutex mutex;
        std::unique_lock lock(mutex);
        std::condition_variable_any().wait_for(lock, stopToken, sleepTime, [] { return false; });
        if (stopToken.stop_requested()) {
          return;
        }
      }
    }};
  }
}

void nsw::NSWMonitoringControllerRc::stopMonitoringAll()
{
  for (auto& thread : m_threads) {
    thread.second.request_stop();
  }
}
