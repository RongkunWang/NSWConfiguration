#ifndef NSWCONFIGURATION_CONFIGSENDERROC_H
#define NSWCONFIGURATION_CONFIGSENDERROC_H

#include "NSWConfiguration/FEBConfig.h"

namespace nsw::ConfigSender::ROC {
  std::map<std::uint8_t, std::uint8_t> readConfiguration(
    const std::string& opcserver_ipport,
    const std::string& sca_address);

  void writeConfiguration(const nsw::FEBConfig& config);

  std::uint8_t readRegister(const std::string& opcserver_ipport,
                            const std::string& sca_address,
                            std::uint8_t       registerId);

  void writeRegister(const std::string& opcserver_ipport,
                     const std::string& sca_address,
                     std::uint8_t       registerId,
                     std::uint8_t       value);

  void writeRegister(const std::string& opcserver_ipport,
                     const std::string& sca_address,
                     const std::string& regAddress,
                     std::uint8_t       value);

  /// Set vmm enable register to 0
  /// \param feb Config needed for OPC IP
  void disableVmmCaptureInputs(const nsw::FEBConfig& feb);

  /// Set vmm enable register to value in passed config
  /// \param feb Reference config
  void enableVmmCaptureInputs(const nsw::FEBConfig& feb);
}  // namespace nsw::ConfigSender::ROC

#endif