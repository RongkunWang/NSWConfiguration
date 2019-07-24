#include <memory>
#include <string>
#include <vector>

#include "ers/ers.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/Utility.h"

nsw::ConfigSender::ConfigSender() {
}

void nsw::ConfigSender::addOpcClientIfNew(std::string opcserver_ipport) {
    if (m_clients.find(opcserver_ipport) == m_clients.end()) {
        m_clients.emplace(opcserver_ipport, std::make_unique<nsw::OpcClient>(opcserver_ipport));
    }
}

void nsw::ConfigSender::sendSpiRaw(std::string opcserver_ipport, std::string node, uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, data, data_size);
}

std::vector<uint8_t> nsw::ConfigSender::readSpi(std::string opcserver_ipport, std::string node, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readSpiSlave(node, data_size);
}

void nsw::ConfigSender::sendSpi(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, vdata.data(), vdata.size());
}

void nsw::ConfigSender::sendI2cRaw(std::string opcserver_ipport, std::string node, uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeI2cRaw(node, data, data_size);
}

void nsw::ConfigSender::sendI2c(std::string opcserver_ipport, std::string node, std::vector<uint8_t> vdata) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeI2cRaw(node, vdata.data(), vdata.size());
}

void nsw::ConfigSender::sendGPIO(std::string opcserver_ipport, std::string node, bool data) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeGPIO(node, data);
}

bool nsw::ConfigSender::readGPIO(std::string opcserver_ipport, std::string node) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readGPIO(node);
}

std::vector<uint8_t> nsw::ConfigSender::readI2c(std::string opcserver_ipport,
                                                std::string node,
                                                size_t number_of_bytes) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readI2c(node, number_of_bytes);
}

std::vector<uint8_t> nsw::ConfigSender::readI2cAtAddress(std::string opcserver_ipport,
                                                         std::string node,
                                                         uint8_t* address,
                                                         size_t address_size,
                                                         size_t number_of_bytes) {
    // Write only the address without data
    nsw::ConfigSender::sendI2cRaw(opcserver_ipport, node, address, address_size);

    // Read back data into the vector readdata
    std::vector<uint8_t> readdata = nsw::ConfigSender::readI2c(opcserver_ipport, node, number_of_bytes);
    return readdata;
}

void nsw::ConfigSender::sendVmmConfig(const nsw::VMMConfig& cfg) {
    auto data = cfg.getByteVector();
    sendSpiRaw(cfg.getOpcServerIp(), cfg.getAddress(), data.data(), data.size());
}

void nsw::ConfigSender::sendI2cMasterSingle(std::string opcserver_ipport, std::string topnode,
                                            const nsw::I2cMasterConfig& cfg, std::string reg_address) {
    ERS_LOG("Sending I2c configuration to " << topnode << "." << reg_address);
    auto addr_bitstr = cfg.getBitstreamMap();
    auto address = topnode + "." + cfg.getName() + "." + reg_address;  // Full I2C address
    auto bitstr = addr_bitstr[reg_address];
    auto data = nsw::stringToByteVector(bitstr);
    for (auto d : data) {
        ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
    }
        sendI2cRaw(opcserver_ipport, address, data.data(), data.size());
}

void nsw::ConfigSender::sendI2cMasterConfig(std::string opcserver_ipport,
                                            std::string topnode, const nsw::I2cMasterConfig& cfg) {
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

void nsw::ConfigSender::sendRocConfig(const nsw::ROCConfig& roc) {
    auto opc_ip = roc.getOpcServerIp();
    auto roc_address = roc.getAddress();

    // 1. Reset all logics
    sendGPIO(opc_ip, roc_address + ".gpio.rocCoreResetN", 0);
    sendGPIO(opc_ip, roc_address + ".gpio.rocPllResetN", 0);
    sendGPIO(opc_ip, roc_address + ".gpio.rocSResetN", 0);

    sendGPIO(opc_ip, roc_address + ".gpio.rocSResetN", 1);

    sendI2cMasterConfig(opc_ip, roc_address, roc.analog);

    sendGPIO(opc_ip, roc_address + ".gpio.rocPllResetN", 1);

    ERS_DEBUG(2, "Waiting for ROC Pll locks...");
    bool roc_locked = 0;
    while (!roc_locked) {
        bool rPll1 = readGPIO(opc_ip, roc_address + ".gpio.rocPllLocked");
        bool rPll2 = readGPIO(opc_ip, roc_address + ".gpio.rocPllRocLocked");
        roc_locked = rPll1 & rPll2;
        ERS_DEBUG(2, "rocPllLocked: " << rPll1 << ", rocPllRocLocked: " << rPll2);
    }

    sendGPIO(opc_ip, roc_address + ".gpio.rocCoreResetN", 1);

    sendI2cMasterConfig(opc_ip, roc_address, roc.digital);
}

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
        auto data = vmm.getByteVector();
        std::vector<uint8_t> dat;
        for (int i = 0; i < 216; i++) {
          dat.push_back(0x84);
        }
        std::cout << "size : " << dat.size() << std::endl;
        ERS_LOG("Sending configuration to " << feb.getAddress() << ".spi." << vmm.getName());
        sendSpiRaw(opc_ip, feb.getAddress() + ".spi." + vmm.getName() , data.data(), data.size());
        ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));
    }

    // Set Vmm Acquisition Enable
    data = {0x0};
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);
}

void nsw::ConfigSender::sendVmmConfigSingle(const nsw::FEBConfig& feb, size_t vmm_id) {
    // Set Vmm Configuration Enable
    std::vector<uint8_t> data = {0xff};
    auto opc_ip = feb.getOpcServerIp();
    auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);

    auto vmm = feb.getVmms()[vmm_id];
    auto vmmdata = vmm.getByteVector();
    ERS_DEBUG(1, "Sending I2c configuration to " << feb.getAddress() << ".spi." << vmm.getName());
    sendSpiRaw(opc_ip, feb.getAddress() + ".spi." + vmm.getName() , vmmdata.data(), vmmdata.size());
    ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));

    // Set Vmm Acquisition Enable
    data = {0x0};
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);
}

void nsw::ConfigSender::sendTdsConfig(const nsw::FEBConfig& feb) {
    auto opc_ip = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();
    for (auto tds : feb.getTdss()) {
        sendTdsConfig(opc_ip, feb_address, tds);
    }
}

void nsw::ConfigSender::sendRocConfig(std::string opc_ip, std::string sca_address,
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

void nsw::ConfigSender::sendTdsConfig(const nsw::TDSConfig& tds) {
    auto opc_ip = tds.getOpcServerIp();
    auto tds_address = tds.getAddress();

    sendGPIO(opc_ip, tds_address + ".gpio.tdsReset", 1);

    sendI2cMasterConfig(opc_ip, tds_address, tds.i2c);

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendTdsConfig(std::string opc_ip, std::string sca_address, const I2cMasterConfig & tds) {
    sendGPIO(opc_ip, sca_address + ".gpio.tdsReset", 1);

    sendI2cMasterConfig(opc_ip, sca_address, tds);

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendAddcConfig(const nsw::ADDCConfig& feb) {

    size_t art_size = 2;
    uint8_t art_data[] = {0x0,0x0};
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0}; // 2 for address (i), 1 for data
    int progress = 1;
    int art_mask = 0;
    bool train = 0;

    auto opc_ip                      = feb.getOpcServerIp();
    auto sca_address                 = feb.getAddress();
    auto ARTCoreregisters            = feb.ARTCoreregisters();
    auto ARTregisters                = feb.ARTregisters();
    auto ARTregistervalues           = feb.ARTregistervalues();
    auto GBTx_eport_registers        = feb.GBTx_eport_registers();
    auto ADDC_GBTx_ConfigurationData = feb.GBTx_ConfigurationData();

    // art
    for (auto art: feb.getARTs()) {

        std::string gpio = ".gpio.art" + std::to_string(art.index());

        // reset cfg
        sendGPIO(opc_ip, sca_address + gpio + "Rstn",  0);  usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "Rstn",  1);  usleep(10000);
        // reset i2c
        sendGPIO(opc_ip, sca_address + gpio + "SRstn", 0); usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "SRstn", 1); usleep(10000);
        // reset core
        sendGPIO(opc_ip, sca_address + gpio + "CRstn", 0); usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "CRstn", 1); usleep(10000);

        // init sca rst
        sendGPIO(opc_ip, sca_address + gpio + "SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "CRstn", 1); usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "Rstn",  1); usleep(10000);

    }

    // gbt
    for (auto gbtx: {0, 1}) {

        std::string gpio = ".gpio.gbtx" + std::to_string(gbtx);
        std::string gbtx_str = ".gbtx" + std::to_string(gbtx) + ".gbtx" + std::to_string(gbtx);

        sendGPIO(opc_ip, sca_address + gpio + "Rstn", 0); usleep(10000);
        sendGPIO(opc_ip, sca_address + gpio + "Rstn", 1); usleep(10000);

        int pr = 0;
        for (uint i=0; i<ADDC_GBTx_ConfigurationData.size(); i++) {
            if((((i*20)/ADDC_GBTx_ConfigurationData.size())%20) - pr == 1 && progress) {
                pr++;
                std::cout << "." << std::flush;
            }
            gbtx_data[1] = ((uint8_t) ((i) >> 8));
            gbtx_data[0] = ((uint8_t) ((i) & 0xff));
            gbtx_data[2] = ADDC_GBTx_ConfigurationData[i];
            sendI2cRaw(opc_ip, sca_address + gbtx_str, gbtx_data, gbtx_size);
        }

    }

    // more art
    for (auto art: feb.getARTs()) {

        std::string core = ".art" + std::to_string(art.index()) + "Core.art" + std::to_string(art.index()) + "Core";

        // art_bcid_config
        art_data[0] = 14;
        art_data[1] = 0xF0;
        sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);
        art_data[0] = 15;
        art_data[1] = 0xFF;
        sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);

        // art_mask_config
        for (uint i=0; i<ARTCoreregisters.size(); i++) {
            art_data[0] = ARTCoreregisters[i];
            art_data[1] = (uint8_t)((art_mask >> (8*i)) & 0xff);
            sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);
        }

        // art_failsafe_mode
        bool fs = art.getEnableFailsafe();
        art_data[0] = 3;
        art_data[1] = (fs) ? 0x06 : 0x0E;
        sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);
        art_data[0] = 4;
        art_data[1] = (fs) ? 0x27 : 0x3F;
        sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);

        // art_pattern_mode_config
        for (uint i=0; i<ARTregisters.size(); i++) {
            art_data[0] = ARTregisters[i];
            art_data[1] = ARTregistervalues[i];
            sendI2cRaw(opc_ip, sca_address + core, art_data, art_size);
        }

    }

    // more gbt
    for (auto gbtx: {0, 1}) {
        std::string gbtx_str = ".gbtx" + std::to_string(gbtx) + ".gbtx" + std::to_string(gbtx);
        for (uint i=0; i<GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, sca_address + gbtx_str, gbtx_data, gbtx_size);
        }
    }
}

std::vector<short unsigned int> nsw::ConfigSender::readAnalogInputConsecutiveSamples(std::string opcserver_ipport,
                                                                        std::string node, 
                                                                        size_t n_samples) {
    addOpcClientIfNew(opcserver_ipport);
    ERS_DEBUG(4, "Reading " <<  n_samples << " consecutive samples from " << node);
    return m_clients[opcserver_ipport]->readAnalogInputConsecutiveSamples(node, n_samples);

}

std::vector<short unsigned int> nsw::ConfigSender::readVmmPdoConsecutiveSamples(FEBConfig& feb,
                                                                                size_t vmm_id,
                                                                                size_t n_samples) {

    auto opc_ip      = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();
    auto& vmms       = feb.getVmms();
    
    vmms[vmm_id].setGlobalRegister("sbmx", 1);  // Route analog monitor to pdo output
    vmms[vmm_id].setGlobalRegister("sbfp", 1);  // Enable PDO output buffers (more stable reading)
    
    sendVmmConfigSingle(feb, vmm_id);
    
    return readAnalogInputConsecutiveSamples(opc_ip, feb_address + ".ai.vmmPdo" + std::to_string(vmm_id), n_samples);

}

