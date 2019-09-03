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
    std::string node, size_t number_of_bytes) {
    addOpcClientIfNew(opcserver_ipport);
    return m_clients[opcserver_ipport]->readI2c(node, number_of_bytes);
}

std::vector<uint8_t> nsw::ConfigSender::readI2cAtAddress(std::string opcserver_ipport,
    std::string node, uint8_t* address, size_t address_size, size_t number_of_bytes) {
    // Write only the address without data
    nsw::ConfigSender::sendI2cRaw(opcserver_ipport, node, address, address_size);

    // Read back data into the vector readdata
    std::vector<uint8_t> readdata = nsw::ConfigSender::readI2c(opcserver_ipport, node, number_of_bytes);
    return readdata;
}

void nsw::ConfigSender::sendI2cAtAddress(std::string opcserver_ipport,
                                         std::string node,
                                         std::vector<uint8_t> address,
                                         std::vector<uint8_t> data) {

    // Insert the address in the beginning of data vector
    for (auto & address_byte : address){
        // data.push_front(address_byte);
        data.insert(data.begin(), address_byte);
    }
    nsw::ConfigSender::sendI2cRaw(opcserver_ipport, node, data.data(), data.size());
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
  // this is used for outside
    auto opc_ip = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();
    // HACK!
    int ntds = feb.getTdss().size();
    for (auto tds : feb.getTdss()) {
        sendTdsConfig(opc_ip, feb_address, tds, ntds);
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
  // unused yet
    auto opc_ip = tds.getOpcServerIp();
    auto tds_address = tds.getAddress();

    sendGPIO(opc_ip, tds_address + ".gpio.tdsReset", 1);

    sendI2cMasterConfig(opc_ip, tds_address, tds.i2c);

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendTdsConfig(std::string opc_ip, std::string sca_address, const I2cMasterConfig & tds, int ntds) {
  // internal
  if (ntds <= 3)
    // old boards
      sendGPIO(opc_ip, sca_address + ".gpio.tdsReset", 1);
  else
  {
    // new boards
      sendGPIO(opc_ip, sca_address + ".gpio.tdsaReset", 1);
      sendGPIO(opc_ip, sca_address + ".gpio.tdsbReset", 1);
      sendGPIO(opc_ip, sca_address + ".gpio.tdscReset", 1);
      sendGPIO(opc_ip, sca_address + ".gpio.tdsdReset", 1);
  }

    sendI2cMasterConfig(opc_ip, sca_address, tds);

    // Read back to verify something? (TODO)
}

void nsw::ConfigSender::sendAddcConfig(const nsw::ADDCConfig& addc) {

    bool verbose = 1;
    size_t art_size = 2;
    uint8_t art_data[] = {0x0,0x0};
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0}; // 2 for address (i), 1 for data
    // int progress = 1;

    auto opc_ip                      = addc.getOpcServerIp();
    auto sca_addr                    = addc.getAddress();
    auto ARTCoreregisters            = addc.ARTCoreregisters();
    auto ARTregisters                = addc.ARTregisters();
    auto ARTregistervalues           = addc.ARTregistervalues();
    auto GBTx_eport_registers        = addc.GBTx_eport_registers();
    auto ADDC_GBTx_ConfigurationData = addc.GBTx_ConfigurationData();

    // init_sca_rst_gpio
    if (verbose)
        std::cout << "ART reset, step 0..." << std::endl;
    sendGPIO(opc_ip, sca_addr + ".gpio.art0SRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1SRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art0CRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1CRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art0Rstn",  1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1Rstn",  1); usleep(10000);
    if (verbose)
        std::cout << " -> done" << std::endl;

    // Reset GBTx0 and GBTx1
    if (verbose)
        std::cout << "GBT reset..." << std::endl;
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 0); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 0); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 1); usleep(10000);
    if (verbose)
        std::cout << " -> done" << std::endl;

    // Set GBTx0 and GBTx1 configuration
    if (verbose)
        std::cout << "GBT configuration" << std::endl;
    for (auto art: addc.getARTs()) {
        auto gbtx = sca_addr + "." + art.getNameGbtx();
        for (uint i = 0; i < ADDC_GBTx_ConfigurationData.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((i) >> 8));
            gbtx_data[0] = ((uint8_t) ((i) & 0xff));
            gbtx_data[2] = ADDC_GBTx_ConfigurationData[i];
            if (i % 50 == 0)
                if (verbose)
                    std::cout << "GBT configuration of " << gbtx << " " << i << " / " << ADDC_GBTx_ConfigurationData.size() << std::endl;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size); usleep(10000);
        }
    }
    if (verbose)
        std::cout << " -> done" << std::endl;

    // Reset ARTs
    if (verbose)
        std::cout << "ART reset" << std::endl;
    for (auto art: addc.getARTs()) {
        auto name = sca_addr + ".gpio." + art.getName();
        if (verbose)
            std::cout << "ART reset: " << name << std::endl;
        sendGPIO(opc_ip, name + "Rstn",  0); usleep(10000); // reset cfg
        sendGPIO(opc_ip, name + "Rstn",  1); usleep(10000);
        sendGPIO(opc_ip, name + "SRstn", 0); usleep(10000); // reset i2c
        sendGPIO(opc_ip, name + "SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, name + "CRstn", 0); usleep(10000); // reset core
        sendGPIO(opc_ip, name + "CRstn", 1); usleep(10000);
    }
    if (verbose)
        std::cout << " -> done" << std::endl;

    // art common config
    if (verbose)
        std::cout << "ART common config" << std::endl;
    for (auto art: addc.getARTs()) {
        for (auto tup: {std::make_pair("Core", art.core),
                        std::make_pair("Ps",   art.ps)}) {
            auto name = sca_addr + "." + art.getName() + tup.first + "." + art.getName() + tup.first;
            auto addr_bitstr = tup.second.getBitstreamMap();
            if (verbose)
                std::cout << "ART common config " << name << std::endl;
            for (auto ab : addr_bitstr) {
                art_data[0] = static_cast<uint8_t>( std::stoi(ab.first) );
                art_data[1] = static_cast<uint8_t>( std::stoi(ab.second, nullptr, 2) );
                sendI2cRaw(opc_ip, name, art_data, art_size);
            }
        }
    }
    if (verbose)
        std::cout << " -> done" << std::endl;

    // Mask ARTs
    if (verbose)
        std::cout << "ART mask" << std::endl;
    for (auto art: addc.getARTs()) {
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        for (auto reg: ARTCoreregisters) {
            art_data[0] = reg;
            art_data[1] = 0xFF;
            sendI2cRaw(opc_ip, name, art_data, art_size);
        }
    }
    if (verbose)
        std::cout << " -> done" << std::endl;

    // Train GBTx
    if (verbose)
        std::cout << "Train GBTx" << std::endl;
    for (auto art: addc.getARTs()) {

        bool train;
        auto core = sca_addr + \
            ".art"  + std::to_string(art.index()) + "Core" + \
            ".art"  + std::to_string(art.index()) + "Core";
        auto gbtx = sca_addr + \
            ".gbtx" + std::to_string(art.index()) + \
            ".gbtx" + std::to_string(art.index());

        // ART pattern mode
        if (verbose)
            std::cout << "ART pattern mode" << std::endl;
        for (uint i=0; i<ARTregisters.size(); i++) {
            art_data[0] = ARTregisters[i];
            art_data[1] = ARTregistervalues[i];
            sendI2cRaw(opc_ip, core, art_data, art_size);
        }
        if (verbose)
            std::cout << "-> done" << std::endl;

        // GBTx to training mode
        if (verbose)
            std::cout << "GBTx training mode" << std::endl;
        gbtx_data[0] = 62;
        gbtx_data[1] = 0;
        gbtx_data[2] = 0x15;
        sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        if (verbose)
            std::cout << "-> done" << std::endl;

        // Enable GBTx eport training
        if (verbose)
            std::cout << "GBTx eport enable" << std::endl;
        train = 1;
        for (uint i=0; i<GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        if (verbose)
            std::cout << "-> done" << std::endl;

        // Pause
        usleep(1000000);

        // Disable GBTx eport training
        if (verbose)
            std::cout << "GBTx eport disable" << std::endl;
        train = 0;
        for (uint i=0; i<GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        if (verbose)
            std::cout << "-> done" << std::endl;

        // ART default mode
        if (verbose)
            std::cout << "ART default mode" << std::endl;
        for (auto reg: ARTregisters) {
            auto addr_bitstr = art.core.getBitstreamMap();
            for (auto ab : addr_bitstr) {
                if (reg == static_cast<uint8_t>( std::stoi(ab.first) )) {
                    art_data[0] = static_cast<uint8_t>( std::stoi(ab.first) );
                    art_data[1] = static_cast<uint8_t>( std::stoi(ab.second, nullptr, 2) );
                    sendI2cRaw(opc_ip, core, art_data, art_size);
                    break;
                }
            }
        }
        if (verbose)
            std::cout << "-> done" << std::endl;

    }
    if (verbose)
        std::cout << " -> done (Train GBTx)" << std::endl;

    // Failsafe mode
    if (verbose)
        std::cout << "ART flag mode (failsafe or no)" << std::endl;
    bool failsafe = 1;
    for (auto art: addc.getARTs()) {
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        art_data[0] = 3;
        art_data[1] = failsafe ? 0x06 : 0x0E;
        sendI2cRaw(opc_ip, name, art_data, art_size);
        art_data[0] = 4;
        art_data[1] = failsafe ? 0x27 : 0x3F;
        sendI2cRaw(opc_ip, name, art_data, art_size);
    }
    if (verbose)
        std::cout << "-> done" << std::endl;

    // Unmask, according to config
    if (verbose)
        std::cout << "ART unmask" << std::endl;
    for (auto art: addc.getARTs()) {
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
    if (verbose)
        std::cout << "-> done" << std::endl;
}

void nsw::ConfigSender::sendTpConfig(nsw::TPConfig& tp) {
    auto opc_ip = tp.getOpcServerIp();
    auto tp_address = tp.getAddress();

    std::map<std::string, I2cMasterConfig*> masters = tp.getI2cMastersMap();
    for (int i = 0; i < tp.getNumMasters(); i++) {
        if ( !masters[registerFilesNamesArr[i]] ) continue;
        ERS_LOG("Sending I2c configuration to " << tp_address << "." << masters[registerFilesNamesArr[i]]->getName());
        auto addr_bitstr = masters[registerFilesNamesArr[i]]->getBitstreamMap();
        std::vector<std::string> key_vec;
        for (auto regEntry : registerFilesOrderArr[i]) {
            key_vec.push_back(regEntry);
        }
        std::vector<std::string>::iterator it;

        for (auto ab : addr_bitstr) {
            it = std::find(key_vec.begin(), key_vec.end(), ab.first);
            auto registerAddress = nsw::intToByteVector(std::distance(key_vec.begin(), it), 4);
            auto address = tp_address + "." + masters[registerFilesNamesArr[i]]->getName() +
                "." + "bus" + std::to_string(i);
            auto bitstr = std::string(32 - ab.second.length(), '0') + ab.second;
            auto data = nsw::stringToByteVector(bitstr);
            std::reverse(data.begin(), data.end());
            data.insert(data.begin(), registerAddress.begin(), registerAddress.end());
            for (auto d : data) {
                ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
            }
            sendI2cRaw(opc_ip, address, data.data(), data.size());
        }
    }
}

std::vector<short unsigned int> nsw::ConfigSender::readAnalogInputConsecutiveSamples(std::string opcserver_ipport,
    std::string node, size_t n_samples) {
    addOpcClientIfNew(opcserver_ipport);
    ERS_DEBUG(4, "Reading " <<  n_samples << " consecutive samples from " << node);
    return m_clients[opcserver_ipport]->readAnalogInputConsecutiveSamples(node, n_samples);
}

std::vector<short unsigned int> nsw::ConfigSender::readVmmPdoConsecutiveSamples(FEBConfig& feb,
    size_t vmm_id, size_t n_samples) {
    auto opc_ip      = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();
    auto& vmms       = feb.getVmms();

    vmms[vmm_id].setGlobalRegister("sbmx", 1);  // Route analog monitor to pdo output
    vmms[vmm_id].setGlobalRegister("sbfp", 1);  // Enable PDO output buffers (more stable reading)

    sendVmmConfigSingle(feb, vmm_id);

    return readAnalogInputConsecutiveSamples(opc_ip, feb_address + ".ai.vmmPdo" + std::to_string(vmm_id), n_samples);
}

