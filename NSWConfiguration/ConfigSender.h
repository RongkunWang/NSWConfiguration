// Class to send configuration to a front end using OPCUA server

#ifndef NSWCONFIGURATION_CONFIGSENDER_H_
#define NSWCONFIGURATION_CONFIGSENDER_H_

#include <map>
#include <memory>
#include <string>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/VMMConfig.h"

namespace nsw {
class ConfigSender {
 private:
    /// Map with key: opc client ip, value: OpcClient instance
    std::map<std::string, std::unique_ptr<nsw::OpcClient>> m_clients;
    /* data */

 public:
    ConfigSender();
    ~ConfigSender() {}  // Disconnect from Opc Server(s)?

    /// High level send function
    void sendVmmConfig(nsw::VMMConfig vmm);

    /// Low level send function
    void sendRaw(std::string opcserver_ipport, std::string node, uint8_t *data, size_t data_size);
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_CONFIGSENDER_H_
