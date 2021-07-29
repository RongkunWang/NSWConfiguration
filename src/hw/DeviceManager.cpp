#include "NSWConfiguration/hw/DeviceManager.h"

namespace nsw::hw {
  template<>
  void DeviceManager::add<nsw::FEBConfig>(const nsw::FEBConfig& config)
  {
    addFeb(config);
  }

  template<>
  void DeviceManager::add<nsw::ADDCConfig>(const nsw::ADDCConfig& config)
  {
    addAddc(config);
  }

  template<>
  void DeviceManager::add<nsw::TPConfig>(const nsw::TPConfig& config)
  {
    addTp(config);
  }

  template<>
  void DeviceManager::add<nsw::RouterConfig>(const nsw::RouterConfig& config)
  {
    addRouter(config);
  }

  template<>
  void DeviceManager::add<nsw::PadTriggerSCAConfig>(const nsw::PadTriggerSCAConfig& config)
  {
    addPadTrigger(config);
  }

  template<>
  void DeviceManager::add<nsw::TPCarrierConfig>(const nsw::TPCarrierConfig& config)
  {
    addTpCarrier(config);
  }

  template<>
  void DeviceManager::add<nsw::FEBConfig>(const std::vector<nsw::FEBConfig>& configs)
  {
    for (const auto& config : configs) {
      addFeb(config);
    }
  }

  template<>
  void DeviceManager::add<nsw::ADDCConfig>(const std::vector<nsw::ADDCConfig>& configs)
  {
    for (const auto& config : configs) {
      addAddc(config);
    }
  }

  template<>
  void DeviceManager::add<nsw::TPConfig>(const std::vector<nsw::TPConfig>& configs)
  {
    for (const auto& config : configs) {
      addTp(config);
    }
  }

  template<>
  void DeviceManager::add<nsw::RouterConfig>(const std::vector<nsw::RouterConfig>& configs)
  {
    for (const auto& config : configs) {
      addRouter(config);
    }
  }

  template<>
  void DeviceManager::add<nsw::PadTriggerSCAConfig>(
    const std::vector<nsw::PadTriggerSCAConfig>& configs)
  {
    for (const auto& config : configs) {
      addPadTrigger(config);
    }
  }

  template<>
  void DeviceManager::add<nsw::TPCarrierConfig>(const std::vector<nsw::TPCarrierConfig>& configs)
  {
    for (const auto& config : configs) {
      addTpCarrier(config);
    }
  }

  void DeviceManager::addFeb(const nsw::FEBConfig& config)
  {
    m_rocs.emplace_back(config);
    for (std::size_t counter = 0; counter < config.getTdss().size(); counter++) {
      m_tdss.emplace_back(config, counter);
    }
    for (std::size_t counter = 0; counter < config.getVmms().size(); counter++) {
      m_vmms.emplace_back(config, counter);
    }
  }

  void DeviceManager::addAddc(const nsw::ADDCConfig& config)
  {
    for (std::size_t counter = 0; counter < config.getARTs().size(); counter++) {
      m_arts.emplace_back(config, counter);
    }
  }

  void DeviceManager::addTp(const nsw::TPConfig& config) { m_tps.emplace_back(config); }

  void DeviceManager::addRouter(const nsw::RouterConfig& config) { m_routers.emplace_back(config); }

  void DeviceManager::addPadTrigger(const nsw::PadTriggerSCAConfig& config)
  {
    m_padTriggers.emplace_back(config);
  }

  void DeviceManager::addTpCarrier(const nsw::TPCarrierConfig& config)
  {
    m_tpCarriers.emplace_back(config);
  }

  void DeviceManager::configure()
  {
    const auto conf = [](auto& devices) {
      for (auto& device : devices) {
        device.writeConfiguration();
      }
    };
    conf(m_rocs);
    conf(m_vmms);
    conf(m_tdss);
    conf(m_tps);
    conf(m_routers);
    conf(m_padTriggers);
  }
}  // namespace nsw::hw
