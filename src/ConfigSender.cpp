#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>

#include <ers/ers.h>

#include "NSWConfiguration/Constants.h"
#include "ic-handler/IChandler.h"

#include "NSWConfiguration/ConfigSender.h"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

#include <ers/ers.h>

#include <thread>
#include "boost/property_tree/ptree.hpp"
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;

nsw::ConfigSender::ConfigSender() {
}

void nsw::ConfigSender::addOpcClientIfNew(const std::string& opcserver_ipport) {
    // std::map doesn't allow duplicates anyway, consider removing this check?
    if (m_clients.find(opcserver_ipport) == m_clients.end()) {
        m_clients.emplace(opcserver_ipport, std::make_unique<nsw::OpcClient>(opcserver_ipport));
    }
}

void nsw::ConfigSender::sendSpiRaw(const std::string& opcserver_ipport, const std::string& node, const uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, data, data_size);
}

std::vector<uint8_t> nsw::ConfigSender::readSpi(const std::string& opcserver_ipport, const std::string& node, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readSpiSlave(node, data_size);
}

void nsw::ConfigSender::sendSpi(const std::string& opcserver_ipport, const std::string& node, const std::vector<uint8_t>& vdata) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, vdata.data(), vdata.size());
}

uint8_t nsw::ConfigSender::readBackRoc(const std::string& opcserver_ipport, const std::string& node,
    unsigned int sclLine, unsigned int sdaLine, uint8_t registerAddress, unsigned int delay ) {
  addOpcClientIfNew(opcserver_ipport);
  return m_clients[opcserver_ipport]->readRocRaw(node, sclLine, sdaLine, registerAddress, delay);
}

uint8_t nsw::ConfigSender::readBackRocDigital(const std::string& opcserver_ipport, const std::string& node, uint8_t registerAddress) {
    // FIXME: Why do we have to call it twice?
    const unsigned int delay{2};
    const auto fullNode = node + ".gpio.bitBanger";
    readBackRoc(opcserver_ipport, fullNode, nsw::roc::mmfe8::digital::SCL_LINE_PIN, nsw::roc::mmfe8::digital::SDA_LINE_PIN, registerAddress, delay);
    return readBackRoc(opcserver_ipport, fullNode, nsw::roc::mmfe8::digital::SCL_LINE_PIN, nsw::roc::mmfe8::digital::SDA_LINE_PIN, registerAddress, delay);
}

uint8_t nsw::ConfigSender::readBackRocAnalog(const std::string& opcserver_ipport, const std::string& node, uint8_t registerAddress) {
    // FIXME: Why do we have to call it twice?
    const unsigned int delay{2};
    const auto fullNode = node + ".gpio.bitBanger";
    readBackRoc(opcserver_ipport, fullNode, nsw::roc::mmfe8::analog::SCL_LINE_PIN, nsw::roc::mmfe8::analog::SDA_LINE_PIN, registerAddress, delay);
    return readBackRoc(opcserver_ipport, fullNode, nsw::roc::mmfe8::analog::SCL_LINE_PIN, nsw::roc::mmfe8::analog::SDA_LINE_PIN, registerAddress, delay);
}

void nsw::ConfigSender::sendI2cRaw(const std::string opcserver_ipport, const std::string node, const uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeI2cRaw(node, data, data_size);
}

void nsw::ConfigSender::sendI2c(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeI2cRaw(node, vdata.data(), vdata.size());
}

void nsw::ConfigSender::sendGPIO(const std::string& opcserver_ipport, const std::string& node, bool data) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeGPIO(node, data);
}

bool nsw::ConfigSender::readGPIO(const std::string& opcserver_ipport, const std::string& node) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readGPIO(node);
}

std::vector<uint8_t> nsw::ConfigSender::readI2c(const std::string& opcserver_ipport,
    const std::string& node, size_t number_of_bytes) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readI2c(node, number_of_bytes);
}

std::vector<uint8_t> nsw::ConfigSender::readI2cAtAddress(const std::string& opcserver_ipport,
    const std::string& node, const uint8_t* address, size_t address_size, size_t number_of_bytes) {
    // Write only the address without data
    nsw::ConfigSender::sendI2cRaw(opcserver_ipport, node, address, address_size);

    // Read back data into the vector readdata
    std::vector<uint8_t> readdata = nsw::ConfigSender::readI2c(opcserver_ipport, node, number_of_bytes);
    return readdata;
}

void nsw::ConfigSender::sendI2cAtAddress(const std::string& opcserver_ipport,
                                         const std::string& node,
                                         const std::vector<uint8_t>& address,
                                         std::vector<uint8_t> data) {
    // Insert the address in the beginning of data vector
    for (auto & address_byte : address) {
        // data.push_front(address_byte);
        data.insert(data.begin(), address_byte);
    }
    nsw::ConfigSender::sendI2cRaw(opcserver_ipport, node, data.data(), data.size());
}

void nsw::ConfigSender::sendI2cMasterSingle(const std::string& opcserver_ipport, const std::string& topnode,
                                            const nsw::I2cMasterConfig& cfg, const std::string& reg_address) {
    ERS_LOG("Sending I2c configuration to " << topnode << "." + cfg.getName() + "." << reg_address);
    auto addr_bitstr = cfg.getBitstreamMap();
    auto address = topnode + "." + cfg.getName() + "." + reg_address;  // Full I2C address
    auto bitstr = addr_bitstr[reg_address];
    auto data = nsw::stringToByteVector(bitstr);
    for (auto d : data) {
        ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
    }
        sendI2cRaw(opcserver_ipport, address, data.data(), data.size());
}

void nsw::ConfigSender::sendI2cMasterConfig(const std::string& opcserver_ipport,
    const std::string& topnode, const nsw::I2cMasterConfig& cfg) {
    ERS_LOG("Sending I2c configuration to " << topnode << "." << cfg.getName());
    auto addr_bitstr = cfg.getBitstreamMap();
    for (auto ab : addr_bitstr) {
        auto address = topnode + "." + cfg.getName() + "." + ab.first;  // Full I2C address
        auto bitstr = ab.second;
        auto data = nsw::stringToByteVector(bitstr);
        for (auto d : data) {
            ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
        }
        sendI2cRaw(opcserver_ipport, address, data.data(), data.size());
    }
}

// TODO(rongkun): consider remove this function ?
void nsw::ConfigSender::sendConfig(const nsw::FEBConfig& feb) {
    sendRocConfig(feb);
    sendTdsConfig(feb);
    sendVmmConfig(feb);
}

void nsw::ConfigSender::sendRocConfig(const nsw::FEBConfig& feb) {
    auto opc_ip = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();

    sendRocConfig(opc_ip, feb_address, feb.getRocAnalog(), feb.getRocDigital());
}

void nsw::ConfigSender::sendVmmConfig(const nsw::FEBConfig& feb) {
    // Set Vmm Configuration Enable
    std::vector<uint8_t> data = {0xff};
    auto opc_ip = feb.getOpcServerIp();

    // TODO(cyildiz): Make new methods: EnableVmmAcquisition() - DisableVmmAcquisition()

    // Set Vmm Acquisition Disable
    auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);

    for (auto vmm : feb.getVmms()) {
        setVMMConfigurationStatusInfoDCS(feb, vmm);
        const auto data = vmm.getByteVector();
        std::vector<uint8_t> dat;
        for (int i = 0; i < 216; i++) {
          dat.push_back(0x84);
        }
        ERS_DEBUG(4, "vmm size(bytes) : " << dat.size());
        ERS_LOG("Sending configuration to " << feb.getAddress() << ".spi." << vmm.getName());
        sendSpiRaw(opc_ip, feb.getAddress() + ".spi." + vmm.getName() , data.data(), data.size());
        ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));
    }

    // Set Vmm Acquisition Enable
    data = {0x0};
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);
}

void nsw::ConfigSender::sendVmmConfigSingle(const nsw::FEBConfig& feb, const size_t vmm_id) {
    // Set Vmm Configuration Enable
    // std::vector<uint8_t> data = {0xff};
    const auto opc_ip = feb.getOpcServerIp();
    const auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  {0xff});

    const auto& vmm = feb.getVmm(vmm_id);
    // FIXME TODO should this modify the config, or a copy of the config?
    setVMMConfigurationStatusInfoDCS(feb, vmm);
    const auto vmmdata = vmm.getByteVector();
    ERS_DEBUG(1, "Sending I2c configuration to " << feb.getAddress() << ".spi." << vmm.getName());
    sendSpiRaw(opc_ip, feb.getAddress() + ".spi." + vmm.getName() , vmmdata.data(), vmmdata.size());
    ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));

    // Set Vmm Acquisition Enable
    // data = {0x0};
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  {0x0});
}

void nsw::ConfigSender::sendIcConfigGBTx(const nsw::L1DDCConfig& l1ddc, ic::fct::IChandler& ich, const std::vector<uint8_t>& data){
    // Send data with IChandler
    // Raise intelligible error if IChandler crashes
    try {
        ich.sendCfg(data);
    }
    catch(...){
        nsw::NSWSenderIssue issue(ERS_HERE, fmt::format("Problem sending GBTx configuration over IC for {}. This may be due to a unpowered L1DDC, or a missing felix server",l1ddc.getName()));
        ers::error(issue);
        throw issue;
    }
}

std::vector<uint8_t> nsw::ConfigSender::readIcConfigGBTx(const nsw::L1DDCConfig& l1ddc, ic::fct::IChandler& ich) const {
    // Read data with IChandler
    // Raise intelligible error if IChandler crashes
    try {
        return ich.readCfg();
    }
    catch(...){
        nsw::NSWSenderIssue issue(ERS_HERE, fmt::format("Problem reading GBTx configuration over IC for {}. This may be due to a unpowered L1DDC, or a missing felix server",l1ddc.getName()));
        ers::error(issue);
        throw issue;
    }
}


bool nsw::ConfigSender::sendGBTxIcConfigHelperFunction(const nsw::L1DDCConfig& l1ddc, ic::fct::IChandler& ich, const std::vector<uint8_t>& data){
    // Upload configuration to GBTx using IC channel, read back and check config
    // return 1 if config read back correctly

    ERS_DEBUG(2, fmt::format("\n\nConfiguration to be uploaded to IC GBTx on {}:{}",l1ddc.getName(),nsw::getPrintableGbtxConfig(data)));

    sendIcConfigGBTx(l1ddc,ich,data);

    std::vector<uint8_t> currentConfig = readIcConfigGBTx(l1ddc,ich);

    ERS_DEBUG(2, fmt::format("\n\nConfiguration READ from IC GBTx on {}:{}",l1ddc.getName(),nsw::getPrintableGbtxConfig(currentConfig)));

    // Check readback
    if (data.size()!=currentConfig.size()) {
        ERS_LOG("WARNING: readback size not expected");
        ERS_LOG("expected="<<data.size()<<" recieved="<<currentConfig.size()<<'\n');
        return false;
    }

    // Some of the readback registers are not writable registers. Only check the writable registers
    constexpr std::size_t maxConfigRegister = 365;

    // Check that the written config matches the readback config
    for (std::size_t i=0; i<data.size(); i++){
        if (i>maxConfigRegister) {
            break;
        }
        if (data.at(i)!=currentConfig.at(i)) {
            ERS_LOG("Unexpected bit read back during GBTx configuration");
            return false;
        }
    }
    ERS_DEBUG(2, "==> Configuration readback is OKAY");
    return true;
}

void nsw::ConfigSender::sendI2cConfigGBTx(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId, const std::vector<uint8_t>& data){
    // Send GBTx configuration over I2C
    const std::string opcServerIp  = l1ddc.getOpcServerIp();
    const std::string opcNodeId    = l1ddc.getOpcNodeId(); 
    if (gbtxId!=1 && gbtxId!=2){
        nsw::NSWSenderIssue issue(ERS_HERE, fmt::format("Attempt to configure GBTx{} using I2C for board {}. This is probably a mistake, since only GBTx2 and GBTx3 are expected to be configured with I2C",gbtxId+1,l1ddc.getName()));
        ers::error(issue);
        throw issue;
    }
    const std::string opcNodeIdFull = (gbtxId==1)?opcNodeId+".gbtx2.gbtx2":opcNodeId+".gbtx3.gbtx3";
    std::size_t chunkLen = l1ddc.i2cBlockSize();
    std::vector<uint8_t> frame;
    for (std::size_t reg=0; reg<data.size(); reg++) {
        if (frame.size() == 0) {
            frame.push_back(static_cast<std::uint8_t>(reg & 0xff));
            frame.push_back(static_cast<std::uint8_t>(reg >> 8));
        }
        frame.push_back(data.at(reg));
        if (frame.size() == chunkLen || reg+1 == data.size()) {
            std::stringstream ss;
            ss<<"\n==> SENDING: ";
            for (const auto & val : frame) ss<<fmt::format(" {:02x}",val);
            ERS_DEBUG(5, ss.str());
            sendI2c(opcServerIp, opcNodeIdFull, frame);
            nsw::snooze(std::chrono::microseconds{l1ddc.i2cDelay()});
            frame.clear();
        }
    }
}

std::vector<uint8_t> nsw::ConfigSender::readI2cConfigGBTx(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId) {
    // Read GBTx configuration over I2C
    const std::string opcServerIp  = l1ddc.getOpcServerIp();
    const std::string opcNodeId    = l1ddc.getOpcNodeId(); 
    if (gbtxId!=1 && gbtxId!=2){
        nsw::NSWSenderIssue issue(ERS_HERE, fmt::format("Attempt to configure GBTx{} using I2C for board {}. This is probably a mistake, since only GBTx2 and GBTx3 are expected to be configured with I2C",gbtxId+1,l1ddc.getName()));
        ers::error(issue);
        throw issue;
    }
    const std::string opcNodeIdFull = (gbtxId==1)?opcNodeId+".gbtx2.gbtx2":opcNodeId+".gbtx3.gbtx3";
    std::vector<uint8_t> data;
    const std::size_t chunkLen = l1ddc.i2cBlockSize();
    const std::size_t nBytesTotal = nsw::NUM_GBTX_WRITABLE_REGISTERS;
    const std::size_t nChunks = (nBytesTotal+chunkLen)/chunkLen;
    for (std::size_t reg=0; reg<nChunks*chunkLen; reg+=chunkLen){
        // Set address to read
        const std::array<std::uint8_t, 2> address{static_cast<std::uint8_t>(reg & 0xff),
                                                  static_cast<std::uint8_t>(reg >> 8)};
        nsw::ConfigSender::sendI2cRaw(opcServerIp, opcNodeIdFull, address.data(), address.size());
        nsw::snooze(std::chrono::microseconds{l1ddc.i2cDelay()});
        // Read back required number of bytes
        const std::size_t nBytesRead = (reg+chunkLen>nBytesTotal)?nBytesTotal-reg:chunkLen;
        const std::vector<uint8_t> readback = nsw::ConfigSender::readI2c(opcServerIp, opcNodeIdFull, nBytesRead);
        nsw::snooze(std::chrono::microseconds{l1ddc.i2cDelay()});
        std::stringstream ss;
        ss<<"\n==> RECEIVED: ";
        for (const auto & val : readback) ss<<fmt::format(" {:02x}",val);
        ERS_DEBUG(5, ss.str());
        data.insert(data.end(),readback.begin(),readback.end());
    }
    return data;
}

bool nsw::ConfigSender::sendGBTxI2cConfigHelperFunction(const nsw::L1DDCConfig& l1ddc, const std::size_t gbtxId, const std::vector<uint8_t>& data){
    // Upload configuration to GBTx using I2C, read back and confirm config
    // return 1 if config read back correctly
    ERS_DEBUG(2, fmt::format("\n\nConfiguration to be uploaded to I2C GBTx on {}:{}",l1ddc.getName(),nsw::getPrintableGbtxConfig(data)));

    // Upload configuration
    ERS_DEBUG(2, "\n==> Uploading configuration to GBTx");
    sendI2cConfigGBTx(l1ddc,gbtxId,data);
    ERS_DEBUG(2, "\n==> Done uploading configuration to GBTx");

    // Read back configuration
    ERS_DEBUG(2, "\n==> Reading back configuration from GBTx");
    const std::vector<uint8_t> currentConfig = readI2cConfigGBTx(l1ddc,gbtxId);
    ERS_DEBUG(2, fmt::format("\n\nConfiguration READ from IC GBTx on {}:{}",l1ddc.getName(),nsw::getPrintableGbtxConfig(currentConfig)));

    // Check readback
    if (data.size()!=currentConfig.size()) {
        ERS_LOG("WARNING: I2C readback size not expected");
        ERS_LOG("expected="<<data.size()<<" recieved="<<currentConfig.size()<<'\n');
        return false;
    }

    // Some of the readback registers are not writable registers. Only check the writable registers
    constexpr std::size_t maxConfigRegister = 365;

    // Check that the written config matches the readback config
    for (std::size_t i=0; i<data.size(); i++){
        if (i>maxConfigRegister) {
            break;
        }
        if (data.at(i)!=currentConfig.at(i)) {
            ERS_LOG("Unexpected bit read back via IC during GBTx configuration");                                                                                                                                                 
            ERS_DEBUG(2, "==> Configuration readback is BAD");
            return false;
        }
    }
    ERS_DEBUG(2, "==> Configuration readback is OKAY");

    return true;
}

void nsw::ConfigSender::sendGBTxConfig(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId, ic::fct::IChandler& ich){
    // Send configuration for one GBTx
    // L1DDCConfig should be initialized with a configuration ptree
    // gbtxId should be 0, 1, 2 depending on which GBTx is to be configured (TODO: 1, 2 not supported)
    ERS_LOG("\nConfigSender::sendGBTxConfig number="<<gbtxId<<" for "<<l1ddc.getName()<<"\n");
    const std::vector<uint8_t> data = l1ddc.getGBTxBytestream(gbtxId);
    // generate bytestream
    if (gbtxId==0){

        // Try sending configuration and check the readback
        // If the readback doesn't match, for nTries, raise error
        std::size_t nTries = MAX_ATTEMPTS;
        while (!sendGBTxIcConfigHelperFunction(l1ddc,ich,data) && nTries>0) {
            ERS_LOG("Retrying configuration. Remaining tries: "<<nTries<<" for "<<l1ddc.getName());
            nTries--;
        }
        if (nTries==0) {
            nsw::NSWSenderIssue issue(ERS_HERE, "Unable to read back correct configuration within " + std::to_string(MAX_ATTEMPTS) + " tries for "+l1ddc.getName());
            ers::error(issue);
            throw issue;
        }
    }
    else if (gbtxId==1 || gbtxId==2){
        // Try sending configuration and check the readback
        // If the readback doesn't match, for nTries, raise error
        std::size_t nTries = MAX_ATTEMPTS;
        while (!sendGBTxI2cConfigHelperFunction(l1ddc,gbtxId,data) && nTries>0) {
            ERS_LOG("Retrying I2C configuration. Remaining tries: "<<nTries<<" for "<<l1ddc.getName());
            nTries--;
        }
        if (nTries==0) {
            nsw::NSWSenderIssue issue(ERS_HERE, "Unable to read back correct configuration within " + std::to_string(MAX_ATTEMPTS) + " tries for "+l1ddc.getName());
            ers::error(issue);
            throw issue;
        }
    }
    else{
        nsw::NSWSenderIssue issue(ERS_HERE, "Invalid GBTx ID while sending GBTx Configuration");
        ers::error(issue);
        throw issue;
    }
}

std::vector<uint8_t> nsw::ConfigSender::readGBTxConfig(const nsw::L1DDCConfig& l1ddc, std::size_t gbtxId, ic::fct::IChandler& ich){
    // read back gbtx configuration
    ERS_LOG(fmt::format("Reading bytestream for {} on GBTx{}",l1ddc.getName(),gbtxId));
    if (gbtxId==0){
        return readIcConfigGBTx(l1ddc,ich);
    }
    else if (gbtxId==1){
        return readI2cConfigGBTx(l1ddc,1);
    }
    else if (gbtxId==2){
        return readI2cConfigGBTx(l1ddc,2);
    }
    else{
        nsw::NSWSenderIssue issue(ERS_HERE, "Invalid GBTx ID while sending GBTx Configuration");
        ers::error(issue);
        throw issue;
    }
}

void nsw::ConfigSender::sendL1DDCConfig(const nsw::L1DDCConfig& l1ddc) {
    // Send configuration for l1ddc
    // This should configure the GBTx's and SCA
    // Currently, configure GBTx0
    ERS_INFO("Configuring "<<l1ddc.getName());

    boost::property_tree::ptree phaseTree; // Store phases for each GBTx
    const std::time_t now = std::time(nullptr);
    phaseTree.push_front(ptree::value_type("created", std::to_string(now)));
    phaseTree.push_front(ptree::value_type("nameL1DDC", l1ddc.getName()));
    phaseTree.push_front(ptree::value_type("nodeL1DDC", l1ddc.getNodeName()));

    // make common IC handler for GBTx0
    const std::uint64_t fid_toflx  = l1ddc.getFidToFlx();
    const std::uint64_t fid_tohost = l1ddc.getFidToHost();
    ic::fct::IChandler ich(fid_toflx,fid_tohost);

    std::vector<std::size_t> GBTxToConfigure;
    if (l1ddc.getConfigureGBTx(0)) GBTxToConfigure.push_back(0);
    if (l1ddc.getConfigureGBTx(1)) GBTxToConfigure.push_back(1);
    if (l1ddc.getConfigureGBTx(2)) GBTxToConfigure.push_back(2);
    for (std::size_t gbtxId : GBTxToConfigure){
        ERS_LOG(fmt::format("\nConfiguring GBTx number {}",gbtxId));

        // Check the initial configuration
        std::vector<uint8_t> currentConfig;
        if (gbtxId==0){
            currentConfig = readIcConfigGBTx(l1ddc,ich);
        }
        else{
            currentConfig = readI2cConfigGBTx(l1ddc,gbtxId);
        }
        ERS_DEBUG(2, fmt::format("\n\nConfiguration READ from GBTx{} on {}, BEFORE uploading new configuration:{}",gbtxId,l1ddc.getName(),nsw::getPrintableGbtxConfig(currentConfig)));

        // Initial configuration with default values
        sendGBTxConfig(l1ddc,gbtxId,ich);
        if (l1ddc.trainGBTxPhaseAlignment()){
            ERS_LOG("\nTraining GBTx phase alignment for "<<l1ddc.getName());
            // Make a non-const copy
            L1DDCConfig l1ddcCopy(l1ddc);
            // send start training configuration
            l1ddcCopy.trainGbtxsOn();
            sendGBTxConfig(l1ddcCopy,gbtxId,ich);
            // wait while registers train
            ERS_LOG("\nGBTx phase training time: "<<l1ddc.trainGBTxPhaseWaitTime()<<"us for "<<l1ddc.getName());
            ERS_LOG("Path = "<<l1ddc.getGBTxPhaseOutputDBPath());
            nsw::snooze(std::chrono::microseconds{l1ddc.trainGBTxPhaseWaitTime()});
            // send stop training configuration
            l1ddcCopy.trainGbtxsOff();
            sendGBTxConfig(l1ddcCopy,gbtxId,ich);

            // Print out phases
            const std::vector<uint8_t> config = readGBTxConfig(l1ddcCopy,gbtxId,ich);
            ERS_LOG(fmt::format("Phase table for {} GBTx{}",l1ddc.getName(),gbtxId));
            const boost::property_tree::ptree phases = nsw::GBTxConfig::getPhasesTree(config);
            phaseTree.put_child(fmt::format("GBTx{}",gbtxId),phases);
        }
    }

    // Save phases to output file
    if (l1ddc.getGBTxPhaseOutputDBPath()!=""){
        std::string outputFileName = fmt::format("{}/{}.json",l1ddc.getGBTxPhaseOutputDBPath(),l1ddc.getNodeName());
        std::ofstream oFile;
        oFile.open(outputFileName);
        boost::property_tree::json_parser::write_json(oFile, phaseTree);
        ERS_LOG("Wrote GBTx phase output to "<<outputFileName);
    }

    ERS_LOG("Finished config for "<<l1ddc.getName());
}

void nsw::ConfigSender::sendTdsConfig(const nsw::FEBConfig& feb, bool reset_tds) {
  // this is used for outside
    auto opc_ip = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();
    // HACK!
    int ntds = feb.getTdss().size();
    for (auto tds : feb.getTdss()) {
        sendTdsConfig(opc_ip, feb_address, tds, ntds, reset_tds);
    }

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendRocConfig(const std::string& opc_ip, const std::string& sca_address,
    const I2cMasterConfig & analog, const I2cMasterConfig & digital) {
    // 1. Reset all logics
    sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 0);
    sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 0);
    sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 0);

    sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 1);

    sendI2cMasterConfig(opc_ip, sca_address, analog);

    sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 1);

    ERS_DEBUG(2, "Waiting for ROC Pll locks...");
    bool roc_locked = 0;
    while (!roc_locked) {
        bool rPll1 = readGPIO(opc_ip, sca_address + ".gpio.rocPllLocked");
        bool rPll2 = readGPIO(opc_ip, sca_address + ".gpio.rocPllRocLocked");
        roc_locked = rPll1 & rPll2;
        ERS_DEBUG(2, "rocPllLocked: " << rPll1 << ", rocPllRocLocked: " << rPll2);
    }

    sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 1);

    sendI2cMasterConfig(opc_ip, sca_address, digital);
}

// TODO(rongkun): consider remove this function ?
// void nsw::ConfigSender::sendTdsConfig(const nsw::FEBConfig& tds, bool reset_tds) {
  // // unused yet
    // auto opc_ip = tds.getOpcServerIp();
    // auto tds_address = tds.getAddress();

    // sendGPIO(opc_ip, tds_address + ".gpio.tdsReset", 1);

    // sendI2cMasterConfig(opc_ip, tds_address, tds.i2c);


// }

void nsw::ConfigSender::sendTdsConfig(const std::string& opc_ip, const std::string& sca_address,
    const I2cMasterConfig & tds, int ntds, bool reset_tds) {
  // internal call
  // sca_address is feb.getAddress()
  if (ntds < 3) {
    // old boards, and PFEB
      sendGPIO(opc_ip, sca_address + ".gpio.tdsReset", 1);
  } else {
    // new boards
    if (tds.getName() == "tds0")
      sendGPIO(opc_ip, sca_address + ".gpio.tdsaReset", 1);
    else if (tds.getName() == "tds1")
      sendGPIO(opc_ip, sca_address + ".gpio.tdsbReset", 1);
    else if (tds.getName() == "tds2")
      sendGPIO(opc_ip, sca_address + ".gpio.tdscReset", 1);
    else if (tds.getName() == "tds3")
      sendGPIO(opc_ip, sca_address + ".gpio.tdsdReset", 1);
  }


    sendI2cMasterConfig(opc_ip, sca_address, tds);
    if (reset_tds) {
      // copy out the configuration, etc
      I2cMasterConfig tdss(tds);
      ptree config = tdss.getConfig();
      // TDS resets

      // ePLL
      // config.put("register12.resets", 0x20);
      // tdss.buildConfig(config);

      // Debug
      // unsigned int reset_byte = config.get<unsigned int>("register12.resets");
      // std::cout << "=======" << std::endl;
      // std::cout  << "tds reset byte: " << reset_byte << std::endl;
      // std::cout << "=======" << std::endl;

      tdss.setRegisterValue("register12", "resets", 0x20);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      tdss.setRegisterValue("register12", "resets", 0x0);
      // config.put("register12.resets", 0x0);
      // tdss.buildConfig(config);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      // logic
      tdss.setRegisterValue("register12", "resets", 0x06);
      // config.put("register12.resets", 0x06);
      // tdss.buildConfig(config);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      tdss.setRegisterValue("register12", "resets", 0x0);
      // config.put("register12.resets", 0x0);
      // tdss.buildConfig(config);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      // SER
      tdss.setRegisterValue("register12", "resets", 0x14);
      // config.put("register12.resets", 0x14);
      // tdss.buildConfig(config);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      tdss.setRegisterValue("register12", "resets", 0x0);
      // config.put("register12.resets", 0x0);
      // tdss.buildConfig(config);
      sendI2cMasterSingle(opc_ip, sca_address, tdss, "register12");

      ERS_LOG("SCA " << sca_address << " TDS " << tdss.getName()  << " readback register 14:");

      std::string address_to_read("register14");
      std::string tds_i2c_address("register14_READONLY");

      auto size_in_bytes = tdss.getTotalSize(tds_i2c_address) / NUM_BITS_IN_BYTE;
      std::string full_node_name = sca_address + "." + tdss.getName()  + "." + address_to_read;
      auto dataread = readI2c(opc_ip, full_node_name , size_in_bytes);
      tdss.decodeVector(tds_i2c_address, dataread);
      ERS_LOG("Readback as bytes: ");
      for (auto val : dataread) {
          ERS_LOG("0x" << std::hex << static_cast<uint32_t>(val));
      }
    }

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendAddcConfig(const nsw::ADDCConfig& addc, int i_art) {
    ERS_LOG(addc.getAddress() << " Begin configuration... (i_art = " << i_art << ")");
    size_t art_size = 2;
    uint8_t art_data[] = {0x0, 0x0};
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0, 0x0, 0x0};  // 2 for address (i), 1 for data

    auto opc_ip                      = addc.getOpcServerIp();
    auto sca_addr                    = addc.getAddress();
    auto ARTCoreregisters            = addc.ARTCoreregisters();
    auto ARTregisters                = addc.ARTregisters();
    auto ARTregistervalues           = addc.ARTregistervalues();
    auto GBTx_eport_registers        = addc.GBTx_eport_registers();
    auto ADDC_GBTx_ConfigurationData = addc.GBTx_ConfigurationData();

    // init_sca_rst_gpio
    ERS_DEBUG(1, "ART reset, step 0...");
    if (i_art == -1 || i_art == 0) {
        sendGPIO(opc_ip, sca_addr + ".gpio.art0SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.art0CRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.art0Rstn",  1); usleep(10000);
    }
    if (i_art == -1 || i_art == 1) {
        sendGPIO(opc_ip, sca_addr + ".gpio.art1SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.art1CRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.art1Rstn",  1); usleep(10000);
    }
    ERS_DEBUG(1, " -> done");

    // Reset GBTx0 and GBTx1
    ERS_DEBUG(1, "GBT reset...");
    if (i_art == -1 || i_art == 0) {
        sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 0); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 1); usleep(10000);
    }
    if (i_art == -1 || i_art == 1) {
        sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 0); usleep(10000);
        sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 1); usleep(10000);
    }
    ERS_DEBUG(1, " -> done");

    //
    // SCA supports up to 16-byte payloads
    //
    ERS_DEBUG(1, "GBT configuration");
    size_t chunklen = 16;
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        auto gbtx = sca_addr + "." + art.getNameGbtx();
        std::vector<uint8_t> datas = {};
        for (size_t i = 0; i < ADDC_GBTx_ConfigurationData.size(); i++) {
            if (datas.size() == 0) {
                datas.push_back(((uint8_t) ((i) & 0xff)) );
                datas.push_back(((uint8_t) ((i) >> 8)) );
            }
            datas.push_back(ADDC_GBTx_ConfigurationData.at(i));
            if (datas.size() == chunklen || i+1 == ADDC_GBTx_ConfigurationData.size()) {
                sendI2c(opc_ip, gbtx, datas);
                datas.clear();
            }
        }
        // To fix 6/2 bit split from GBTx bug, try moving the 40 MHz TTC clock as suggested by DM.
        gbtx_data[1] = ((uint8_t) (0) );
        gbtx_data[0] = ((uint8_t) (9) );
        gbtx_data[2] = ((uint8_t) (8) );
        sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size); usleep(10000);
    }
    ERS_DEBUG(1, " -> done");

    // Reset ARTs
    ERS_DEBUG(1, "ART reset");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        auto name = sca_addr + ".gpio." + art.getName();
        ERS_DEBUG(1, "ART reset: " << name);
        sendGPIO(opc_ip, name + "Rstn",  0); usleep(10000);  // reset cfg
        sendGPIO(opc_ip, name + "Rstn",  1); usleep(10000);
        sendGPIO(opc_ip, name + "SRstn", 0); usleep(10000);  // reset i2c
        sendGPIO(opc_ip, name + "SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, name + "CRstn", 0); usleep(10000);  // reset core
        sendGPIO(opc_ip, name + "CRstn", 1); usleep(10000);
    }
    ERS_DEBUG(1, " -> done");

    // art common config
    ERS_DEBUG(1, "ART common config");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        for (auto tup : {std::make_pair("Core", art.core),
                         std::make_pair("Ps",   art.ps)}) {
            auto name = sca_addr + "." + art.getName() + tup.first + "." + art.getName() + tup.first;
            auto addr_bitstr = tup.second.getBitstreamMap();
            ERS_DEBUG(1, "ART common config " << name);
            for (auto ab : addr_bitstr) {
                art_data[0] = static_cast<uint8_t>(std::stoi(ab.first) );
                art_data[1] = static_cast<uint8_t>(std::stoi(ab.second, nullptr, 2) );
                sendI2cRaw(opc_ip, name, art_data, art_size);
            }
        }
    }
    ERS_DEBUG(1, " -> done");

    // Mask ARTs
    ERS_DEBUG(1, "ART mask");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        for (auto reg : ARTCoreregisters) {
            art_data[0] = reg;
            art_data[1] = 0xFF;
            sendI2cRaw(opc_ip, name, art_data, art_size);
        }
    }
    ERS_DEBUG(1, " -> done");

    // Train GBTx
    ERS_DEBUG(1, "Train GBTx");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        bool train;
        auto core = sca_addr + \
            ".art"  + std::to_string(art.index()) + "Core" + \
            ".art"  + std::to_string(art.index()) + "Core";
        auto gbtx = sca_addr + \
            ".gbtx" + std::to_string(art.index()) + \
            ".gbtx" + std::to_string(art.index());

        // ART pattern mode
        ERS_DEBUG(1, "ART pattern mode");
        for (uint i=0; i < ARTregisters.size(); i++) {
            art_data[0] = ARTregisters.at(i);
            art_data[1] = ARTregistervalues.at(i);
            sendI2cRaw(opc_ip, core, art_data, art_size);
        }
        ERS_DEBUG(1, "-> done");

        // GBTx to training mode
        ERS_DEBUG(1, "GBTx training mode");
        gbtx_data[0] = 62;
        gbtx_data[1] = 0;
        gbtx_data[2] = 0x15;
        sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        ERS_DEBUG(1, "-> done");

        // Enable GBTx eport training
        ERS_DEBUG(1, "GBTx eport enable");
        train = 1;
        for (uint i=0; i < GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers.at(i)) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers.at(i)) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        ERS_DEBUG(1, "-> done");

        // Pause
        usleep(1000000);

        // Disable GBTx eport training
        ERS_DEBUG(1, "GBTx eport disable");
        train = 0;
        for (uint i=0; i < GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers.at(i)) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers.at(i)) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        ERS_DEBUG(1, "-> done");

        // ART default mode
        ERS_DEBUG(1, "ART default mode");
        for (auto reg : ARTregisters) {
            auto addr_bitstr = art.core.getBitstreamMap();
            for (auto ab : addr_bitstr) {
                if (reg == static_cast<uint8_t>(std::stoi(ab.first) )) {
                    art_data[0] = static_cast<uint8_t>(std::stoi(ab.first) );
                    art_data[1] = static_cast<uint8_t>(std::stoi(ab.second, nullptr, 2) );
                    sendI2cRaw(opc_ip, core, art_data, art_size);
                    break;
                }
            }
        }
        ERS_DEBUG(1, "-> done");

    }
    ERS_DEBUG(1, " -> done (Train GBTx)");

    // Failsafe mode
    ERS_DEBUG(1, "ART flag mode (failsafe or no)");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        ERS_DEBUG(1, "Failsafe for: " << art.getName() << ": " << art.failsafe());
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        art_data[0] = 3;
        art_data[1] = art.failsafe() ? 0x06 : 0x0E;
        sendI2cRaw(opc_ip, name, art_data, art_size);
        art_data[0] = 4;
        art_data[1] = art.failsafe() ? 0x27 : 0x3F;
        sendI2cRaw(opc_ip, name, art_data, art_size);
    }
    ERS_DEBUG(1, "-> done");

    // Unmask, according to config
    ERS_DEBUG(1, "ART unmask");
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        for (auto reg: ARTCoreregisters) {
            auto addr_bitstr = art.core.getBitstreamMap();
            for (auto ab : addr_bitstr) {
                if (reg == static_cast<uint8_t>( std::stoi(ab.first) )) {
                    art_data[0] = static_cast<uint8_t>( std::stoi(ab.first) );
                    art_data[1] = static_cast<uint8_t>( std::stoi(ab.second, nullptr, 2) );
                    sendI2cRaw(opc_ip, name, art_data, art_size);
                    break;
                }
            }
        }
    }
    ERS_DEBUG(1, "-> done");

    // Adjust ART BCRCLK
    ERS_DEBUG(1, "ART BCRCLK phase");
    uint phase_end = 4;
    for (auto art : addc.getARTs()) {
        if (i_art != -1 && i_art != art.index())
            continue;
        auto name   = addc.getAddress() + "." + art.getNameGbtx();
        uint phase = 0;
        while (phase <= phase_end) {
            // coarse phase
            gbtx_data[0] = 11;
            gbtx_data[1] = 0;
            gbtx_data[2] = (uint8_t)(phase);
            sendI2cRaw(opc_ip, name, gbtx_data, gbtx_size);
            phase = phase + 1;
        }
    }
    ERS_DEBUG(1, "-> done");

    ERS_LOG(addc.getAddress() << " Configuration done.");
}

void nsw::ConfigSender::alignArtGbtxMmtp(const std::map<std::string, nsw::ADDCConfig> & addcs_map,
                                         const std::map<std::string, nsw::TPConfig>   & tps_map) {
  std::vector<nsw::ADDCConfig> addcs = {};
  for (const auto & obj: addcs_map)
    addcs.push_back(obj.second);
  if (tps_map.size() > 1) {
    throw std::runtime_error("Cant align more than 1 MMTP");
  }
  for (const auto & tp: tps_map) {
    alignArtGbtxMmtp(addcs, tp.second);
  }
}

void nsw::ConfigSender::alignArtGbtxMmtp(const std::vector<nsw::ADDCConfig> & addcs,
                                         const nsw::TPConfig & tp) {

  // if no ARTs want to be aligned: exit
  bool go = 0;
  for (const auto & addc : addcs) {
    for (const auto & art: addc.getARTs()) {
      if (!art.TP_GBTxAlignmentSkip()) {
        go = 1;
      }
    }
  }
  if (!go) {
    return;
  }

  // check ART/MMTP communication
  ERS_INFO("Checking ART communication for " << tp.getOpcServerIp() << "." << tp.getAddress());
  size_t n_resets = 0;
  while (true) {

    // admit defeat
    if (n_resets > nsw::mmtp::FIBER_ALIGN_ATTEMPTS) {
      throw std::runtime_error("Failed to stabilize input to " + tp.getAddress() + ". Crashing");
    }

    // allow communication to settle after previous reset
    nsw::snooze(nsw::mmtp::FIBER_ALIGN_SLEEP);

    // read MMTP alignment register
    auto aligned = std::vector<uint32_t>(nsw::mmtp::NUM_FIBERS);
    for (size_t read = 0; read < nsw::mmtp::FIBER_ALIGN_N_READS; read++) {
      const auto word = readSCAXRegisterWord(tp, nsw::mmtp::REG_FIBER_ALIGNMENT);
      for (size_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
        bool align = ((word >> fiber) & 1);
        if (align) {
          aligned.at(fiber)++;
        }
      }
    }

    // skip fibers unless explicitly requested
    auto skipped = std::bitset<nsw::mmtp::NUM_FIBERS>();
    skipped.flip();
    for (const auto & addc : addcs) {
      for (const auto & art : addc.getARTs()) {
        if (!art.TP_GBTxAlignmentSkip()) {
          const auto fiber = art.TP_GBTxAlignmentBit();
          skipped.set(fiber, false);
        }
      }
    }

    // announce
    for (const auto & addc : addcs) {
      for (const auto & art : addc.getARTs()) {
        if (art.TP_GBTxAlignmentSkip()) {
          continue;
        }
        const auto fiber = art.TP_GBTxAlignmentBit();
        const auto align = aligned.at(fiber);
        std::stringstream msg;
        msg << addc.getAddress()
            << "."
            << art.getName()
            << " ("
            << fiber
            << ") "
            << align
            << " aligned out of "
            << nsw::mmtp::FIBER_ALIGN_N_READS
          ;
        if (align < nsw::mmtp::FIBER_ALIGN_N_READS) {
          ERS_INFO(msg.str());
        } else {
          ERS_LOG(msg.str());
        }
      }
    }

    // build the reset
    // if any fiber of a quad has any misalignments,
    // reset that QPLL
    uint32_t reset = 0;
    for (uint32_t qpll = 0; qpll < nsw::mmtp::NUM_QPLL; qpll++) {
      for (uint32_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
        if (skipped.test(fiber)) {
          continue;
        }
        if (fiber / nsw::mmtp::NUM_FIBERS_PER_QPLL != qpll) {
          continue;
        }
        if (aligned.at(fiber) < nsw::mmtp::FIBER_ALIGN_N_READS) {
          reset += (1 << qpll);
          break;
        }
      }
    }
    ERS_INFO("alignArtGbtxMmtp Reset word = " << reset);

    // the moment of truth
    if (reset == 0) {
      ERS_INFO("alignArtGbtxMmtp success!");
      break;
    }

    // or, set/unset the reset
    sendSCAXRegister(tp, nsw::mmtp::REG_FIBER_QPLL_RESET, reset);
    sendSCAXRegister(tp, nsw::mmtp::REG_FIBER_QPLL_RESET, 0x00);

    // and take note
    n_resets++;
  }
}

std::vector<uint8_t> nsw::ConfigSender::readSCAXRegister(const nsw::SCAConfig& scax, uint8_t address) {
  auto addr = nsw::intToByteVector(address, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  auto data = readI2cAtAddress(scax.getOpcServerIp(), scax.getAddress(), addr.data(), addr.size(), nsw::NUM_BYTES_IN_WORD32);
  return data;
}

uint32_t nsw::ConfigSender::readSCAXRegisterWord(const nsw::SCAConfig& scax, uint8_t address) {
  return nsw::byteVectorToWord32(readSCAXRegister(scax, address), nsw::scax::SCAX_LITTLE_ENDIAN);
}

void nsw::ConfigSender::sendSCAXRegister(const nsw::SCAConfig& scax, uint8_t address, uint32_t message, bool quiet) {
  auto data = nsw::intToByteVector(message, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  auto addr = nsw::intToByteVector(address, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  std::vector<uint8_t> payload(addr);
  payload.insert(payload.end(), data.begin(), data.end() );
  if (!quiet)
    ERS_LOG("... writing to TP: address, message =  " <<
            static_cast<int>(address) << ", " << static_cast<int>(message) );
  sendI2cRaw(scax.getOpcServerIp(), scax.getAddress(), payload.data(), payload.size());
}

void nsw::ConfigSender::sendTPCarrierConfig(const nsw::TPCarrierConfig& carrier, const bool quiet) {
  sendSCAXRegister(carrier, nsw::carrier::REG_RJOUT_SEL, carrier.RJOutSel(), quiet);
}

void nsw::ConfigSender::sendTPConfig(const nsw::TPConfig& tp, bool quiet) {
    //
    // Collect registers to be written
    //
    std::vector<std::pair<uint8_t, uint32_t> > list_of_messages = {
      {nsw::mmtp::REG_ADDC_EMU_DISABLE,  static_cast<uint32_t>(true)},
      {nsw::mmtp::REG_L1A_LATENCY,       static_cast<uint32_t>(tp.ARTWindowCenter())},
      {nsw::mmtp::REG_L1A_WIN_UPPER,     static_cast<uint32_t>(tp.ARTWindowLeft())},
      {nsw::mmtp::REG_L1A_WIN_LOWER,     static_cast<uint32_t>(tp.ARTWindowRight())},
      {nsw::mmtp::REG_L1A_CONTROL,       nsw::mmtp::L1A_RESET_ENABLE},
      {nsw::mmtp::REG_L1A_CONTROL,       nsw::mmtp::L1A_RESET_DISABLE},
      {nsw::mmtp::REG_FIBER_BC_OFFSET,   static_cast<uint32_t>(tp.FiberBCOffset())},
      {nsw::mmtp::REG_INPUT_PHASE,       static_cast<uint32_t>(tp.GlobalInputPhase())},
      {nsw::mmtp::REG_HORX_ENV_MON_ADDR, static_cast<uint32_t>(tp.HorxEnvMonAddr())},
    };
    if (tp.GlobalInputOffset() != -1)
      list_of_messages.push_back(std::make_pair(nsw::mmtp::REG_INPUT_PHASEOFFSET,        static_cast<uint32_t>(tp.GlobalInputOffset())));
    if (tp.SelfTriggerDelay() != -1)
      list_of_messages.push_back(std::make_pair(nsw::mmtp::REG_SELFTRIGGER_DELAY,        static_cast<uint32_t>(tp.SelfTriggerDelay())));
    if (tp.VmmMaskHotThresh() != -1)
      list_of_messages.push_back(std::make_pair(nsw::mmtp::REG_VMM_MASK_HOT_THRESH,      static_cast<uint32_t>(tp.VmmMaskHotThresh())));
    if (tp.VmmMaskHotThreshHyst() != -1)
      list_of_messages.push_back(std::make_pair(nsw::mmtp::REG_VMM_MASK_HOT_THRESH_HYST, static_cast<uint32_t>(tp.VmmMaskHotThreshHyst())));
    if (tp.VmmMaskDrainPeriod() != -1)
      list_of_messages.push_back(std::make_pair(nsw::mmtp::REG_VMM_MASK_DRAIN_PERIOD,    static_cast<uint32_t>(tp.VmmMaskDrainPeriod())));

    const auto skippedReg = tp.SkipRegisters<std::uint8_t>();
    //
    // Write registers
    //
    for (auto element : list_of_messages) {
      // found the skipped reg, skipping
      if (skippedReg.find(element.first) != skippedReg.end()) {
        continue;
      }
      sendSCAXRegister(tp, element.first, element.second, quiet);
    }

    //
    // Fiber BC Offset. Each bit corresponds to a delay of 1 BC for each fiber.
    // Global 0xB knob. An overall global delay for all inputs w.r.t. the TP's FELIX-derived clock
    // Global 0xC knob. An overall global delay for half of inputs, in addition to 0xB
    // Self-trigger delay
    //


// Once TP SCAX registers are autogenerated in NSWSCAXRegisters,
// ... this model can be used. For the meantime, we're setting certain registers by hand.
//
//     std::map<std::string, I2cMasterConfig*> masters = tp.getI2cMastersMap();
//     for (int i = 0; i < tp.getNumMasters(); i++) {
//         if ( !masters[registerFilesNamesArr[i]] ) continue;
//         ERS_LOG("Sending I2c configuration to " << tp_address << "." <<
//             masters[registerFilesNamesArr[i]]->getName());
//         auto addr_bitstr = masters[registerFilesNamesArr[i]]->getBitstreamMap();
//         std::vector<std::string> key_vec;
//         for (auto regEntry : registerFilesOrderArr[i]) {
//             key_vec.push_back(regEntry);
//         }
//         std::vector<std::string>::iterator it;

//         for (auto ab : addr_bitstr) {
//             it = std::find(key_vec.begin(), key_vec.end(), ab.first);
//             auto registerAddress = nsw::intToByteVector(std::distance(key_vec.begin(), it), 4);
//             auto address = tp_address + "." + masters[registerFilesNamesArr[i]]->getName() +
//                 "." + "bus" + std::to_string(i);
//             auto bitstr = std::string(32 - ab.second.length(), '0') + ab.second;
//             auto data = nsw::stringToByteVector(bitstr);
//             std::reverse(data.begin(), data.end());
//             data.insert(data.begin(), registerAddress.begin(), registerAddress.end());
//             for (auto d : data) {
//                 ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
//             }
//             sendI2cRaw(opc_ip, address, data.data(), data.size());
//         }
//     }
}

std::vector<short unsigned int> nsw::ConfigSender::readAnalogInputConsecutiveSamples(const std::string& opcserver_ipport,
    const std::string& node, size_t n_samples) {
    addOpcClientIfNew(opcserver_ipport);
    ERS_DEBUG(4, "Reading " <<  n_samples << " consecutive samples from " << node);
    return m_clients[opcserver_ipport]->readAnalogInputConsecutiveSamples(node, n_samples);
}

std::vector<short unsigned int> nsw::ConfigSender::readVmmPdoConsecutiveSamples(FEBConfig& feb,
    size_t vmm_id, size_t n_samples) {
    const auto opc_ip      = feb.getOpcServerIp();
    const auto feb_address = feb.getAddress();
    // auto& vmms       = feb.getVmms();

    // FIXME TODO unsafe access for any board that has first VMM not
    // at position 0.
    // Calling code should be safe, or internally here it should be
    // made more robust
    // vmms.at(vmm_id).setGlobalRegister("sbmx", 1);  // Route analog monitor to pdo output
    // vmms.at(vmm_id).setGlobalRegister("sbfp", 1);  // Enable PDO output buffers (more stable reading)

    // Internally getVmm handles the offset to extract the correct VMM
    auto& vmm = feb.getVmm(vmm_id);
    vmm.setGlobalRegister("sbmx", 1);  // Route analog monitor to pdo output
    vmm.setGlobalRegister("sbfp", 1);  // Enable PDO output buffers (more stable reading)

    sendVmmConfigSingle(feb, vmm_id);

    return readAnalogInputConsecutiveSamples(opc_ip, feb_address + ".ai.vmmPdo" + std::to_string(vmm_id), n_samples);
}



std::uint32_t nsw::ConfigSender::readSCAID(FEBConfig& feb) {

    auto opc_ip      = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();

    addOpcClientIfNew(opc_ip);

    return m_clients[opc_ip]->readScaID(feb_address);
}

std::string nsw::ConfigSender::readSCAAddress(FEBConfig& feb) {

    auto opc_ip      = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();

    addOpcClientIfNew(opc_ip);

    return m_clients[opc_ip]->readScaAddress(feb_address);
}

bool nsw::ConfigSender::readSCAOnline(FEBConfig& feb) {

    auto opc_ip      = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();

    addOpcClientIfNew(opc_ip);

    return m_clients[opc_ip]->readScaOnline(feb_address);
}

void nsw::ConfigSender::sendFPGA(const std::string& opcserver_ipport, const std::string& node,
                                 const std::string& bitfile_path) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeXilinxFpga(node, bitfile_path);
}

void nsw::ConfigSender::enableVmmCaptureInputs(const nsw::FEBConfig& feb)
{
    ptree tree;
    tree.put_child("reg008vmmEnable", feb.getConfig().get_child("rocCoreDigital.reg008vmmEnable"));
    const auto configConverter = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(tree, ConfigType::REGISTER_BASED);
    const auto translatedPtree = configConverter.getFlatRegisterBasedConfig(feb.getRocDigital().getBitstreamMap());
    const auto partialConfig = nsw::I2cMasterConfig(translatedPtree, ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS, true);
    sendI2cMasterConfig(feb.getOpcServerIp(), feb.getAddress(), partialConfig);
}

void nsw::ConfigSender::disableVmmCaptureInputs(const nsw::FEBConfig& feb)
{
    ptree tree;
    tree.put("reg008vmmEnable", 0);
    const auto partialConfig = nsw::I2cMasterConfig(tree, ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS, true);
    sendI2cMasterConfig(feb.getOpcServerIp(), feb.getAddress(), partialConfig);
}

void nsw::ConfigSender::setVMMConfigurationStatusInfoDCS(const nsw::FEBConfig& feb, const nsw::VMMConfig& vmm) {

    ERS_DEBUG(1, "[" + feb.getAddress() + "," + vmm.getName() + "]" 
    	+ " Write VMMConfigurationStatusInfo FreeVariable parameter for DCS Use");

    auto opc_ip = feb.getOpcServerIp();

    // VMM registers for temperature monitoring
    const int scmx = vmm.getGlobalRegister("scmx");
    const int sbmx = vmm.getGlobalRegister("sbmx");
    const int sbfp = vmm.getGlobalRegister("sbfp");
    const int sm = vmm.getGlobalRegister("sm");
    const int reset = vmm.getGlobalRegister("reset");

    const bool isVMMTemperatureModeEnabled = (scmx==0 && sbmx==1 && sbfp==1 && sm==4 && reset!=3)? true:false ;

    writeFreeVariable(opc_ip, feb.getAddress() + ".spi." + vmm.getName() + ".configurationStatus", isVMMTemperatureModeEnabled);

}
