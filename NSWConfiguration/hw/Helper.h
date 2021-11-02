#ifndef NSWCONFIGURATION_HW_HELPER_H
#define NSWCONFIGURATION_HW_HELPER_H


#include <algorithm>
#include <stdexcept>

#include "NSWConfiguration/ConfigConverter.h"

/**
 * \brief This namespace contains internal implementation details of the HW interfaces
 *
 * Every device should have its own namespace within the internal namespace. This file
 * should contain all functions that would be private static helper functions.
 */
namespace nsw::hw::internal {
  namespace ROC {
    /**
     * \brief Determines if a range of values belong to the analog or digital part
     *
     * \param values range of names of values
     * \tparam Range Iterable list of strings
     * \throws std::logic_error \ref values do not contain only analog or only digital values
     */
    template<typename Range>  // add requires with c++20
    bool namesAnalog(const Range& names)
    {
      const auto onlyDigital =
        std::all_of(std::cbegin(names), std::cend(names), [](const auto& path) {
          return TRANSLATION_MAP_ROC_DIGITAL.find(path) != std::cend(TRANSLATION_MAP_ROC_DIGITAL);
        });
      const auto onlyAnalog =
        std::all_of(std::cbegin(names), std::cend(names), [](const auto& path) {
          return TRANSLATION_MAP_ROC_ANALOG.find(path) != std::cend(TRANSLATION_MAP_ROC_ANALOG);
        });
      if (not onlyAnalog and not onlyDigital) {
        throw std::logic_error(
          "Passed a list of mixed analog and digital values or invalid values");
      }
      return onlyAnalog;
    }
  }  // namespace ROC
}  // namespace nsw::hw::internal

#endif
