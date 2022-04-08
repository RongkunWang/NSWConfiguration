#ifndef NSWCONFIGURATION_HW_DEVICEMANAGER
#define NSWCONFIGURATION_HW_DEVICEMANAGER

#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <concepts>
#include <span>

#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/hw/FEB.h"
#include "NSWConfiguration/hw/ART.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/MMTP.h"
#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/TPCarrier.h"

ERS_DECLARE_ISSUE(nsw,
                  NSWHWConfigIssue,
                  message,
                  ((std::string)message)
                  )
namespace nsw::hw {
  class DeviceManager
  {
  public:
    /**
     * \brief Construct a new Device Manager object
     *
     * \param multithreaded Configure multithreaded 
     */
    explicit DeviceManager(bool multithreaded = true);

    /**
     * \brief Options for configuring devices
     *
     */
    enum class Options {
      RESET_VMM,
      RESET_TDS,
      DISABLE_VMM_CAPTURE_INPUTS
    };
    /**
     * \brief Add a config to the manager
     *
     * \param config config ptree
     */
    void add(const boost::property_tree::ptree& config)
    {
      constexpr static std::string_view OPC_NODE_ID_KEY{"OpcNodeId"};
      const auto opcNodeId = [&config] () {
        try {
          return config.get<std::string>(std::string{OPC_NODE_ID_KEY});
        } catch (const boost::property_tree::ptree_bad_data& ex) {
          throw NSWHWConfigIssue(ERS_HERE, fmt::format("Did not find {} in ptree", OPC_NODE_ID_KEY));
        }
      }();
      const auto type = nsw::getElementType(config.get<std::string>(std::string{OPC_NODE_ID_KEY}));
      if (type == "MMFE8" or type == "SFEB6" or type == "SFEB8" or type == "PFEB" or type == "SFEB") {
        addFeb(FEBConfig{config});
      }
      else if (type == "ADDC") {
        addAddc(ADDCConfig{config});
      }
      else if (type == "MMTP") {
        addMMTp(TPConfig{config});
      }
      else if (type == "STGCTP") {
        addSTGCTp(config);
      }
      else if (type == "Router") {
        addRouter(RouterConfig{config});
      }
      else if (type == "PadTrigger") {
        addPadTrigger(config);
      }
      else if (type == "TPCarrier") {
        addTpCarrier(TPCarrierConfig{config});
      }
      else {
        throw NSWHWConfigIssue(ERS_HERE, fmt::format("Received an unknown device type {}", type));
      }
    }

    /**
     * \brief Add a span of configs of one type to the manager
     *
     * \param configs span of ptrees
     */
    void add(std::span<const boost::property_tree::ptree> configs);

    /**
     * \brief Add objects to the manager
     *
     * \param configs ptrees
     */
    void add(const std::same_as<boost::property_tree::ptree> auto&... configs)
    {
      (add(configs), ...);
    }

    /**
     * \brief Configure all devices
     *
     * \param options A set of options to be applied
     */
    void configure(std::span<const Options> options = {}) const;

    /**
     * \brief Connect all devices
     */
    void connect() const;

    /**
     * \brief Enable VMM capture inputs of all ROCs
     */
    void enableVmmCaptureInputs() const;

    /**
     * \brief Disable VMM capture inputs of all ROCs
     */
    void disableVmmCaptureInputs() const;

    /**
     * \brief toggle idle state to high for all trigger electronics
     */
    void toggleIdleStateHigh() const;

    /**
     * \brief Get all ARTs
     *
     * \return std::vector<ART>& ARTs
     */
    std::vector<ART>& getArts() { return m_arts; }
    const std::vector<ART>& getArts() const { return m_arts; }  //!< overload

    /**
     * \brief Get all STGCTPs
     *
     * \return std::vector<STGCTP>& TPs
     */
    std::vector<STGCTP>& getSTGCTps() { return m_stgctps; }
    const std::vector<STGCTP>& getSTGCTps() const { return m_stgctps; }  //!< overload

    /**
     * \brief Get all MMTPs
     *
     * \return std::vector<MMTP>& TPs
     */
    std::vector<MMTP>& getMMTps() { return m_mmtps; }
    const std::vector<MMTP>& getMMTps() const { return m_mmtps; }  //!< overload

    /**
     * \brief Get all pad triggers
     *
     * \return std::vector<PadTrigger>& Pad triggers
     */
    std::vector<PadTrigger>& getPadTriggers() { return m_padTriggers; }
    const std::vector<PadTrigger>& getPadTriggers() const { return m_padTriggers; }  //!< overload

    /**
     * \brief Get all routers
     *
     * \return std::vector<Router>& Routers
     */
    std::vector<Router>& getRouters() { return m_routers; }
    const std::vector<Router>& getRouters() const { return m_routers; }  //!< overload

    /**
     * \brief Get all TP carriers
     *
     * \return std::vector<TPCarrier>& TP carriers
     */
    std::vector<TPCarrier>& getTpCarriers() { return m_tpCarriers; }
    const std::vector<TPCarrier>& getTpCarriers() const { return m_tpCarriers; }  //!< overload

    /**
     * \brief Get all FEBs
     *
     * \return std::vector<FEB>& FEBs
     */
    std::vector<FEB>& getFebs() { return m_febs; }
    const std::vector<FEB>& getFebs() const { return m_febs; }  //!< overload

    /**
     * \brief Clear all managed devices and OPC connections
     */
    void clear();

    /**
     * \brief Clear all OPC connections
     */
    void clearOpc();

    /**
     * \brief Set the command sender to the RC application
     *
     * \param sender Command sender to RC application
     */
    void setCommandSender(nsw::CommandSender&& sender) { m_opcManager.setCommandSender(std::move(sender)); }

  private:
    bool m_multithreaded{};
    OpcManager m_opcManager{};
    std::vector<nsw::hw::FEB> m_febs{};
    std::vector<ART> m_arts{};
    std::vector<MMTP> m_mmtps{};
    std::vector<STGCTP> m_stgctps{};
    std::vector<Router> m_routers{};
    std::vector<PadTrigger> m_padTriggers{};
    std::vector<TPCarrier> m_tpCarriers{};

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
     * \brief Add MMTP from TPConfig object
     *
     * \param config config object
     */
    void addMMTp(const nsw::TPConfig& config);

    /**
     * \brief Add STGCTP from ptree directly
     *
     * \param ptree object
     */
    void addSTGCTp(const boost::property_tree::ptree& config);

    /**
     * \brief Add Router from RouterConfig object
     *
     * \param config config object
     */
    void addRouter(const nsw::RouterConfig& config);

    /**
     * \brief Add PadTrigger object
     *
     * \param config config object
     */
    void addPadTrigger(const boost::property_tree::ptree& config);

    /**
     * \brief Add TP Carrier from TPCarrierConfig object
     *
     * \param config config object
     */
    void addTpCarrier(const nsw::TPCarrierConfig& config);

    /**
     * \brief Apply a function to a range of devices and handle exceptions
     *
     * \param devices Range of devices
     * \param func Function to be applied
     * \param exceptionHandler Function to handle exceptions
     */
    template<std::ranges::range Range>
    void applyFunc(const Range& devices,
                   const std::regular_invocable<typename Range::value_type> auto& func,
                   const std::regular_invocable<std::exception> auto& exceptionHandler) const
    {
      try {
        if (m_multithreaded) {
          std::vector<std::future<void>> threads{};
          threads.reserve(devices.size());
          for (const auto& device : devices) {
            threads.push_back(std::async(
              std::launch::async, [&func](const auto& deviceLocal) { func(deviceLocal); }, device));
          }
          for (auto& thread : threads) {
            thread.get();
          }
        } else {
          for (const auto& device : devices) {
            func(device);
          }
        }
      } catch (const OpcReadWriteIssue& ex) {
        throw NSWConfigurationOpcError(ERS_HERE, ex.what());
      } catch (const OpcConnectionIssue& ex) {
        throw NSWConfigurationOpcError(ERS_HERE, ex.what());
      } catch (std::exception& ex) {
        exceptionHandler(ex);
      }
    }
  };
}  // namespace nsw::hw

#endif
