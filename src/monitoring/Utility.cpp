#include "NSWConfiguration/monitoring/Utility.h"

#include <dal/ResourceSetAND.h>
#include <dal/util.h>
#include <ers/ers.h>
#include <RunControl/Common/OnlineServices.h>

#include <NSWConfigurationDal/NSW_MonitoringGroup.h>

#include "NSWConfiguration/Issues.h"

std::vector<nsw::mon::Config> nsw::mon::parseMonitoringGroups(const std::string& partitionName,
                                                              const std::string& groupSetName)
{
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  auto& conf = rcSvc.getConfiguration();
  const auto* partition = daq::core::get_partition(conf, partitionName);
  if (partition == nullptr) {
    throw NSWInvalidPartition(ERS_HERE);
  }
  const auto* groupSet = conf.get<daq::core::ResourceSetAND>(groupSetName);
  if (groupSet == nullptr) {
    ers::error(NSWMonitoringGroupSetNotFound(ERS_HERE, groupSetName));
    return {};
  }
  const auto groupsBase = groupSet->get_Contains();
  std::vector<mon::Config> configs{};
  configs.reserve(std::size(groupsBase));
  const auto enabled = [&partition](const auto* group) { return not group->disabled(*partition); };
  const auto valid = [&conf](const auto* group) {
    return conf.cast<dal::NSW_MonitoringGroup>(group) != nullptr;
  };
  std::ranges::transform(groupsBase | std::views::filter(enabled) | std::views::filter(valid),
                         std::back_inserter(configs),
                         [&conf](const auto* groupBase) -> mon::Config {
                           const auto* group = conf.cast<dal::NSW_MonitoringGroup>(groupBase);
                           return {group->UID(), std::chrono::seconds{group->get_frequency()}};
                         });
  return configs;
}
