#ifndef NSWCONFIGURATION_HW_DEVICEMANAGER
#define NSWCONFIGURATION_HW_DEVICEMANAGER

// #include <exception> // C++20
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
// #include <execution> // C++20
// #include <ranges> // C++20

#include "NSWConfiguration/hw/FEB.h"
#include "NSWConfiguration/hw/ART.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/TP.h"
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
      if (type == "MMFE8" or type == "SFEB" or type == "PFEB") {
        addFeb(FEBConfig{config});
      }
      else if (type == "ADDC") {
        addAddc(ADDCConfig{config});
      }
      else if (type == "TP") {
        addTp(TPConfig{config});
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
     * \brief Add a range of configs of one type to the manager
     *
     * \param configs range of config objects
     */
    // C++20
    // void add(const std::ranges::range auto& configs)
    // {
    //   for (const auto& config : configs) {
    //     add(config);
    //   }
    // }

    /**
     * \brief Add objects to the manager
     *
     * \tparam Configs <Device>Config classes
     * \param configs <Device>Config objects
     */
    template<typename... Configs>
    void add(const Configs&... configs)
    {
      (add(configs), ...);
    }

    /**
     * \brief Configure all devices
     *
     * \param options A set of options to be applied
     */
    void configure(const std::vector<Options>& options = {}) const;

    /**
     * \brief Enable VMM capture inputs of all ROCs
     */
    void enableVmmCaptureInputs() const;

    /**
     * \brief Disable VMM capture inputs of all ROCs
     */
    void disableVmmCaptureInputs() const;

    /**
     * \brief Get all ARTs
     *
     * \return std::vector<ART>& ARTs
     */
    std::vector<ART>& getArts() { return m_arts; }
    const std::vector<ART>& getArts() const { return m_arts; }  //!< overload

    /**
     * \brief Get all TPs
     *
     * \return std::vector<TP>& TPs
     */
    std::vector<TP>& getTps() { return m_tps; }
    const std::vector<TP>& getTps() const { return m_tps; }  //!< overload

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
  private:
    bool m_multithreaded{};
    OpcManager m_opcManager{};
    std::vector<nsw::hw::FEB> m_febs{};
    std::vector<ART> m_arts{};
    std::vector<TP> m_tps{};
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
     * @brief Apply a function to a range of devices and handle exceptions
     * 
     * @param devices Range of devices
     * @param func Function to be applied
     * @param exceptionHandler Function to handle exceptions
     */
    // C++20
    // template<std::ranges::range Range>
    // void applyFunc(const Range &devices,
    //                const std::regular_invocable<typename Range::value_type> auto &func,
    //                const std::regular_invocable<std::exception> auto &exceptionHandler) {
    // {
    //   std::exception_ptr eptr{};
    //   const auto funcWithExceptionHandler = [&func, &eptr] (const auto& device) mutable {
    //     try {
    //       func(device);
    //     }
    //     catch (...) {
    //       eptr = std::current_exception();
    //     }
    //   };
    //   const auto funcWithPolicy = [&funcWithExceptionHandler, &devices] (const auto policy) {
    //     std::for_each(policy, std::cbegin(devices), std::cend(devices), funcWithExceptionHandler);
    //   };

    //   // Call the function
    //   if (m_multithreaded) {
    //     funcWithPolicy(std::execution::par_unseq);
    //   }
    //   else {
    //     funcWithPolicy(std::execution::seq);
    //   }

    //   // Handle exceptions
    //   if (eptr) {
    //     try {
    //       std::rethrow_exception(eptr);
    //     }
    //     catch (const std::exception& e) {
    //       exceptionHandler(e);
    //     }
    //   }
    // }
  };

}  // namespace nsw::hw

#endif
