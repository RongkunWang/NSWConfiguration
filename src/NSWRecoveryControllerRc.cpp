#include "NSWConfiguration/NSWRecoveryControllerRc.h"

#include <stop_token>
#include <utility>
#include <string>
#include <memory>
#include <chrono>
#include <ranges>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <ers/ers.h>
#include <is/info.h>
#include <is/infoT.h>
#include <is/infoany.h>
#include <is/infodynany.h>
#include <is/infodictionary.h>
#include <is/infodocument.h>
#include <is/type.h>

// Header to the RC online services
#include <RunControl/Common/OnlineServices.h>
#include <RunControl/Common/RunControlCommands.h>
#include <RunControl/Common/UserExceptions.h>
#include <config/Configuration.h>

#include <swrod/LinkStatistics.h>
#include <swrod/ROBStatistics.h>

#include <dal/Segment.h>
#include <dal/util.h>
#include <dal/Partition.h>
#include <dal/ResourceSet.h>

#include "NSWConfiguration/recovery/ElinkAnalyzer.h"

#include "NSWConfigurationDal/NSWRecoveryControllerApplication.h"
#include "NSWConfigurationDal/SwRodInputLink.h"
#include "NSWConfigurationDal/SwRodRob.h"

using namespace std::chrono_literals;

nsw::NSWRecoveryControllerRc::NSWRecoveryControllerRc(std::string partitionName,
                                                      std::string segmentName) :
  m_partitionName{std::move(partitionName)}, m_segmentName{std::move(segmentName)}
{}

void nsw::NSWRecoveryControllerRc::configure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  // Retrieving the configuration db
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
  const auto* app = rcBase.cast<nsw::dal::NSWRecoveryControllerApplication>();

  m_isNameSwrod = app->get_swrodIsServerName();
  m_interval = std::chrono::seconds{app->get_interval()};
  m_percentageThreshold = app->get_percentageThreshold();
  m_minimumThreshold = app->get_minimumThreshold();
  const auto ipcpartition = rcSvc.getIPCPartition();
  m_isDictionary = std::make_unique<ISInfoDictionary>(ipcpartition);
  m_oksDb = std::make_unique<Configuration>("");
  m_partition = daq::core::get_partition(*m_oksDb, m_partitionName);

  const auto* const segment = m_partition->get_segment(m_segmentName);
  std::set<std::string> types{"SwRodApplication"};
  const auto swRodApps = segment->get_all_applications(&types);
  for (const auto* swRodApp : swRodApps) {
    ERS_LOG(fmt::format("Found swROD application {}", swRodApp->UID()));
    for (const auto& module : swRodApp->cast<daq::core::ResourceSet>()->get_Contains()) {
      const auto robs = module->cast<daq::core::ResourceSet>()->get_Contains();
      for (const auto& rob : robs) {
        if (rob->disabled(*m_partition)) {
          continue;
        }
        ERS_LOG(fmt::format("Found ROB {}", rob->UID()));
        const auto robIdName = fmt::format("{:08x}", rob->cast<nsw::dal::SwRodRob>()->get_Id());
        m_swRodRobNames.push_back(robIdName);
        m_analyzers.try_emplace(robIdName,
                                ElinkAnalyzer{m_percentageThreshold, m_minimumThreshold});
        m_robToAppMap.try_emplace(robIdName, swRodApp->UID());
        for (const auto& dataChannel : rob->cast<daq::core::ResourceSet>()->get_Contains()) {
          const auto links = dataChannel->cast<daq::core::ResourceSet>()->get_Contains();
          for (const auto& link : links) {
            if (link->disabled(*m_partition)) {
              continue;
            }
            ERS_LOG(fmt::format("Found elink {}", link->UID()));
            m_elinkToAppMap.try_emplace(link->UID(), swRodApp->UID());
            m_fidToElinkIdMap.try_emplace(link->cast<nsw::dal::SwRodInputLink>()->get_FelixId(),
                                          link->UID());
            m_elinkToObjectMap.try_emplace(link->UID(), link);
          }
        }
      }
    }
  }

  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::connect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::prepareForRun(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_worker = std::jthread([this](std::stop_token stopToken) { loop(std::move(stopToken)); });
  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::stopRecording(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_worker.request_stop();
  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::disconnect(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::unconfigure(const daq::rc::TransitionCmd& /*cmd*/)
{
  ERS_LOG("Start");
  m_elinkToAppMap.clear();
  m_fidToElinkIdMap.clear();
  ERS_LOG("End");
}

void nsw::NSWRecoveryControllerRc::user(const daq::rc::UserCmd& usrCmd) {}

void nsw::NSWRecoveryControllerRc::loop(std::stop_token stopToken) const
{
  while (not stopToken.stop_requested()) {
    const auto startTime = std::chrono::high_resolution_clock::now();
    for (const auto& name : m_swRodRobNames) {
      analyze(name);
    }
    const auto timeDiff = std::chrono::high_resolution_clock::now() - startTime;
    const auto sleepTime =
      std::max(0ms, std::chrono::duration_cast<std::chrono::milliseconds>(m_interval - timeDiff));
    std::mutex mutex;
    std::unique_lock lock(mutex);
    std::condition_variable_any().wait_for(lock, stopToken, sleepTime, [] { return false; });
    if (stopToken.stop_requested()) {
      return;
    }
  }
}

void nsw::NSWRecoveryControllerRc::analyze(const std::string& name) const
{
  // ISType type{};
  // m_isDictionary->getType(fmt::format("DF.swrod.{}.ROB-{}", m_robToAppMap.at(name), name), type);
  // ERS_INFO("Type is " << type);
  swrod::ROBStatistics val{};
  const auto key = fmt::format("DF.swrod.{}.ROB-{}", m_robToAppMap.at(name), name);
  if (m_isDictionary->contains(key)) {
    m_isDictionary->getValue(key, val);
    // const auto bla = val.getAttributeValue<>("linksStatistics");
    // ERS_INFO(bla.getAttributesNumber());
    // removeLinks(m_analyzers.at(name).analyze());
    removeLinks(m_analyzers.at(name).analyze(val.linksStatistics, val.disabledLinks));
    // ERS_INFO(val.getAttributeDescription(i).name() << " " << val.getAttributeType(i));
  }
}

void nsw::NSWRecoveryControllerRc::removeLinks(const std::unordered_set<std::uint64_t>& fids) const
{
  if (fids.empty()) {
    return;
  }
  ERS_INFO(fmt::format("Got FIDs {}", fids));
  ERS_INFO(fmt::format("Have map {}", m_fidToElinkIdMap));
  ERS_INFO(fmt::format("Have map2 {}", std::views::keys(m_elinkToObjectMap)));
  const auto getId = [this](const auto fid) {
    ERS_INFO(fmt::format("TRANSFORM {}", fid));
    auto bla = m_fidToElinkIdMap.at(fid);
    ERS_INFO(fmt::format("TRANSFORM result {}", bla));
    return bla;
  };
  const auto filterDisabled = [this](const auto& id) {
    ERS_INFO(fmt::format("FILTER {}", id));
    const auto blub = m_elinkToObjectMap.at(id);
    ERS_INFO(fmt::format(
      "FILTER blub {} {}", fmt::ptr(blub), fmt::ptr(m_partition)));
    ERS_INFO(m_partition->UID());
    auto bla = not blub->disabled(*m_partition);
    ERS_INFO(fmt::format("FILTER result {}", bla));
    return bla;
  };
  const auto links = fmt::format(
    "{}", fmt::join(fids | std::views::transform(getId) | std::views::filter(filterDisabled), ","));
  if (links.empty()) {
    return;
  }
  const auto appName = m_elinkToAppMap.at(m_fidToElinkIdMap.at(*std::begin(fids)));
  ERS_INFO(fmt::format("Removing from {} the following links {}", appName, links));
  daq::rc::HardwareError issue(ERS_HERE, links.c_str(), appName.c_str());
  issue.add_qualifier("SWROD");
  ers::error(issue);
}
