#ifndef NSWCONFIGURATION_VMMINTERFACE_H
#define NSWCONFIGURATION_VMMINTERFACE_H

#include "NSWConfiguration/FEBConfig.h"

namespace nsw::DeviceInterface::VMM {
  std::map<std::uint8_t, std::vector<std::uint8_t>> readConfiguration(
    [[maybe_unused]] const nsw::FEBConfig& config,
    [[maybe_unused]] std::size_t           numVmm);

  void writeConfiguration(const nsw::FEBConfig& config, std::size_t numVmm);

  std::vector<std::uint8_t> readRegister(
    [[maybe_unused]] const nsw::FEBConfig& config,
    [[maybe_unused]] std::size_t           numVmm,
    [[maybe_unused]] std::uint8_t          registerId);

  void writeRegister([[maybe_unused]] const nsw::FEBConfig& config,
                     [[maybe_unused]] std::size_t           numVmm,
                     [[maybe_unused]] std::uint8_t          registerId,
                     [[maybe_unused]] std::uint64_t         value);

  void writeRegister([[maybe_unused]] const nsw::FEBConfig& config,
                     [[maybe_unused]] std::size_t           numVmm,
                     [[maybe_unused]] const std::string&    regAddress,
                     [[maybe_unused]] std::uint64_t         value);
}  // namespace nsw::DeviceInterface::VMM

#endif