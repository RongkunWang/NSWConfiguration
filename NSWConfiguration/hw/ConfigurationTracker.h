#ifndef NSWCONFIGURATION_HW_CONFIGURATIONTRACKER_H
#define NSWCONFIGURATION_HW_CONFIGURATIONTRACKER_H

#include <map>
#include <set>
#include <string>

#include <ers/ers.h>


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
    using valueType = __uint128_t;
  };

  /**
   * \brief ROC 8 bit registers
   */
  template<>
  struct configTypeSelector<DeviceType::ROC> {
    using valueType = std::uint8_t;
  };

  /**
   * \brief TP 32 bit registers TODO
   */
  template<>
  struct configTypeSelector<DeviceType::TP> {
    using valueType = std::uint32_t;
  };

  /**
   * \brief TPCarrier 32 bit registers TODO
   */
  template<>
  struct configTypeSelector<DeviceType::TPCarrier> {
    using valueType = std::uint32_t;
  };

  /**
   * \brief ART 8 bit registers
   */
  template<>
  struct configTypeSelector<DeviceType::ART> {
    using valueType = std::uint8_t;
  };

  template<DeviceType Device>
  using valueType_t = typename configTypeSelector<Device>::valueType;

  template<DeviceType Device>
  class ConfigurationTrackerMap
  {
    using RegAddress = std::uint8_t;
    using Value = valueType_t<Device>;
    using Data = std::map<RegAddress, Value>;
  public:
    void update(RegAddress reg, Value value);
    void update(const Data& values);
    void validate(RegAddress reg, Value value);
    void validate(const Data& values);
    [[nodiscard]] std::set<RegAddress> getErrors() const;

  private:
    [[nodiscard]] bool check(RegAddress reg, Value value) const;
    void updateErrors(RegAddress reg, bool result);

    Data m_currentData{};
    std::set<RegAddress> m_errors{};
  };

}  // namespace nsw::hw::internal

#endif