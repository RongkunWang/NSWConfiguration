#ifndef NSWCONFIGURATION_HW_HELPER_H
#define NSWCONFIGURATION_HW_HELPER_H

#include <span>
#include <string>

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
     * \throws std::logic_error \ref values do not contain only analog or only digital values
     */
    bool namesAnalog(std::span<const std::string> names);
  }  // namespace ROC
}  // namespace nsw::hw::internal

#endif
