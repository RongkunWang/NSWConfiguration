#include "NSWConfiguration/hw/DeviceManager.h"

#include <future>

nsw::hw::DeviceManager::DeviceManager(const bool multithreaded) : m_multithreaded(multithreaded) {}

void nsw::hw::DeviceManager::addFeb(const nsw::FEBConfig& config)
{
  m_febs.emplace_back(config);
}

void nsw::hw::DeviceManager::addAddc(const nsw::ADDCConfig& config)
{
  for (std::size_t counter = 0; counter < config.getARTs().size(); counter++) {
    m_arts.emplace_back(config, counter);
  }
}

void nsw::hw::DeviceManager::addTp(const nsw::TPConfig& config)
{
  m_tps.emplace_back(config);
}

void nsw::hw::DeviceManager::addRouter(const nsw::RouterConfig& config)
{
  m_routers.emplace_back(config);
}

void nsw::hw::DeviceManager::addPadTrigger(const nsw::hw::PadTrigger& config)
{
  m_padTriggers.emplace_back(config);
}

void nsw::hw::DeviceManager::addTpCarrier(const nsw::TPCarrierConfig& config)
{
  m_tpCarriers.emplace_back(config);
}

void nsw::hw::DeviceManager::configure(const std::vector<Options>& options) const
{
  const auto conf = [this](const auto& devices, const std::string& deviceName, const auto... params) {
    // C++20
    // applyFunc(
    //   m_febs,
    //   [params](const auto& device) { device.writeConfiguration(params...); },
    //   [](const auto& ex) {
    //     nsw::NSWHWConfigIssue issue(
    //       ERS_HERE, fmt::format("Configuration of device failed due to: {}", ex.what()));
    //     ers::fatal(issue);
    //   });
    ERS_INFO(fmt::format("Configuring {} {}", devices.size(), deviceName));
    try {
      if (m_multithreaded) {
        std::vector<std::future<void>> threads{};
        threads.reserve(devices.size());
        for (const auto& device : devices) {
          threads.push_back(std::async(
            std::launch::async,
            [](const auto& deviceLocal, const auto... paramsLocal) {
              deviceLocal.writeConfiguration(paramsLocal...);
            },
            device,
            params...));
        }
        for (auto& thread : threads) {
          thread.get();
        }
      } else {
        for (const auto& device : devices) {
          device.writeConfiguration(params...);
        }
      }
    } catch (std::exception& ex) {
      nsw::NSWHWConfigIssue issue(
        ERS_HERE, "Configuration of device failed due to : " + std::string(ex.what()));
      // TODO: This should be an error
      ers::fatal(issue);
    }
  };

  conf(
    m_febs, "FEB",
    std::find(std::cbegin(options), std::cend(options), Options::RESET_VMM) != std::cend(options),
    std::find(std::cbegin(options), std::cend(options), Options::DISABLE_VMM_CAPTURE_INPUTS) !=
      std::cend(options),
    std::find(std::cbegin(options), std::cend(options), Options::RESET_TDS) != std::cend(options));
  conf(m_arts, "ART");
  conf(m_tps, "TP");
  conf(m_routers, "Router");
  conf(m_padTriggers, "Pad Trigger");
  conf(m_tpCarriers, "TP Carrier");
}

void nsw::hw::DeviceManager::enableVmmCaptureInputs() const
{
  // C++20
  // applyFunc(
  //   m_febs,
  //   [](const auto& device) { device.getRoc().enableVmmCaptureInputs(); },
  //   [](const auto& ex) {
  //     nsw::NSWHWConfigIssue issue(
  //       ERS_HERE, fmt::format("Enabling VMM capture inputs failed due to: {}", ex.what()));
  //     ers::fatal(issue);
  //   });
  try {
    if (m_multithreaded) {
      std::vector<std::future<void>> threads{};
      threads.reserve(m_febs.size());
      for (const auto& feb : m_febs) {
        threads.push_back(std::async(
          std::launch::async,
          [](const auto& febLocal) { febLocal.getRoc().enableVmmCaptureInputs(); },
          feb));
      }
      for (auto& thread : threads) {
        thread.get();
      }
    } else {
      for (const auto& feb : m_febs) {
        feb.getRoc().enableVmmCaptureInputs();
      }
    }
  } catch (std::exception& ex) {
    nsw::NSWHWConfigIssue issue(
      ERS_HERE, "Configuration of device failed due to : " + std::string(ex.what()));
    // TODO: This should be an error
    ers::fatal(issue);
  }
}

void nsw::hw::DeviceManager::disableVmmCaptureInputs() const
{
  // C++20
  // applyFunc(
  //   m_febs,
  //   [](const auto& device) { device.getRoc().disableVmmCaptureInputs(); },
  //   [](const auto& ex) {
  //     nsw::NSWHWConfigIssue issue(
  //       ERS_HERE, fmt::format("Disabling VMM capture inputs failed due to: {}", ex.what()));
  //     ers::fatal(issue);
  //   });
  try {
    if (m_multithreaded) {
      std::vector<std::future<void>> threads{};
      threads.reserve(m_febs.size());
      for (const auto& feb : m_febs) {
        threads.push_back(std::async(
          std::launch::async,
          [](const auto& febLocal) { febLocal.getRoc().disableVmmCaptureInputs(); },
          feb));
      }
      for (auto& thread : threads) {
        thread.get();
      }
    } else {
      for (const auto& feb : m_febs) {
        feb.getRoc().disableVmmCaptureInputs();
      }
    }
  } catch (std::exception& ex) {
    nsw::NSWHWConfigIssue issue(
      ERS_HERE, "Configuration of device failed due to : " + std::string(ex.what()));
    // TODO: This should be an error
    ers::fatal(issue);
  }
}
