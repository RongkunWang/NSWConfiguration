// Class to send configuration to a front end using OPCUA server

#ifndef NSWCONFIGURATION_CONFIGSENDER_H_
#define NSWCONFIGURATION_CONFIGSENDER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/TPCarrierConfig.h"
#include "NSWConfiguration/TPConfig.h"
#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/PadTriggerSCAConfig.h"
#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/Constants.h"

#include "ic-over-netio/IChandler.h"

ERS_DECLARE_ISSUE(nsw,
                  NSWSenderIssue,
                  message,
                  ((std::string)message)
                  )

namespace nsw {


// TODO(cyildiz): ConfigSender is not an accurate name for this class, as it can also
// read back from Opc Server. This class is more like a higher level Opc client, we can
// call it NSW
class ConfigSender {
 private:
    /// Map with key: opc client ip, value: OpcClient instance
    std::map<std::string, std::unique_ptr<nsw::OpcClient>> m_clients;

    /// Add new client if it connects to a new Opc Server
    void addOpcClientIfNew(const std::string& opcserver_ipport);

 public:
    ConfigSender();
    ~ConfigSender() = default;  // Disconnect from Opc Server(s)?

    /// Send configuration to roc
    void sendRocConfig(const std::string& opc_ip, const std::string& sca_address,
                       const I2cMasterConfig & analog, const I2cMasterConfig & digital);

    /// Send configuration to tds
    void sendTdsConfig(const std::string& opc_ip, const std::string& sca_address,
        const I2cMasterConfig & tds, int ntds, bool reset_tds = false);

    // TODO(rongkun): consider remove this function ?
    /// Send configuration to all ROC, VMM and TDS that belongs to the FEB
    void sendConfig(const nsw::FEBConfig& feb);

    /// Send configuration to roc in the feb
    void sendRocConfig(const nsw::FEBConfig& feb);

    /// Send configuration to all vmms in the feb
    void sendVmmConfig(const nsw::FEBConfig& feb);

    /// Send configuration to a single vmm in the feb
    void sendVmmConfigSingle(const nsw::FEBConfig& feb, size_t vmm_id);

    /// Send configuration to all tds in the feb
    void sendTdsConfig(const nsw::FEBConfig& feb, bool reset_tds = false);

    /**
     * \brief Send data with ICHandler
     * 
     * 
     * \param l1ddc L1DDC config object
     * \param ich IChandler object
     * \param data Data to be sent
     */
    void sendIcConfig(const nsw::L1DDCConfig& l1ddc, IChandler& ich, std::vector<uint8_t>& data);

    /**
     * \brief Read data with ICHandler
     * 
     * 
     * \param l1ddc L1DDC config object
     * \param ich IChandler object
     */
    std::vector<uint8_t> readIcConfig(const nsw::L1DDCConfig& l1ddc, IChandler& ich) const;

    /**
     * \brief Configure the GBTx's of a given L1DDC
     * 
     * \todo Implement GBTx 1 and 2 (Currently only 0 is implemented)
     * 
     * \param l1ddc L1DDC config object
     */
    void sendL1DDCConfig(const nsw::L1DDCConfig& l1ddc);

    /**
     * \brief Sends GBTx configuration for a given L1DDC and GBTx ID
     * 
     * \param l1ddc L1DDC config object
     * \param gbtxId GBTx ID
     */
    void sendGBTxConfig(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId);

    /**
     * \brief Reads GBTx configuration for a given L1DDC and GBTx ID
     * 
     * \param l1ddc L1DDC config object
     * \param gbtxId GBTx ID
     */
    std::vector<uint8_t> readGBTxConfig(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId) const;

    /**
     * \brief Helper function that sends GBTX configuration and reads it back. 
     * 
     * \param ich IC Handler instance
     * \param data Data to be sent
     * \return true if the read-back configuration matches the input
     * \return false if the read-back configuration does not match the input
     */
    bool sendGBTxConfigHelperFunctionReturnTrueIfCorrect(const nsw::L1DDCConfig& l1ddc, IChandler& ich,std::vector<uint8_t>& data);


    /// Send configuration to ADDC and its ARTs
    /// By default (i_art == -1), configure both ARTs
    void sendAddcConfig(const nsw::ADDCConfig& feb, int i_art = -1);

    /// Send configuration to ADDC for aligning ART to TP
    void alignArtGbtxMmtp(const std::map<std::string, nsw::ADDCConfig> & addcs_map,
                          const std::map<std::string, nsw::TPConfig>   & tps_map);
    void alignArtGbtxMmtp(const std::vector<nsw::ADDCConfig> & addcs, const nsw::TPConfig& tp);

    /// High level send function
    uint32_t readPadTriggerSCAControlRegister(const nsw::PadTriggerSCAConfig& obj);
    void sendPadTriggerSCAControlRegister(const nsw::PadTriggerSCAConfig& obj, bool write = true);
    void sendPadTriggerSCAConfig(const nsw::PadTriggerSCAConfig& obj);

    /// High level send function, and the kids
    void sendRouterConfig(const nsw::RouterConfig& obj);
    void sendRouterWaitGPIO(const nsw::RouterConfig& obj);
    bool sendRouterCheckGPIO(const nsw::RouterConfig& obj);
    void sendRouterSetSCAID(const nsw::RouterConfig& obj);
    void sendRouterSoftReset(const nsw::RouterConfig& obj,
                             std::chrono::seconds reset_hold  = nsw::router::RESET_HOLD,
                             std::chrono::seconds reset_sleep = nsw::router::RESET_SLEEP);

    /// High level send/read functions (SCAX, TP, TPCarrier)
    void sendTPConfig(const nsw::TPConfig& tp, bool quiet = false);
    void sendTPCarrierConfig(const nsw::TPCarrierConfig& carrier, const bool quiet = false);
    void sendSCAXRegister                (const nsw::SCAConfig& scax, uint8_t address, uint32_t message, bool quiet = false);
    uint32_t readSCAXRegisterWord        (const nsw::SCAConfig& scax, uint8_t address);
    std::vector<uint8_t> readSCAXRegister(const nsw::SCAConfig& scax, uint8_t address);

    /// High level send/read functions (pad trigger)
    void sendPadTriggerConfigRegister(const nsw::PadTriggerSCAConfig& pt, uint8_t address, uint32_t message, bool quiet = false);
    uint32_t readPadTriggerConfigRegister(const nsw::PadTriggerSCAConfig& pt, uint8_t address);

    /// High level send function to send configuration to all addresses under an I2cMaster
    void sendI2cMasterConfig(const std::string& opcserver_ipport, const std::string& topnode,
        const nsw::I2cMasterConfig& cfg);

    /// Write only one of the register addresses in I2cMaster
    void sendI2cMasterSingle(const std::string& opcserver_ipport, const std::string& topnode,
        const nsw::I2cMasterConfig& cfg, const std::string& reg_address);

    /// Read back ROC
    /// \param opcserver_ipport OPCServer IP and port
    /// \param node node ID in the OPC space, something such as "SCA Name.gpio.bitBanger"
    /// \param sclLine scl lines to use
    /// \param sdaLine sda lines to use
    /// \param registerAddress ROC register address as uint8_t (This can be deduced from register name)
    /// \param delay I2c delay value, 2 corresponds to 100kHz
    /// \return result 8 bit register value
    uint8_t readBackRoc(const std::string& opcserver_ipport, const std::string& node,
        unsigned int sclLine, unsigned int sdaLine, uint8_t registerAddress, unsigned int delay);

    /// Wrapper to read back from the digital ROC section
    /// DO NOT CALL IT ON ANYTHING BUT MMFE8
    [[nodiscard]]
    uint8_t readBackRocDigital(const std::string& opcserver_ipport, const std::string& node, uint8_t registerAddress);

    /// Wrapper to read back from the analog ROC section
    /// DO NOT CALL IT ON ANYTHING BUT MMFE8
    [[nodiscard]]
    uint8_t readBackRocAnalog(const std::string& opcserver_ipport, const std::string& node, uint8_t registerAddress);

    /// Low level Spi send function
    void sendSpiRaw(const std::string& opcserver_ipport, const std::string& node, uint8_t *data, size_t data_size);

    std::vector<uint8_t> readSpi(const std::string& opcserver_ipport, const std::string& node, size_t data_size);

    /// Low level Spi send function
    void sendSpi(const std::string& opcserver_ipport, const std::string& node, const std::vector<uint8_t>& vdata);

    /// Low level I2c send function
    void sendI2c(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata);

    /// Low level I2c send function
    void sendI2cRaw(const std::string opcserver_ipport, const std::string node, uint8_t *data, size_t data_size);

    /// Low level GPIO send function
    void sendGPIO(const std::string& opcserver_ipport, const std::string& node, bool data);

    /// Read  back GPIO register
    bool readGPIO(const std::string& opcserver_ipport, const std::string& node);

    // Read back I2c register as vector
    std::vector<uint8_t> readI2c(const std::string& opcserver_ipport, const std::string& node,
        size_t number_of_bytes = 1);

    // Read back I2c register as vector for ADDC
    std::vector<uint8_t> readI2cAtAddress(const std::string& opcserver_ipport, const std::string& node,
                                          uint8_t* address, size_t address_size, size_t number_of_bytes = 1);

    // Send I2c register as vector for ADDC
    void sendI2cAtAddress(const std::string& opcserver_ipport, const std::string& node,
                          const std::vector<uint8_t>& address, std::vector<uint8_t> data);

    /// Read multiple consecutive samples from an analog input
    std::vector<short unsigned int> readAnalogInputConsecutiveSamples(const std::string& opcserver_ipport,
                                                         const std::string& node, size_t n_samples);

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
    std::vector<short unsigned int> readVmmPdoConsecutiveSamples(FEBConfig& feb,
                                                                 size_t vmm_id,
                                                                 size_t n_samples);

    // Read SCA ID
    std::uint32_t readSCAID(FEBConfig& feb);

    // Read SCA Address
    std::string readSCAAddress(FEBConfig& feb);

    // Read SCA Online Status
    bool readSCAOnline(FEBConfig& feb);

    /// Program FPGA from bitfile
    /// \param bitfile_path relative or absolute path of the binary file that contains the configuration
    void sendFPGA(const std::string& opcserver_ipport, const std::string& node, const std::string& bitfile_path);

    /// Set vmm enable register to 0
    /// \param feb Config needed for OPC IP
    void disableVmmCaptureInputs(const nsw::FEBConfig& feb);

    /// Set vmm enable register to value in passed config
    /// \param feb Reference config
    void enableVmmCaptureInputs(const nsw::FEBConfig& feb);

    // Read anytype SCA OPC UA's FreeVariable
    // example use: ReadFreeVariable<bool>(...)
    template <typename T>
    inline T readFreeVariable(const std::string& opcserver_ipport, const std::string& node) {
    
        addOpcClientIfNew(opcserver_ipport);
        return m_clients.at(opcserver_ipport)->readFreeVariable<T>(node);
    
    }

    // Write anytype SCA OPC UA's FreeVariable
    template<typename T>
    inline void writeFreeVariable(const std::string& opcserver_ipport, const std::string& node, T value) {

        addOpcClientIfNew(opcserver_ipport);
        m_clients[opcserver_ipport]->writeFreeVariable(node, value);

    }    

    // Program to set VMMConfigurationStatusInfo FreeVariable parameter
    // Used by SCA DCS for VMM boards (polyneikis)
    // \param  feb Config needed for SCA OPC board
    // \param  vmm Config needed for SCA OPC board
    // 1 : VMM Temperature mode
    // 0 : Baseline mode (No VMM temperature output)
    void setVMMConfigurationStatusInfoDCS(const nsw::FEBConfig& feb, const nsw::VMMConfig& vmm);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_CONFIGSENDER_H_
