#ifndef NSWCONFIGURATION_ROCINTERFACE_H
#define NSWCONFIGURATION_ROCINTERFACE_H

#include "NSWConfiguration/FEBConfig.h"

namespace nsw::DeviceInterface::ROC {
  std::map<std::uint8_t, std::uint8_t> readConfiguration(
    const nsw::FEBConfig& config);

  void writeConfiguration(const nsw::FEBConfig& config);

  std::uint8_t readRegister(const nsw::FEBConfig& config,
                            std::uint8_t          registerId);

  void writeRegister(const nsw::FEBConfig& config,
                     std::uint8_t          registerId,
                     std::uint8_t          value);

  void writeRegister(const nsw::FEBConfig& config,
                     const std::string&    regAddress,
                     std::uint8_t          value);

  /// Set vmm enable register to 0
  /// \param feb Config needed for OPC IP
  void disableVmmCaptureInputs(const nsw::FEBConfig& feb);

  /// Set vmm enable register to value in passed config
  /// \param feb Reference config
  void enableVmmCaptureInputs(const nsw::FEBConfig& feb);
}  // namespace nsw::DeviceInterface::ROC

#endif