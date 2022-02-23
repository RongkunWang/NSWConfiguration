#include "NSWConfiguration/hw/ConfigurationTracker.h"

#include <stdexcept>
#include <type_traits>

#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/FEBConfig.h"

template<nsw::hw::internal::DeviceType Device>
nsw::hw::internal::ConfigurationTrackerMap<Device>::ConfigurationTrackerMap(
  const configType_t<Device>& config) :
  m_initialData{transformConfig(config)},
  m_currentData{m_initialData}
{}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::update(const std::string& key, valueType_t<Device> value) {
  m_currentData.at(key) = value;
}

template<nsw::hw::internal::DeviceType Device>
bool nsw::hw::internal::ConfigurationTrackerMap<Device>::validate(const std::string& key, valueType_t<Device> value) const {
  if (m_currentData.find(key) != std::cend(m_currentData)) {
    ERS_INFO("Cannot validate " << key << ", since it is not part of the configuration. Probably a status register. Skipping.");
    return true;
  }
  return m_currentData.at(key) == value;
}

template<nsw::hw::internal::DeviceType Device>
bool nsw::hw::internal::ConfigurationTrackerMap<Device>::checkFullWrite(const configType_t<Device>& config) const {
  const auto transformedConfig = transformConfig(config);
  return transformedConfig == m_currentData or transformedConfig == m_initialData;
}

template<nsw::hw::internal::DeviceType Device>
auto nsw::hw::internal::ConfigurationTrackerMap<Device>::transformConfig(
  const configType_t<Device>& config) const -> Data
{
  if constexpr (std::is_same_v<configType_t<Device>, boost::property_tree::ptree>) {
    return transformPtreetoMap<valueType_t<Device>>(config);
  }
  if constexpr (std::is_same_v<configType_t<Device>, nsw::ROCConfig>) {
    const auto converterAnalog = ConfigConverter<ConfigConversionType::ROC_ANALOG>(
      config.getAnalog().getConfig(), ConfigType::REGISTER_BASED);
    const auto converterDigital = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(
      config.getDigital().getConfig(), ConfigType::REGISTER_BASED);
    return transformPtreetoMap<valueType_t<Device>>(
      converterAnalog.getFlatRegisterBasedConfig(config.getAnalog().getBitstreamMap()) +
      converterDigital.getFlatRegisterBasedConfig(config.getDigital().getBitstreamMap()));
  }
  throw std::runtime_error("Cannot create config map");
}

template class nsw::hw::internal::ConfigurationTrackerMap<nsw::hw::internal::DeviceType::ROC>;
