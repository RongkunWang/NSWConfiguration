#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_UTILITY_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_UTILITY_H

#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include <fmt/format.h>

#include <ers/ers.h>
#include <is/infodictionary.h>

#include <NSWConfigurationIs/Statistics.h>

#include "NSWConfiguration/monitoring/Config.h"

using namespace std::chrono_literals;

ERS_DECLARE_ISSUE(
  nsw,
  NSWMonitoringFrequency,
  fmt::format("Unable to execute monitoring of {} within time limit of {}s. Took {}s.",
              group,
              timeLimit,
              timeActual),
  ((std::string)group)((long)timeLimit)((long)timeActual))

ERS_DECLARE_ISSUE(
  nsw,
  NSWMonitoringGroupSetNotFound,
  fmt::format("Did not find monitoring group set with name {}. Will not monitor anything.",
              group),
  ((std::string)group))

namespace nsw::mon {
  /**
   * \brief Parse monitoring groups from OKS and create config objects
   *
   * \param partitionName Name of the partition
   * \param groupSetName Name of the OKS resource set holding the monitoring groups
   * \return std::vector<nsw::mon::Config> Vector of config objects (name and frequency)
   */
  [[nodiscard]] std::vector<nsw::mon::Config> parseMonitoringGroups(
    const std::string& partitionName,
    const std::string& groupSetName);

  /**
   * @brief Start monitoring and return threads
   *
   * @param configs Groups to monitor
   * @param monFunc Function that does the monitoring
   * @param isDict IS dictionary
   * @param isServerName Name of IS server to publish stats
   * @param appName Name of the app
   * @return std::map<std::string, std::jthread> Map of group name to thread
   */
  [[nodiscard]] std::map<std::string, std::jthread> startMonitoring(
    std::span<const nsw::mon::Config> configs,
    const std::regular_invocable<const std::string&> auto& monFunc,
    const ISInfoDictionary* isDict,
    const std::string& isServerName,
    const std::string& appName)
  {
    constexpr static std::string_view KEY_STATS{"Statistics"};
    std::map<std::string, std::jthread> threads{};
    for (const auto& config : configs) {
      threads[config.m_name] = std::jthread{[&config, isDict, &isServerName, &appName, &monFunc](
                                              const std::stop_token stopToken) {
        while (not stopToken.stop_requested()) {
          const auto startTime = std::chrono::high_resolution_clock::now();
          monFunc(config.m_name);
          const auto timeDiff = std::chrono::high_resolution_clock::now() - startTime;
          auto stats = nsw::mon::is::Statistics{};
          stats.time = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count());
          isDict->checkin(
            fmt::format("{}.{}.{}.{}", isServerName, KEY_STATS, config.m_name, appName), stats);
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
    return threads;
  }
}  // namespace nsw::mon

#endif
