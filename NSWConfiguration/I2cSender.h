#ifndef NSWCONFIGURATION_I2CSENDER_H
#define NSWCONFIGURATION_I2CSENDER_H

#include <vector>
#include <string>

#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/Constants.h"

namespace nsw::ConfigSender::I2c {
  /// Low level I2c send function
  void sendI2c(const std::unique_ptr<nsw::OpcClient>& opc_connection,
               const std::string&                     node,
               const std::vector<uint8_t>&            vdata);

  /// Low level I2c send function
  void sendI2cRaw(const std::unique_ptr<nsw::OpcClient>& opc_connection,
                  const std::string&                     node,
                  const uint8_t*                         data,
                  size_t                                 data_size);

  /// Low level GPIO send function
  void sendGPIO(const std::unique_ptr<nsw::OpcClient>& opc_connection,
                const std::string&                     node,
                bool                                   data);

  /// Read  back GPIO register
  bool readGPIO(const std::unique_ptr<nsw::OpcClient>& opc_connection,
                const std::string&                     node);

  // Read back I2c register as vector
  std::vector<uint8_t> readI2c(
    const std::unique_ptr<nsw::OpcClient>& opc_connection,
    const std::string&                     node,
    size_t                                 number_of_bytes = 1);

  // Read back I2c register as vector for ADDC
  std::vector<uint8_t> readI2cAtAddress(
    const std::unique_ptr<nsw::OpcClient>& opc_connection,
    const std::string&                     node,
    const uint8_t*                         address,
    size_t                                 address_size,
    size_t                                 number_of_bytes = 1);

  // Send I2c register as vector for ADDC
  void sendI2cAtAddress(const std::unique_ptr<nsw::OpcClient>& opc_connection,
                        const std::string&                     node,
                        const std::vector<uint8_t>&            address,
                        std::vector<uint8_t>                   data);

  /// High level send function to send configuration to all addresses under an
  /// I2cMaster
  void sendI2cMasterConfig(
    const std::unique_ptr<nsw::OpcClient>& opc_connection,
    const std::string&                     topnode,
    const nsw::I2cMasterConfig&            cfg);

  /// Write only one of the register addresses in I2cMaster
  void sendI2cMasterSingle(
    const std::unique_ptr<nsw::OpcClient>& opc_connection,
    const std::string&                     topnode,
    const nsw::I2cMasterConfig&            cfg,
    const std::string&                     reg_address);

  void sendI2cMasterSingle(
    const std::unique_ptr<nsw::OpcClient>& opc_connection,
    const std::string&                     node,
    const std::vector<uint8_t>&            data,
    const std::string&                     reg_address);
}  // namespace nsw::ConfigSender::I2c

#endif