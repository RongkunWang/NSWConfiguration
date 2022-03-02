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
     * \tparam Config <Device>Config class
     * \param config config object
     */
    template<typename Config>
    void add(const Config& config)
    {
      static_assert(!std::is_same_v<std::decay_t<decltype(config)>, nsw::VMMConfig>,
                    "VMM configs are added through the parent FEBConfig object");
      static_assert(!std::is_same_v<std::decay_t<decltype(config)>, nsw::I2cMasterConfig>,
                    "ROC and TDS configs are added through the parent FEBConfig object");
      static_assert(!std::is_same_v<std::decay_t<decltype(config)>, nsw::ARTConfig>,
                    "ART configs are added through the parent ADDCConfig object");
      static_assert(std::is_same_v<std::decay_t<decltype(config)>, nsw::FEBConfig> ||
                      std::is_same_v<std::decay_t<decltype(config)>, nsw::ADDCConfig> ||
                      std::is_same_v<std::decay_t<decltype(config)>, nsw::TPConfig> ||
                      std::is_same_v<std::decay_t<decltype(config)>, nsw::RouterConfig> ||
                      std::is_same_v<std::decay_t<decltype(config)>, nsw::hw::PadTrigger> ||
                      std::is_same_v<std::decay_t<decltype(config)>, nsw::TPCarrierConfig>,
                    "Unknown config type. Provide a <Device>Config object");
      if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::FEBConfig>) {
        addFeb(config);
      }
      else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::ADDCConfig>) {
        addAddc(config);
      }
      else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::TPConfig>) {
        addTp(config);
      }
      else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::RouterConfig>) {
        addRouter(config);
      }
      else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::hw::PadTrigger>) {
        addPadTrigger(config);
      }
      else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, nsw::TPCarrierConfig>) {
        addTpCarrier(config);
      }
      else {
        throw std::logic_error("If you see me, fix the static asserts above me!");
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

  private:
    bool m_multithreaded{};
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
    void addPadTrigger(const nsw::hw::PadTrigger& config);

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
