// Class to send configuration to a front end using OPCUA server

#ifndef NSWCONFIGURATION_CONFIGSENDER_H_
#define NSWCONFIGURATION_CONFIGSENDER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/TDSConfig.h"

namespace nsw {
class ConfigSender {
 private:
    /// Map with key: opc client ip, value: OpcClient instance
    std::map<std::string, std::unique_ptr<nsw::OpcClient>> m_clients;

    /// Add new client if it connects to a new Opc Server
    void addOpcClientIfNew(std::string opcserver_ipport);

 public:
    ConfigSender();
    ~ConfigSender() {}  // Disconnect from Opc Server(s)?

    /// High level send function - TODO(cyildiz): deprecate
    void sendVmmConfig(const nsw::VMMConfig& vmm);

    /// High level send function - TODO(cyildiz): deprecate
    void sendRocConfig(const nsw::ROCConfig& roc);

    /// Send configuration to roc
    void sendRocConfig(std::string opc_ip, std::string sca_address,
                       const I2cMasterConfig & analog, const I2cMasterConfig & digital);

    /// Send configuration to tds
    void sendTdsConfig(std::string opc_ip, std::string sca_address, const I2cMasterConfig & tds);

    /// Send configuration to roc in the feb
    void sendRocConfig(const nsw::FEBConfig& feb);

    /// Send configuration to all vmms in the feb
    void sendVmmConfig(const nsw::FEBConfig& feb);

    /// Send configuration to all tds in the feb
    void sendTdsConfig(const nsw::FEBConfig& feb);

    /// High level send function - TODO(cyildiz): deprecate
    void sendTdsConfig(const nsw::TDSConfig& tds);

    /// High level send function to send configuration to all addresses under an I2cMaster
    void sendI2cMasterConfig(std::string opcserver_ipport, std::string topnode, const nsw::I2cMasterConfig& cfg);

    /// Write only one of the register addresses in I2cMaster
    void sendI2cMasterSingle(std::string opcserver_ipport, std::string topnode, const nsw::I2cMasterConfig& cfg,
                             std::string reg_address);

    /// Low level Spi send function
    void sendSpiRaw(std::string opcserver_ipport, std::string node, uint8_t *data, size_t data_size);

    /// Low level Spi send function
    void sendSpi(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata);

    /// Low level I2c send function
    void sendI2c(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata);

    /// Low level I2c send function
    void sendI2cRaw(std::string opcserver_ipport, std::string node, uint8_t *data, size_t data_size);

    /// Low level GPIO send function
    void sendGPIO(std::string opcserver_ipport, std::string node, bool data);

    /// Read  back GPIO register
    bool readGPIO(std::string opcserver_ipport, std::string node);

    // Read back I2c register as vector
    std::vector<uint8_t> readI2c(std::string opcserver_ipport, std::string node);
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_CONFIGSENDER_H_
