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

    ERS_LOG(addc.getAddress() << " Begin configuration...");
    size_t art_size = 2;
    uint8_t art_data[] = {0x0,0x0};
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0}; // 2 for address (i), 1 for data

    auto opc_ip                      = addc.getOpcServerIp();
    auto sca_addr                    = addc.getAddress();
    auto ARTCoreregisters            = addc.ARTCoreregisters();
    auto ARTregisters                = addc.ARTregisters();
    auto ARTregistervalues           = addc.ARTregistervalues();
    auto GBTx_eport_registers        = addc.GBTx_eport_registers();
    auto ADDC_GBTx_ConfigurationData = addc.GBTx_ConfigurationData();

    // init_sca_rst_gpio
    ERS_DEBUG(1, "ART reset, step 0...");
    sendGPIO(opc_ip, sca_addr + ".gpio.art0SRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1SRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art0CRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1CRstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art0Rstn",  1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.art1Rstn",  1); usleep(10000);
    ERS_DEBUG(1, " -> done");

    // Reset GBTx0 and GBTx1
    ERS_DEBUG(1, "GBT reset...");
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 0); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx0Rstn", 1); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 0); usleep(10000);
    sendGPIO(opc_ip, sca_addr + ".gpio.gbtx1Rstn", 1); usleep(10000);
    ERS_DEBUG(1, " -> done");

    // Set GBTx0 and GBTx1 configuration
    ERS_DEBUG(1, "GBT configuration");
    for (auto art: addc.getARTs()) {
        auto gbtx = sca_addr + "." + art.getNameGbtx();
        for (uint i = 0; i < ADDC_GBTx_ConfigurationData.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((i) >> 8));
            gbtx_data[0] = ((uint8_t) ((i) & 0xff));
            gbtx_data[2] = ADDC_GBTx_ConfigurationData[i];
            if (i % 50 == 0)
                ERS_DEBUG(1, "GBT configuration of "
                          << gbtx << " " << i << " / "
                          << ADDC_GBTx_ConfigurationData.size());
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size); usleep(10000);
        }
    }
    ERS_DEBUG(1, " -> done");

    // Reset ARTs
    ERS_DEBUG(1, "ART reset");
    for (auto art: addc.getARTs()) {
        auto name = sca_addr + ".gpio." + art.getName();
        ERS_DEBUG(1, "ART reset: " << name);
        sendGPIO(opc_ip, name + "Rstn",  0); usleep(10000); // reset cfg
        sendGPIO(opc_ip, name + "Rstn",  1); usleep(10000);
        sendGPIO(opc_ip, name + "SRstn", 0); usleep(10000); // reset i2c
        sendGPIO(opc_ip, name + "SRstn", 1); usleep(10000);
        sendGPIO(opc_ip, name + "CRstn", 0); usleep(10000); // reset core
        sendGPIO(opc_ip, name + "CRstn", 1); usleep(10000);
    }
    ERS_DEBUG(1, " -> done");

    // art common config
    ERS_DEBUG(1, "ART common config");
    for (auto art: addc.getARTs()) {
        for (auto tup: {std::make_pair("Core", art.core),
                        std::make_pair("Ps",   art.ps)}) {
            auto name = sca_addr + "." + art.getName() + tup.first + "." + art.getName() + tup.first;
            auto addr_bitstr = tup.second.getBitstreamMap();
            ERS_DEBUG(1, "ART common config " << name);
            for (auto ab : addr_bitstr) {
                art_data[0] = static_cast<uint8_t>( std::stoi(ab.first) );
                art_data[1] = static_cast<uint8_t>( std::stoi(ab.second, nullptr, 2) );
                sendI2cRaw(opc_ip, name, art_data, art_size);
            }
        }
    }
    ERS_DEBUG(1, " -> done");

    // Mask ARTs
    ERS_DEBUG(1, "ART mask");
    for (auto art: addc.getARTs()) {
        auto name = sca_addr + "." + art.getName() + "Core" + "." + art.getName() + "Core";
        for (auto reg: ARTCoreregisters) {
            art_data[0] = reg;
            art_data[1] = 0xFF;
            sendI2cRaw(opc_ip, name, art_data, art_size);
        }
    }
    ERS_DEBUG(1, " -> done");

    // Train GBTx
    ERS_DEBUG(1, "Train GBTx");
    for (auto art: addc.getARTs()) {

        bool train;
        auto core = sca_addr + \
            ".art"  + std::to_string(art.index()) + "Core" + \
            ".art"  + std::to_string(art.index()) + "Core";
        auto gbtx = sca_addr + \
            ".gbtx" + std::to_string(art.index()) + \
            ".gbtx" + std::to_string(art.index());

        // ART pattern mode
        ERS_DEBUG(1, "ART pattern mode");
        for (uint i=0; i<ARTregisters.size(); i++) {
            art_data[0] = ARTregisters[i];
            art_data[1] = ARTregistervalues[i];
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
        for (uint i=0; i<GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        ERS_DEBUG(1, "-> done");

        // Pause
        usleep(1000000);

        // Disable GBTx eport training
        ERS_DEBUG(1, "GBTx eport disable");
        train = 0;
        for (uint i=0; i<GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
            gbtx_data[2] = train ? 0xff : 0x00;
            sendI2cRaw(opc_ip, gbtx, gbtx_data, gbtx_size);
        }
        ERS_DEBUG(1, "-> done");

        // ART default mode
        ERS_DEBUG(1, "ART default mode");
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
        ERS_DEBUG(1, "-> done");

    }
    ERS_DEBUG(1, " -> done (Train GBTx)");

    // Failsafe mode
    ERS_DEBUG(1, "ART flag mode (failsafe or no)");
    for (auto art: addc.getARTs()) {
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
    ERS_DEBUG(1, "-> done");
    ERS_LOG(addc.getAddress() << " Configuration done.");
}

void nsw::ConfigSender::alignAddcGbtxTp(const nsw::ADDCConfig& addc) {

    // the TP register
    std::string regAddr = "0x02";
    auto regAddrVec = nsw::hexStringToByteVector(regAddr, 4, true);

    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0};
    size_t n_attempts = 0;
    size_t max_attempts = 2;

    // align each ART
    for (auto art: addc.getARTs()){

        // allow N failed attempts
        while (n_attempts < max_attempts) {

            ERS_DEBUG(1, addc.getAddress() << "/" << art.getName()
                      << " GBTx phase alignment attempt " << n_attempts);
            bool success = 0;

            for (uint phase = 0; phase < art.NPhase(); phase++) {

                // addc phase
                gbtx_data[1] = 0;
                gbtx_data[0] = 8;
                gbtx_data[2] = phase;
                sendI2cRaw(addc.getOpcServerIp(), addc.getAddress() + "." + art.getNameGbtx(), gbtx_data, gbtx_size);
                usleep(100000);

                // TP response
                auto outdata = readI2cAtAddress(art.getOpcServerIp_TP(), art.getOpcNodeId_TP(), regAddrVec.data(), regAddrVec.size(), 4);
                usleep(100000);

                // debug
                ERS_DEBUG(1, addc.getAddress() << "/" << art.getName()
                          << " GBTx phase = " << art.PhaseToString(phase)
                          << " -> " << art.getOpcNodeId_TP()
                          << " = " << nsw::vectorToBitString(outdata));

                // success?
                if (art.IsAlignedWithTP(outdata)) {

                    // check subsequent phases
                    std::vector<uint> good_phases = {phase};
                    for (uint next = 1; phase+next < art.NPhase(); next++) {
                        gbtx_data[2] = phase+next;
                        sendI2cRaw(addc.getOpcServerIp(), addc.getAddress() + "." + art.getNameGbtx(), gbtx_data, gbtx_size);
                        auto nextdata = readI2cAtAddress(art.getOpcServerIp_TP(), art.getOpcNodeId_TP(), regAddrVec.data(), regAddrVec.size(), 4);
                        usleep(100000);
                        ERS_DEBUG(1, addc.getAddress() << "/" << art.getName()
                                  << " GBTx phase = " << art.PhaseToString(phase+next)
                                  << " -> " << art.getOpcNodeId_TP()
                                  << " = " << nsw::vectorToBitString(nextdata));
                        if (art.IsAlignedWithTP(nextdata))
                            good_phases.push_back(phase+next);
                        else
                            break;
                    }

                    // choose which phase
                    uint chosen_phase = 0;
                    std::string phase_position = art.TP_GBTxAlignmentPhase();
                    if (phase_position == "first")
                        chosen_phase = good_phases.front();
                    else if (phase_position == "middle")
                        chosen_phase = good_phases[ good_phases.size()/2 ];
                    else if (phase_position == "last")
                        chosen_phase = good_phases.back();
                    else
                        throw std::runtime_error("Need ART phase to be first, middle, or last; got " + phase_position);

                    // apply choice
                    gbtx_data[2] = chosen_phase;
                    sendI2cRaw(addc.getOpcServerIp(), addc.getAddress() + "." + art.getNameGbtx(), gbtx_data, gbtx_size);
                    ERS_LOG(addc.getAddress() << "/" << art.getName() << " Aligned! Chosen phase: " << std::to_string(chosen_phase));
                    success = 1;
                    break;
                }
            }

            // try again?
            if (success)
                break;
            if (n_attempts == max_attempts-1) {
                auto msg = addc.getAddress() + "/" + art.getName() + " failed TP alignment! Crashing.";
                ERS_LOG(msg);
                throw std::runtime_error(msg);;
            }
            n_attempts++;
        }
    }
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

void nsw::ConfigSender::sendPadTriggerSCAConfig(const nsw::PadTriggerSCAConfig& obj) {

    // basics
    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();
    std::vector<std::string> repeaters = {"1", "2", "3", "4", "5", "6"};
    std::vector<std::string> vttxs = {"1", "2"};

    // I2C
    size_t address_size_repeater = 1;
    size_t data_size_repeater    = 2;
    uint8_t address_repeater[]   = {0x0};
    uint8_t data_data_repeater[] = {0x00, 0x2F};

    // 0.0: Repeater GPIO
    std::cout << "Repeater GPIO. Writing 1" << std::endl;
    for (auto rep: repeaters) {
        sendGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep, 1);
        usleep(100000);
    }

    // 0.1: Read them
    for (auto rep: repeaters) {
        std::cout << " Readback " << rep << ": " << readGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep) << std::endl;
        usleep(100000);
    }
    std::cout << std::endl;

    // 1.0 Repeater I2C
    std::cout << "Repeater I2C. Writing " << std::hex << unsigned(data_data_repeater[1]) << std::dec << std::endl;
    for (auto rep: repeaters) {
        std::string node = sca_addr + ".repeaterChip" + rep + ".chip" + rep;
        sendI2cRaw(opc_ip, node, data_data_repeater, data_size_repeater);
        usleep(100000);
    }

    // 1.1 Read them
    for (auto rep: repeaters) {
        std::string node = sca_addr + ".repeaterChip" + rep + ".chip" + rep;
        auto val = readI2cAtAddress(opc_ip, node, address_repeater, address_size_repeater);
        std::cout << " Readback " << rep << ": " << unsigned(val[0]) << std::endl;
        usleep(100000);
    }
    std::cout << std::endl;

    // VTTX
    size_t address_size = 1;
    size_t data_size    = 2;
    uint8_t address[]   = {0x0};
    uint8_t data_data[] = {0x0, 0xC7};

    // 2.0 VTTX
    std::cout << "VTTx I2C: Writing " << std::hex << unsigned(data_data[1]) << std::dec << std::endl;
    for (auto vttx: vttxs) {
        std::string node = sca_addr + ".vttx" + vttx + ".chipVTT" + vttx;
        sendI2cRaw(opc_ip, node, data_data, data_size);
        usleep(100000);
    }

    // 2.1 Read them
    for (auto vttx: vttxs) {
        std::string node = sca_addr + ".vttx" + vttx + ".chipVTT" + vttx;
        auto val = readI2cAtAddress(opc_ip, node, address, address_size);
        std::cout << " Readback " << vttx << ": " << std::hex << unsigned(val[0]) << std::dec << std::endl;
        usleep(100000);
    }

}

void nsw::ConfigSender::sendRouterConfig(const nsw::RouterConfig& obj) {

    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();

    // Set Router control mode to SCA mode: Line 17 in excel
    auto ctrlMod0 = sca_addr + ".gpio.ctrlMod0";
    auto ctrlMod1 = sca_addr + ".gpio.ctrlMod1";
    sendGPIO(opc_ip, ctrlMod0, 0);
    sendGPIO(opc_ip, ctrlMod1, 0);
    std::cout << std::left << std::setw(30) << ctrlMod0 << " " << readGPIO(opc_ip, ctrlMod0) << std::endl;
    std::cout << std::left << std::setw(30) << ctrlMod1 << " " << readGPIO(opc_ip, ctrlMod1) << std::endl;

    // Send Soft_RST: Line 11 in excel
    auto soft_reset = sca_addr + ".gpio.softReset";
    sendGPIO(opc_ip, soft_reset, 1); std::cout << std::left << std::setw(30) << soft_reset << " " << readGPIO(opc_ip, soft_reset) << std::endl;
    sendGPIO(opc_ip, soft_reset, 0); std::cout << std::left << std::setw(30) << soft_reset << " " << readGPIO(opc_ip, soft_reset) << std::endl;

    // Read SCA IO status back: Line 6 & 8 in excel
    // (only need to match with star mark bits)
    std::vector< std::pair<std::string, bool> > check = { {"fpgaConfigOK",   1},
                                                          {"mmcmBotLock",    1},
                                                          {"fpgaInit",       1},
                                                          {"rxClkReady",     1},
                                                          {"txClkReady",     1},
                                                          {"cpllTopLock",    1},
                                                          {"cpllBotLock",    1},
                                                          {"mmcmTopLock",    1},
                                                          {"semFatalError",  0},
                                                          {"masterChannel0", 1} };
    for (auto kv: check) {
        auto bit = sca_addr + ".gpio." + kv.first;
        bool exp = kv.second;
        bool obs = readGPIO(opc_ip, bit);
        bool yay = obs==exp;
        std::cout << std::left << std::setw(30) << bit << " ::"
                  << " Expected = " << exp
                  << " Observed = " << obs
                  << " -> " << (yay ? "Good" : "Bad")
                  << std::endl;
    }

    // Reset cout
    std::cout << std::setw(0);

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

