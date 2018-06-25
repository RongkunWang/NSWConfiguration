// Opc Client implementation

#ifndef NSWCONFIGURATION_OPCCLIENT_H_
#define NSWCONFIGURATION_OPCCLIENT_H_

#include <unistd.h>
#include <time.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

// From UaoForQuasar
#include "UaoClientForOpcUaSca/include/ClientSessionFactory.h"

// Generated  files
#include "UaoClientForOpcUaSca/include/SpiSlave.h"
#include "UaoClientForOpcUaSca/include/I2cSlave.h"
#include "UaoClientForOpcUaSca/include/DigitalIO.h"
#include "UaoClientForOpcUaSca/include/AnalogInput.h"

// From: open62541-compat, seems not necessary at the moment
// #include "uaplatformlayer.h"

#include "NSWConfiguration/OpcClient.h"

namespace nsw {

class OpcClient {
 private:
    std::string m_server_ipport;

    // TODO(cyildiz): Should we use a unique_ptr/shared_ptr instead of this?
    UaClientSdk::UaSession* m_session;

 public:
    /// Initialize Opc Platform Layer and creates a UaSession
    explicit OpcClient(std::string server_ip_port);
    ~OpcClient() {}

    // vector may not be the best option...
    void writeSpiSlave(std::string node, std::vector<uint8_t> data);
    void writeSpiSlaveRaw(std::string node, uint8_t* data, size_t data_size);

    void writeI2c(std::string node, std::vector<uint8_t> data);
    void writeI2cRaw(std::string node, uint8_t* data, size_t data_size);

    void writeGPIO(std::string node, bool value);
    bool readGPIO(std::string node);

    // Read back the I2c
    std::vector<uint8_t> readI2c(std::string node);

    double readAnalogOutput(std::string node);
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_OPCCLIENT_H_
