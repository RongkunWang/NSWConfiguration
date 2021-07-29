#ifndef NSWCONFIGURATION_HW_DEVICEMANAGER
#define NSWCONFIGURATION_HW_DEVICEMANAGER

#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/ROC.h"
#include "NSWConfiguration/hw/TDS.h"
#include "NSWConfiguration/hw/VMM.h"
#include "NSWConfiguration/hw/ART.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/TP.h"
#include "NSWConfiguration/hw/TPCarrier.h"

namespace nsw::hw {
  class DeviceManager
  {
    template<typename... Bases>
    struct Visitor : Bases... {
      using Bases::operator()...;
    };

#if !defined(__GNUG__)
    template<class... Bases>
    Visitor(Bases...) -> Visitor<Bases...>;
#endif

  public:
    /**
     * \brief Add a config to the manager
     *
     * \tparam Config <Device>Config class
     * \param config config object
     */
    template<typename Config>
    void add([[maybe_unused]] const Config& config)
    {
      throw std::runtime_error("Unknown config type. Provide a <Device>Config object.");
    }

    /**
     * \brief Add a vector of configs of one type to the manager
     *
     * \tparam Config <Device>Config class
     * \param configs vector of config objects
     */
    template<typename Config>
    void add([[maybe_unused]] const std::vector<Config>& configs)
    {
      throw std::runtime_error("Unknown config type. Provide a <Device>Config object.");
    }

    /**
     * \brief Add objects to the manager
     *
     * \tparam Configs <Device>Config classes
     * \param configs <Device>Config objects
     */
    template<typename... Configs>
    void add(const Configs&... configs)
    {
      Visitor visitor{
        [&](const nsw::FEBConfig& config) { addFeb(config); },
        [&](const nsw::ADDCConfig& config) { addAddc(config); },
        [&](const nsw::TPConfig& config) { addTp(config); },
        [&](const nsw::RouterConfig& config) { addRouter(config); },
        [&](const nsw::PadTriggerSCAConfig& config) { addPadTrigger(config); },
        [&](const nsw::TPCarrierConfig& config) { addTpCarrier(config); },
        [&](const nsw::VMMConfig&) {
          throw std::runtime_error("VMM configs are added through the parent FEBConfig object");
        },
        [&](const nsw::I2cMasterConfig&) {
          throw std::runtime_error(
            "ROC and TDS configs are added through the parent FEBConfig object");
        },
        [&](const nsw::ARTConfig&) {
          throw std::runtime_error("ART configs are added through the parent ADDCConfig object");
        },
        [&](const auto&) {
          throw std::runtime_error("Unknown config type. Provide a <Device>Config object.");
        }};
      (Visitor(configs), ...);
    }

    /**
     * \brief Configure all devices
     *
     */
    void configure();

  private:
    std::vector<nsw::hw::ROC> m_rocs;
    std::vector<nsw::hw::VMM> m_vmms;
    std::vector<nsw::hw::TDS> m_tdss;
    std::vector<ART> m_arts;
    std::vector<TP> m_tps;
    std::vector<Router> m_routers;
    std::vector<PadTrigger> m_padTriggers;
    std::vector<TPCarrier> m_tpCarriers;

    /**
     * \brief Add ROC, TDSs, and VMMs from FEBConfig object
     *
     * \param config config object
     */
    void addFeb(const nsw::FEBConfig& config);

    /**
     * \brief Add ARTs from ADDCConfig object
     *
     * \param config config object
     */
    void addAddc(const nsw::ADDCConfig& config);

    /**
     * \brief Add TP from TPConfig object
     *
     * \param config config object
     */
    void addTp(const nsw::TPConfig& config);

    /**
     * \brief Add Router from RouterConfig object
     *
     * \param config config object
     */
    void addRouter(const nsw::RouterConfig& config);

    /**
     * \brief Add PadTrigger from PadTriggerSCAConfig object
     *
     * \param config config object
     */
    void addPadTrigger(const nsw::PadTriggerSCAConfig& config);

    /**
     * \brief Add TP Carrier from TPCarrierConfig object
     *
     * \param config config object
     */
    void addTpCarrier(const nsw::TPCarrierConfig& config);
  };
}  // namespace nsw::hw

#endif
