#ifndef NSWCONFIGURATION_TDSINTERFACE_H
#define NSWCONFIGURATION_TDSINTERFACE_H

#include "NSWConfiguration/FEBConfig.h"

namespace nsw::DeviceInterface::TDS {
  std::map<std::uint8_t, std::vector<std::uint8_t>> readConfiguration(
    const nsw::FEBConfig& config,
    std::size_t           numTds);

  void writeConfiguration(const nsw::FEBConfig& config,
                          std::size_t           numTds,
                          bool                  reset_tds);

  std::vector<std::uint8_t> readRegister(const nsw::FEBConfig& config,
                                         std::size_t           numTds,
                                         std::uint8_t          registerId);

  void writeRegister(const nsw::FEBConfig& config,
                     std::size_t           numTds,
                     std::uint8_t          registerId,
                     __uint128_t           value);

  void writeRegister(const nsw::FEBConfig& config,
                     std::size_t           numTds,
                     const std::string&    regAddress,
                     __uint128_t           value);
}  // namespace nsw::DeviceInterface::TDS

#endif