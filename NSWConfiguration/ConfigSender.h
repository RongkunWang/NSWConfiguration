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

namespace nsw {
class ConfigSender {
 private:
    /// Map with key: opc client ip, value: OpcClient instance
    std::map<std::string, std::unique_ptr<nsw::OpcClient>> m_clients;
    /* data */

    /// Add new client if it connects to a new Opc Server
    void addOpcClientIfNew(std::string opcserver_ipport);

 public:
    ConfigSender();
    ~ConfigSender() {}  // Disconnect from Opc Server(s)?

    /// High level send function
    void sendVmmConfig(const nsw::VMMConfig& vmm);

    /// High level send function
    void sendRocConfig(const nsw::ROCConfig& roc);

    /// Low level Spi send function
    void sendSpiRaw(std::string opcserver_ipport, std::string node, uint8_t *data, size_t data_size);

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
