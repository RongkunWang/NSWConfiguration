#ifndef NSWCONFIGURATION_NSWCONFIGURATION_CONCEPTS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_CONCEPTS_H

#include <type_traits>

#include "NSWConfiguration/hw/FEB.h"

namespace nsw {
  template<typename T>
  concept HWIDevice = requires(T hwi)
  {
    hwi.writeConfiguration();
    hwi.readConfiguration();
  };

  template<typename T>
  concept HWIContainer = std::is_same_v<T, nsw::hw::FEB>;

  template<typename T>
  concept HWI = HWIContainer<T> || HWIDevice<T>;
}  // namespace nsw

#endif
