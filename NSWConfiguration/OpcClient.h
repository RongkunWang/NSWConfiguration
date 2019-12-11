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
#include "UaoClientForOpcUaSca/include/IoBatch.h"

// From: open62541-compat, seems not necessary at the moment
// #include "uaplatformlayer.h"

#include "NSWConfiguration/OpcClient.h"

namespace nsw {

class OpcClient {
 private:
    std::string m_server_ipport;

    std::unique_ptr<UaClientSdk::UaSession> m_session;

    UaClientSdk::SessionSecurityInfo m_security;
    UaClientSdk::SessionConnectInfo m_sessionConnectInfo;

 public:
    /// Initialize Opc Platform Layer and creates a UaSession
    explicit OpcClient(std::string server_ip_port);
    ~OpcClient();

    OpcClient(const OpcClient&) = delete;

    /// Retry when communication fails
    bool   SUCCESS          = 0;
    size_t THIS_RETRY       = 0;
    const size_t MAX_RETRY  = 5;

    // vector may not be the best option...

    /// Read from Spi Slave. This method will remove the current configuration.
    ///
    /// \param node Node ID in the OPC space
    /// \param number_of_chunks Number of 96 bit chunks to read
    /// \param current_node Current ptree node we are at, required for recursive calls
    /// \return vector of bytes, with size number_of_chunks*12
    std::vector<uint8_t> readSpiSlave(std::string node, size_t number_of_chunks);


    void writeSpiSlave(std::string node, std::vector<uint8_t> data);
    void writeSpiSlaveRaw(std::string node, uint8_t* data, size_t number_of_bytes);

    void writeI2c(std::string node, std::vector<uint8_t> data);
    void writeI2cRaw(std::string node, uint8_t* data, size_t number_of_bytes);

    void writeGPIO(std::string node, bool value);
    bool readGPIO(std::string node);

    /// Read back the I2c
    std::vector<uint8_t> readI2c(std::string node, size_t number_of_bytes = 1);

    //! Read current value of an analog output
    float readAnalogInput(std::string node);

    //! Read n_samples consecutive samples from an analog output.
    std::vector<short unsigned int> readAnalogInputConsecutiveSamples(std::string node, size_t n_samples);

    // Read back ROC. Easy busy...
         uint8_t readRocRaw(std::string node, unsigned int scl, unsigned int sda, uint8_t registerAddress, unsigned int i2cDelay);
    
        
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_OPCCLIENT_H_
