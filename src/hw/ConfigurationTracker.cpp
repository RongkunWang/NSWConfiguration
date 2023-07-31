#include "NSWConfiguration/hw/ConfigurationTracker.h"

#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>

#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/FEBConfig.h"

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::update(const RegAddress reg,
                                                                const Value value)
{
  m_currentData.insert_or_assign(reg, value);
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::update(const Data& values)
{
  for (const auto& [key, value] : values) {
    m_currentData.insert_or_assign(key, value);
  }
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::update(const DataVector& values)
{
  update(convertVector(values));
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::validate(const RegAddress reg,
                                                                  const Value value)
{
  const auto result = check(reg, value);
  updateErrors(reg, result);
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::validate(const Data& values)
{
  for (const auto& [reg, value] : values) {
    validate(reg, value);
  }
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::validate(const DataVector& values)
{
  validate(convertVector(values));
}

template<nsw::hw::internal::DeviceType Device>
std::set<typename nsw::hw::internal::ConfigurationTrackerMap<Device>::RegAddress>
nsw::hw::internal::ConfigurationTrackerMap<Device>::getErrors() const
{
  return m_errors;
}

template<nsw::hw::internal::DeviceType Device>
bool nsw::hw::internal::ConfigurationTrackerMap<Device>::check(const RegAddress reg,
                                                               const Value value) const
{
  if (m_currentData.find(reg) == std::cend(m_currentData)) {
    ERS_LOG(
      "Cannot validate "
      << reg
      << ", since it is not part of the configuration. Probably a status register. Skipping.");
    return true;
  }
  return m_currentData.at(reg) == value;
}

template<nsw::hw::internal::DeviceType Device>
void nsw::hw::internal::ConfigurationTrackerMap<Device>::updateErrors(const RegAddress reg, const bool result)
{
  if (result) {
    if (m_errors.contains(reg)) {
      m_errors.erase(reg);
    }
  } else {
    m_errors.emplace(reg);
  }
}

template<nsw::hw::internal::DeviceType Device>
typename nsw::hw::internal::ConfigurationTrackerMap<Device>::Data
nsw::hw::internal::ConfigurationTrackerMap<Device>::convertVector(const DataVector& values)
{
  Data result{};
  std::ranges::transform(values,
                         std::inserter(result, std::end(result)),
                         [reg = RegAddress{}](const auto& value) mutable ->
                         typename decltype(result)::value_type {
                           return {reg++, value};
                         });
  return result;
}

template class nsw::hw::internal::ConfigurationTrackerMap<nsw::hw::internal::DeviceType::GBTx>;
template class nsw::hw::internal::ConfigurationTrackerMap<nsw::hw::internal::DeviceType::ROC>;
