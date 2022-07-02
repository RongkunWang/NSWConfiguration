#include "NSWConfiguration/hw/DeviceManager.h"

#include <future>

nsw::hw::DeviceManager::DeviceManager(const bool multithreaded) : m_multithreaded(multithreaded) {}

void nsw::hw::DeviceManager::add(const std::span<const boost::property_tree::ptree> configs)
{
  for (const auto& config : configs) {
    add(config);
  }
}

void nsw::hw::DeviceManager::addFeb(const nsw::FEBConfig& config)
{
  m_febs.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::addAddc(const nsw::ADDCConfig& config)
{
  for (std::size_t counter = 0; counter < config.getARTs().size(); counter++) {
    m_arts.emplace_back(m_opcManager, config, counter);
  }
}

void nsw::hw::DeviceManager::addMMTp(const nsw::TPConfig& config)
{
  m_mmtps.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::addSTGCTp(const boost::property_tree::ptree& config)
{
  m_stgctps.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::addRouter(const nsw::RouterConfig& config)
{
  m_routers.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::addPadTrigger(const boost::property_tree::ptree& config)
{
  m_padTriggers.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::addTpCarrier(const nsw::TPCarrierConfig& config)
{
  m_tpCarriers.emplace_back(m_opcManager, config);
}

void nsw::hw::DeviceManager::configure(const std::span<const Options> options)
{
  const auto conf = [this](const auto& devices, const std::string& deviceName, const auto... params) {
    ERS_INFO(fmt::format("Configuring {} {}", devices.size(), deviceName));
    applyFunc(
      devices,
      [&params...](const auto& device) { device.writeConfiguration(params...); },
      [](const auto& ex) {
        nsw::NSWHWConfigIssue issue(
          ERS_HERE, fmt::format("Configuration of device failed due to non OPC related issue: {}", ex.what()));
        ers::error(issue);
      });
  };

  m_configurationErrorCounter = 0;
  conf(
    m_febs, "FEB",
    std::find(std::cbegin(options), std::cend(options), Options::RESET_VMM) != std::cend(options),
    std::find(std::cbegin(options), std::cend(options), Options::RESET_TDS) != std::cend(options),
    std::find(std::cbegin(options), std::cend(options), Options::DISABLE_VMM_CAPTURE_INPUTS) != std::cend(options));
  conf(m_arts, "ART");
  conf(m_mmtps, "MMTP");
  conf(m_routers, "Router");
  conf(m_padTriggers, "Pad Trigger");
  conf(m_stgctps, "STGCTP");
  conf(m_tpCarriers, "TP Carrier");
}

void nsw::hw::DeviceManager::connect() const
{
  // MMG TP config and STG TP config are racing because
  // they are in different config applications.
  // Racing is fine, if STG TP is reset afterward.
  for (const auto& dev: m_stgctps) {
    dev.doReset();
  }
}

void nsw::hw::DeviceManager::enableVmmCaptureInputs()
{
  m_configurationErrorCounter = 0;
  applyFunc(
    m_febs,
    [](const auto& device) { device.getRoc().enableVmmCaptureInputs(); },
    [](const auto& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, fmt::format("Enabling VMM capture inputs failed due to: {}", ex.what()));
      ers::error(issue);
    });
}

void nsw::hw::DeviceManager::disableVmmCaptureInputs()
{
  m_configurationErrorCounter = 0;
  applyFunc(
    m_febs,
    [](const auto& device) { device.getRoc().disableVmmCaptureInputs(); },
    [](const auto& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, fmt::format("Disabling VMM capture inputs failed due to: {}", ex.what()));
      ers::error(issue);
    });
}

void nsw::hw::DeviceManager::toggleIdleStateHigh()
{
  applyFunc(
    m_padTriggers,
    [](const auto& device) { device.toggleIdleState(); },
    [](const auto& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, fmt::format("Toggling Pad Trigger Idle State High failed due to: {}", ex.what()));
      ers::error(issue);
    });
  applyFunc(
    m_mmtps,
    [](const auto& device) { device.toggleIdleStateHigh(); },
    [](const auto& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, fmt::format("Toggling MMTP Idle State High failed due to: {}", ex.what()));
      ers::error(issue);
    });
  applyFunc(
    m_stgctps,
    [](const auto& device) { device.doReset(); },
    [](const auto& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, fmt::format("Resetting sTGC TP failed due to: {}", ex.what()));
      ers::error(issue);
    });
}

void nsw::hw::DeviceManager::clear()
{
  clearOpc();
  m_febs.clear();
  m_arts.clear();
  m_mmtps.clear();
  m_stgctps.clear();
  m_routers.clear();
  m_padTriggers.clear();
  m_tpCarriers.clear();
}

void nsw::hw::DeviceManager::clearOpc()
{
  m_opcManager.clear();
}
