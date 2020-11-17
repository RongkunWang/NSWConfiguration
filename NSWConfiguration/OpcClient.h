// Opc Client implementation

#ifndef NSWCONFIGURATION_OPCCLIENT_H_
#define NSWCONFIGURATION_OPCCLIENT_H_

#include <unistd.h>
#include <time.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "ers/ers.h"

// From UaoForQuasar (UaoClientForOpcUaSca/include)
#include "ClientSessionFactory.h"

#include "NSWConfiguration/OpcClient.h"

// Throw this if constructor fails
ERS_DECLARE_ISSUE(nsw,
                  OpcConnectionIssue,
                  "Can't create OpcClient instance for: " << opcserver_ipport
                  << ", error: " << message,
                  ((std::string) opcserver_ipport)
                  ((std::string) message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  OpcReadWriteIssue,
                  "Read or write failed for OpcServer: " << opcserver_ipport
                  << ", node: " << opcnode << ", message: " << message,
                  ((std::string) opcserver_ipport)
                  ((std::string) opcnode)
                  ((std::string) message)  // Describe the problem or forward the message that comes from downstream
                  )

namespace nsw {

class OpcClient {
 private:
    std::string m_server_ipport;

    std::unique_ptr<UaClientSdk::UaSession> m_session;

    UaClientSdk::SessionSecurityInfo m_security;
    UaClientSdk::SessionConnectInfo m_sessionConnectInfo;

 public:
    /// Initialize Opc Platform Layer and creates a UaSession
    explicit OpcClient(const std::string& server_ip_port);
    ~OpcClient();

    OpcClient(const OpcClient&) = delete;

    static constexpr size_t MAX_RETRY  = 5;

    // vector may not be the best option...

    /// Read from Spi Slave. This method will remove the current configuration.
    ///
    /// \param node Node ID in the OPC space
    /// \param number_of_chunks Number of 96 bit chunks to read
    /// \param current_node Current ptree node we are at, required for recursive calls
    /// \return vector of bytes, with size number_of_chunks*12
    std::vector<uint8_t> readSpiSlave(const std::string& node, size_t number_of_chunks) const;


    void writeSpiSlave(const std::string& node, const std::vector<uint8_t>& data) const;
    void writeSpiSlaveRaw(const std::string& node, const uint8_t* data, size_t number_of_bytes) const;

    void writeI2c(const std::string& node, const std::vector<uint8_t>& data) const;
    void writeI2cRaw(const std::string& node, const uint8_t* data, size_t number_of_bytes) const;

    void writeGPIO(const std::string& node, bool value) const;
    bool readGPIO(const std::string& node) const;

    /// Read back the I2c
    std::vector<uint8_t> readI2c(const std::string& node, size_t number_of_bytes = 1) const;

    //! Read current value of an analog output
    float readAnalogInput(const std::string& node) const;

    //! Read n_samples consecutive samples from an analog output.
    std::vector<short unsigned int> readAnalogInputConsecutiveSamples(const std::string& node, size_t n_samples) const;

    // Read SCA ID
    int readScaID(const std::string& node) const; 

    // Read SCA Address
    std::string readScaAddress(const std::string& node) const;

    // Read SCA Online Status
    bool readScaOnline(const std::string& node) const;

    /// Read back ROC
    /// \param node node ID in the OPC space, something such as "SCA Name.gpio.bitBanger"
    /// \param scl scl lines to use
    /// \param sda sda lines to use
    /// \param registerAddress ROC register address as uint8_t (This can be deduced from register name)
    /// \param i2cDelay I2c delay value, 2 corresponds to 100kHz
    /// \return result 8 bit register value
    uint8_t readRocRaw(const std::string& node, unsigned int scl, unsigned int sda, uint8_t registerAddress, unsigned int i2cDelay) const;

    /// Program FPGA
    /// \param bitfile_path relative or absolute path of the binary file that contains the configuration
    void writeXilinxFpga(const std::string& node, const std::string& bitfile_path) const;
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_OPCCLIENT_H_
