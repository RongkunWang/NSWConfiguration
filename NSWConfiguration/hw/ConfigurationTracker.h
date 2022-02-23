#ifndef NSWCONFIGURATION_HW_CONFIGURATIONTRACKER_H
#define NSWCONFIGURATION_HW_CONFIGURATIONTRACKER_H

#include <map>
#include <string>

#include <ers/ers.h>

#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/TPConfig.h"
#include "NSWConfiguration/TPCarrierConfig.h"
#include "NSWConfiguration/RouterConfig.h"

namespace nsw {
ERS_DECLARE_ISSUE(hw,
                  FullConfigurationIssue,
                  "Write configuration is not used to reset " << node << " and does not correspond to the current configuration.",
                  ((std::string)node)
                  )
} // namespace nsw

namespace nsw::hw::internal {
  /**
   * \brief Used to infer the conversion map
   */
  enum class DeviceType { ROC, TDS, ART, PadTrigger, Router, TP, TPCarrier };

  /**
   * \brief Type selector for the translation map (depends on device type)
   *
   * Defines translation map type and register size for each device type. Every device needs
   * a template specialization of this template (see below for examples or ask @joroemer in
   * case of doubt).
   *
   * \tparam DeviceType Device type
   */
  template<DeviceType>
  struct configTypeSelector {};

  /**
   * \brief TDS 128 bit registers
   */
  template<>
  struct configTypeSelector<DeviceType::TDS> {
    using configType = boost::property_tree::ptree;
    using valueType = __uint128_t;
  };

  /**
   * \brief ROC 8 bit registers
   */
  template<>
  struct configTypeSelector<DeviceType::ROC> {
    using configType = ROCConfig;
    using valueType = std::uint8_t;
  };

  /**
   * \brief TP 32 bit registers TODO
   */
  template<>
  struct configTypeSelector<DeviceType::TP> {
    using configType = boost::property_tree::ptree;
    using valueType = std::uint32_t;
  };

  /**
   * \brief TPCarrier 32 bit registers TODO
   */
  template<>
  struct configTypeSelector<DeviceType::TPCarrier> {
    using configType = boost::property_tree::ptree;
    using valueType = std::uint32_t;
  };

  /**
   * \brief ART 8 bit registers
   */
  template<>
  struct configTypeSelector<DeviceType::ART> {
    using configType = boost::property_tree::ptree;
    using valueType = std::uint8_t;
  };

  // Do not touch those for new devices
  template<DeviceType Device>
  using configType_t = typename configTypeSelector<Device>::configType;

  template<DeviceType Device>
  using valueType_t = typename configTypeSelector<Device>::valueType;

  template<DeviceType Device>
  class ConfigurationTrackerMap
  {
  public:
    explicit ConfigurationTrackerMap(const configType_t<Device>& config);
    void update(const std::string& key, valueType_t<Device> value);
    void update(const configType_t<Device>& config);
    bool validate(const std::string& key, valueType_t<Device> value) const;
    [[nodiscard]] bool checkFullWrite(const configType_t<Device>& config) const;

  private:
    using Data = std::map<std::string, valueType_t<Device>>;
    [[nodiscard]] Data transformConfig(const configType_t<Device>& config) const;
    Data m_initialData{};
    Data m_currentData{};
  };

}  // namespace nsw::hw::internal

#endif