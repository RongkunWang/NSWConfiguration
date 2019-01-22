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


// TODO(cyildiz): ConfigSender is not an accurate name for this class, as it can also
// read back from Opc Server. This class is more like a higher level Opc client, we can
// call it NSW
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

    /// Send configuration to all ROC, VMM and TDS that belongs to the FEB
    void sendConfig(const nsw::FEBConfig& feb);

    /// Send configuration to roc in the feb
    void sendRocConfig(const nsw::FEBConfig& feb);

    /// Send configuration to all vmms in the feb
    void sendVmmConfig(const nsw::FEBConfig& feb);

    /// Send configuration to a single vmm in the feb
    void sendVmmConfigSingle(const nsw::FEBConfig& feb, size_t vmm_id);

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

    std::vector<uint8_t> readSpi(std::string opcserver_ipport, std::string node, size_t data_size);

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
    std::vector<uint8_t> readI2c(std::string opcserver_ipport, std::string node, size_t number_of_bytes = 1);

    // Read back I2c register as vector for ADDC
    std::vector<uint8_t> readI2cAtAddress(std::string opcserver_ipport, std::string node,
                                          uint8_t* address, size_t address_size, size_t number_of_bytes = 1);

    /// Read multiple consecutive samples from an analog input
    std::vector<float> readAnalogInputConsecutiveSamples(std::string opcserver_ipport,
                                                         std::string node, size_t n_samples);

    /// Read multiple samples from a channel of a VMM in a frontend
    ///
    /// This function modifies the vmm configuration,
    /// configures vmm, and reads multiple ADC samples
    ///
    /// \param feb The front end config instance
    /// \param vmm_id The vmm number (0-7)
    /// \param channel_id The vmm channel (0-63)
    /// \param n_samples Number of samples to read
    /// \return vector of values that were read
    std::vector<float> readVmmPdoConsecutiveSamples(FEBConfig& feb, size_t vmm_id,
                                                    size_t channel_id, size_t n_samples);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_CONFIGSENDER_H_
