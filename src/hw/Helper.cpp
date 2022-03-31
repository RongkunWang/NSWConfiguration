#include "NSWConfiguration/hw/Helper.h"

#include <algorithm>
#include <stdexcept>

#include "NSWConfiguration/ConfigConverter.h"

bool nsw::hw::internal::ROC::namesAnalog(const std::span<const std::string> names)
{
  const auto onlyDigital = std::all_of(std::cbegin(names), std::cend(names), [](const auto& path) {
    return TRANSLATION_MAP_ROC_DIGITAL.find(path) != std::cend(TRANSLATION_MAP_ROC_DIGITAL);
  });
  const auto onlyAnalog = std::all_of(std::cbegin(names), std::cend(names), [](const auto& path) {
    return TRANSLATION_MAP_ROC_ANALOG.find(path) != std::cend(TRANSLATION_MAP_ROC_ANALOG);
  });
  if (not onlyAnalog and not onlyDigital) {
    throw std::logic_error("Passed a list of mixed analog and digital values or invalid values");
  }
  return onlyAnalog;
}
