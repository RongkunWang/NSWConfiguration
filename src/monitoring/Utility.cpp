#include "NSWConfiguration/monitoring/Utility.h"

#include <dal/ResourceSetAND.h>
#include <dal/util.h>
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
  const auto groupsBase = groupSet->get_Contains();
  std::vector<mon::Config> configs{};
  configs.reserve(std::size(groupsBase));
  const auto filter = [&partition](const auto* group) { return not group->disabled(*partition); };
  std::ranges::transform(groupsBase | std::views::filter(filter),
                         std::back_inserter(configs),
                         [&conf](const auto* groupBase) -> mon::Config {
                           const auto* group = conf.cast<dal::NSW_MonitoringGroup>(groupBase);
                           return {group->UID(), std::chrono::seconds{group->get_frequency()}};
                         });
  return configs;
}
