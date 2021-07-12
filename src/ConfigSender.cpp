#include "NSWConfiguration/ConfigSender.h"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

#include "ers/ers.h"

#include <thread>
#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

nsw::ConfigSender::ConfigSender() {
}

void nsw::ConfigSender::addOpcClientIfNew(const std::string& opcserver_ipport) {
    // std::map doesn't allow duplicates anyway, consider removing this check?
    if (m_clients.find(opcserver_ipport) == m_clients.end()) {
        m_clients.emplace(opcserver_ipport, std::make_unique<nsw::OpcClient>(opcserver_ipport));
    }
}

void nsw::ConfigSender::sendSpiRaw(const std::string& opcserver_ipport, const std::string& node, uint8_t* data, size_t data_size) {
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
    readBackRoc(opcserver_ipport, fullNode, nsw::roc::digital::SCL_LINE_PIN, nsw::roc::digital::SDA_LINE_PIN, registerAddress, delay);
    return readBackRoc(opcserver_ipport, fullNode, nsw::roc::digital::SCL_LINE_PIN, nsw::roc::digital::SDA_LINE_PIN, registerAddress, delay);
}

uint8_t nsw::ConfigSender::readBackRocAnalog(const std::string& opcserver_ipport, const std::string& node, uint8_t registerAddress) {
    // FIXME: Why do we have to call it twice?
    const unsigned int delay{2};
    const auto fullNode = node + ".gpio.bitBanger";
    readBackRoc(opcserver_ipport, fullNode, nsw::roc::analog::SCL_LINE_PIN, nsw::roc::analog::SDA_LINE_PIN, registerAddress, delay);
    return readBackRoc(opcserver_ipport, fullNode, nsw::roc::analog::SCL_LINE_PIN, nsw::roc::analog::SDA_LINE_PIN, registerAddress, delay);
}

void nsw::ConfigSender::sendI2cRaw(const std::string opcserver_ipport, const std::string node, uint8_t* data, size_t data_size) {
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
    const std::string& node, uint8_t* address, size_t address_size, size_t number_of_bytes) {
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
        auto data = vmm.getByteVector();
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

void nsw::ConfigSender::sendVmmConfigSingle(const nsw::FEBConfig& feb, size_t vmm_id) {
    // Set Vmm Configuration Enable
    std::vector<uint8_t> data = {0xff};
    auto opc_ip = feb.getOpcServerIp();
    auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);

    auto vmm = feb.getVmms()[vmm_id];
    setVMMConfigurationStatusInfoDCS(feb, vmm);
    auto vmmdata = vmm.getByteVector();
    ERS_DEBUG(1, "Sending I2c configuration to " << feb.getAddress() << ".spi." << vmm.getName());
    sendSpiRaw(opc_ip, feb.getAddress() + ".spi." + vmm.getName() , vmmdata.data(), vmmdata.size());
    ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));

    // Set Vmm Acquisition Enable
    data = {0x0};
    sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);
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
            datas.push_back(ADDC_GBTx_ConfigurationData[i]);
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
        for (uint i=0; i < GBTx_eport_registers.size(); i++) {
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
        for (uint i=0; i < GBTx_eport_registers.size(); i++) {
            gbtx_data[1] = ((uint8_t) ((GBTx_eport_registers[i]) >> 8));
            gbtx_data[0] = ((uint8_t) ((GBTx_eport_registers[i]) & 0xff));
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

void nsw::ConfigSender::alignAddcGbtxTp(const std::map<std::string, nsw::ADDCConfig> & addcs_map) {
    std::vector<nsw::ADDCConfig> addcs = {};
    for (auto & obj: addcs_map)
        addcs.push_back(obj.second);
    alignAddcGbtxTp(addcs);
}

void nsw::ConfigSender::alignAddcGbtxTp(std::vector<nsw::ADDCConfig> & addcs) {

    // check if any ADDCs want to be aligned
    bool go = 0;
    for (auto & addc : addcs)
        for (auto art: addc.getARTs())
            if (!art.TP_GBTxAlignmentSkip())
                go = 1;
    if (!go)
        return;

    // collect all TPs from the ARTs
    std::vector< nsw::TPConfig > tps;
    for (auto & addc : addcs) {
        for (auto art : addc.getARTs()) {
            bool exists = false;
            for (const auto & tp: tps) {
                if (art.getOpcServerIp_TP() == tp.getOpcServerIp() && art.getOpcNodeId_TP() == tp.getAddress()) {
                    exists = true;
                }
            }
            if (!exists) {
                ptree config;
                config.put("OpcServerIp", art.getOpcServerIp_TP());
                config.put("OpcNodeId",   art.getOpcNodeId_TP());
                nsw::TPConfig tp(config);
                tps.push_back(tp);
            }
        }
    }
    for (auto tp : tps)
        ERS_LOG("Found TP for alignment: " << tp.getOpcServerIp() << "/" << tp.getAddress());

    // loop over TPs
    for (auto tp : tps) {

        // check alignment
        auto outdata = readSCAXRegister(tp, nsw::mmtp::REG_FIBER_ALIGNMENT);
        ERS_LOG(tp.getOpcServerIp() << "/" << tp.getAddress() << " Found " << nsw::vectorToBitString(outdata, true));

        // TP collect data from the ARTs
        // to decide if stability is good
        size_t n_resets = 0;
        while (true) {
            if (n_resets > nsw::mmtp::FIBER_ALIGN_ATTEMPTS)
                throw std::runtime_error("Failed to stabilize input to " + tp.getAddress() + ". Crashing");

            // should sleep before trying the first time
            // in case the fiber communication hasn't settled
            usleep(nsw::mmtp::FIBER_ALIGN_SLEEP);

            // read for glitches
            auto glitches = std::vector<int>(nsw::mmtp::NUM_FIBERS);
            for (size_t i_read = 0; i_read < nsw::mmtp::FIBER_ALIGN_N_READS; i_read++) {
                auto outdata = readSCAXRegister(tp, nsw::mmtp::REG_FIBER_ALIGNMENT);
                for (auto & addc : addcs)
                    for (auto art : addc.getARTs())
                        if (art.IsMyTP(tp.getOpcServerIp(), tp.getAddress()) && !art.TP_GBTxAlignmentSkip())
                            glitches[art.TP_GBTxAlignmentBit()] += (art.IsAlignedWithTP(outdata) ? 0 : 1);
            }

            // announce
            for (auto & addc : addcs) {
                for (auto art : addc.getARTs()) {
                    if (art.IsMyTP(tp.getOpcServerIp(), tp.getAddress()) && !art.TP_GBTxAlignmentSkip()) {
                        std::stringstream msg;
                        msg << addc.getAddress() << "." << art.getName() << " "
                            << glitches[art.TP_GBTxAlignmentBit()] << " glitches"
                            << " (" << art.TP_GBTxAlignmentBit() << ")";
                        if (glitches[art.TP_GBTxAlignmentBit()] > 0)
                            ERS_INFO(msg.str());
                        else
                            ERS_LOG(msg.str());
                    }
                }
            }

            // build the reset
            uint32_t reset = 0;
            for (size_t ipll = 0; ipll < nsw::mmtp::NUM_FIBERS/nsw::mmtp::NUM_FIBERS_PER_QPLL; ipll++) {
                int total_glitches = 0;
                for (size_t fib = 0; fib < nsw::mmtp::NUM_FIBERS_PER_QPLL; fib++)
                    total_glitches += glitches.at(ipll*nsw::mmtp::NUM_FIBERS_PER_QPLL + fib);
                if (total_glitches > 0)
                    reset += 1 << static_cast<uint32_t>(ipll);
            }
            usleep(1e6);
            ERS_INFO("alignAddcGbtxTp Reset word = " << reset);

            // the moment of truth
            if (reset == 0) {
                ERS_INFO("alignAddcGbtxTp success!");
                break;
            }

            // or, set/unset the reset
            sendSCAXRegister(tp, nsw::mmtp::REG_FIBER_QPLL_RESET, reset);
            usleep(1e6);
            sendSCAXRegister(tp, nsw::mmtp::REG_FIBER_QPLL_RESET, 0x00);

            // protecc
            n_resets++;
        }
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
      {nsw::mmtp::REG_ADDC_EMU_DISABLE, static_cast<uint32_t>(true)},
      {nsw::mmtp::REG_L1A_LATENCY,      static_cast<uint32_t>(tp.ARTWindowCenter())},
      {nsw::mmtp::REG_L1A_WIN_UPPER,    static_cast<uint32_t>(tp.ARTWindowLeft())},
      {nsw::mmtp::REG_L1A_WIN_LOWER,    static_cast<uint32_t>(tp.ARTWindowRight())},
      {nsw::mmtp::REG_L1A_CONTROL,      nsw::mmtp::L1A_RESET_ENABLE},
      {nsw::mmtp::REG_L1A_CONTROL,      nsw::mmtp::L1A_RESET_DISABLE},
      {nsw::mmtp::REG_FIBER_BC_OFFSET,  static_cast<uint32_t>(tp.FiberBCOffset())},
      {nsw::mmtp::REG_INPUT_PHASE,      static_cast<uint32_t>(tp.GlobalInputPhase())},
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

    //
    // Write registers
    //
    for (auto element : list_of_messages) {
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

uint32_t nsw::ConfigSender::readPadTriggerSCAControlRegister(const nsw::PadTriggerSCAConfig & obj) {
    auto ip   = obj.getOpcServerIp();
    auto addr = obj.getAddress() + ".fpga.fpga";
    int i2c_reg_control = 0;

    // address and data
    uint8_t address[]    = {(uint8_t)(i2c_reg_control)};
    size_t  address_size = 1;
    size_t  data_size    = 4;

    // readback
    auto vals = readI2cAtAddress(ip, addr, address, address_size, data_size);
    std::stringstream msg;
    msg << " Readback " << addr << ": ";
    for (auto val : vals)
        msg << std::hex << unsigned(val) << " " << std::dec;
    ERS_INFO(msg.str());

    // format
    uint32_t word = 0;
    word += (uint32_t)(vals[0] << 24);
    word += (uint32_t)(vals[1] << 16);
    word += (uint32_t)(vals[2] <<  8);
    word += (uint32_t)(vals[3] <<  0);

    return word;
}

uint32_t nsw::ConfigSender::readPadTriggerConfigRegister(const nsw::PadTriggerSCAConfig& pt, uint8_t address) {
  std::vector<uint8_t> addr = { address };
  auto data = readI2cAtAddress(pt.getOpcServerIp(), pt.getAddress() + ".fpga.fpga",
                               addr.data(), addr.size(), nsw::NUM_BYTES_IN_WORD32);
  auto data32 = nsw::byteVectorToWord32(data, nsw::padtrigger::SCA_LITTLE_ENDIAN);
  return data32;
}

void nsw::ConfigSender::sendPadTriggerConfigRegister(const nsw::PadTriggerSCAConfig& pt, uint8_t address, uint32_t message, bool quiet) {
  std::vector<uint8_t> payload = { address };
  auto data = nsw::intToByteVector(message, nsw::NUM_BYTES_IN_WORD32, 
                                   nsw::padtrigger::SCA_LITTLE_ENDIAN);
  payload.insert(payload.end(), data.begin(), data.end());
  if (!quiet) {
    ERS_LOG("... writing to pad trigger "
            << pt.getOpcServerIp() << " " << pt.getAddress()
            << ": address, message =  0x" << static_cast<int>(address)
            << " 0x" << std::hex << nsw::byteVectorToWord32(data, nsw::padtrigger::SCA_LITTLE_ENDIAN)
            );
  }
  sendI2cRaw(pt.getOpcServerIp(), pt.getAddress() + ".fpga.fpga",
             payload.data(), payload.size());
}

void nsw::ConfigSender::sendPadTriggerSCAControlRegister(const nsw::PadTriggerSCAConfig & obj, bool write) {
    auto ip   = obj.getOpcServerIp();
    auto addr = obj.getAddress() + ".fpga.fpga";
    int i2c_reg_control = 0;

    // address and data
    uint8_t address[]    = {(uint8_t)(i2c_reg_control)};
    size_t  address_size = 1;
    size_t  data_size    = 4;

    // write
    if (write) {
        uint32_t i2c_val_32 = (uint32_t)(obj.ControlRegister());
        uint8_t data_data[]  = {address[0],
                                (uint8_t)((i2c_val_32 >> 24) & 0xff),
                                (uint8_t)((i2c_val_32 >> 16) & 0xff),
                                (uint8_t)((i2c_val_32 >>  8) & 0xff),
                                (uint8_t)((i2c_val_32 >>  0) & 0xff)};
        std::stringstream msg;
        msg << " Writing  " << addr
            << " reg "      << i2c_reg_control
            << " val "      << std::hex << obj.ControlRegister()
            << " -> msg = ";
        for (auto val : data_data)
            msg << std::hex << unsigned(val) << " " << std::dec;
        ERS_INFO(msg.str());
        sendI2cRaw(ip, addr, data_data, address_size + data_size);
    }

    // readback
    readPadTriggerSCAControlRegister(obj);
}


void nsw::ConfigSender::sendPadTriggerSCAConfig(const nsw::PadTriggerSCAConfig& obj) {
    // basics
    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();

    // Repeaters
    if (obj.ConfigRepeaters()) {
        std::vector< std::tuple<std::string, uint8_t, uint8_t> > repeater_sequence_of_commands = {
            // ENABLE SMBUS REGISTERS ON ALL REPEATER CHIPS
            {"1", 0x07, 0x01}, {"1", 0x07, 0x11}, {"1", 0x07, 0x21}, {"1", 0x07, 0x31},
            {"2", 0x07, 0x01}, {"2", 0x07, 0x11}, {"2", 0x07, 0x21}, {"2", 0x07, 0x31},
            {"3", 0x07, 0x01}, {"3", 0x07, 0x11}, {"3", 0x07, 0x21}, {"3", 0x07, 0x31},
            {"4", 0x07, 0x01}, {"4", 0x07, 0x11}, {"4", 0x07, 0x21}, {"4", 0x07, 0x31},
            {"5", 0x07, 0x01}, {"5", 0x07, 0x11}, {"5", 0x07, 0x21}, {"5", 0x07, 0x31},
            {"6", 0x07, 0x01}, {"6", 0x07, 0x11}, {"6", 0x07, 0x21}, {"6", 0x07, 0x31},
            // SET REPEATER CHIPS EQUALISER SETTINGS
            {"1", 0x14, 0x03}, {"1", 0x16, 0x03}, {"1", 0x18, 0x03}, {"1", 0x1A, 0x03},
            {"2", 0x14, 0x03}, {"2", 0x16, 0x03}, {"2", 0x18, 0x03}, {"2", 0x1A, 0x03},
            {"3", 0x14, 0x03}, {"3", 0x16, 0x03}, {"3", 0x18, 0x03}, {"3", 0x1A, 0x03},
            {"4", 0x14, 0x03}, {"4", 0x16, 0x03}, {"4", 0x18, 0x03}, {"4", 0x1A, 0x03},
            {"5", 0x14, 0x03}, {"5", 0x16, 0x03}, {"5", 0x18, 0x03}, {"5", 0x1A, 0x03},
            {"6", 0x14, 0x03}, {"6", 0x16, 0x03}, {"6", 0x18, 0x03}, {"6", 0x1A, 0x03},
            // DISABLE SMBUS REGISTERS ON ALL REPEATER CHIPS
            // (not really necessary but it’s a protection against occasional commands sent by mistake)
            {"1", 0x07, 0x00}, {"1", 0x07, 0x10}, {"1", 0x07, 0x20}, {"1", 0x07, 0x30},
            {"2", 0x07, 0x00}, {"2", 0x07, 0x10}, {"2", 0x07, 0x20}, {"2", 0x07, 0x30},
            {"3", 0x07, 0x00}, {"3", 0x07, 0x10}, {"3", 0x07, 0x20}, {"3", 0x07, 0x30},
            {"4", 0x07, 0x00}, {"4", 0x07, 0x10}, {"4", 0x07, 0x20}, {"4", 0x07, 0x30},
            {"5", 0x07, 0x00}, {"5", 0x07, 0x10}, {"5", 0x07, 0x20}, {"5", 0x07, 0x30},
            {"6", 0x07, 0x00}, {"6", 0x07, 0x10}, {"6", 0x07, 0x20}, {"6", 0x07, 0x30},
        };
        for (auto command: repeater_sequence_of_commands) {
            auto [rep, address, value] = command;

            // values
            auto node = sca_addr + ".repeaterChip" + rep + ".repeaterChip" + rep;
            size_t address_size_repeater = 1;
            size_t data_size_repeater    = 2;
            uint8_t address_repeater[]   = {address};
            uint8_t data_data_repeater[] = {address, value};

            // GPIO enable
            sendGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep, 1);

            // Repeater I2C: write
            ERS_INFO(opc_ip << " " << sca_addr << ": Repeater I2C. "
                     << "Writing 0x" << std::hex << unsigned(data_data_repeater[1])
                     << " to address 0x" << unsigned(data_data_repeater[0]));
            sendI2cRaw(opc_ip, node, data_data_repeater, data_size_repeater);
            usleep(10000);

            // Repeater I2C: readback
            auto val = readI2cAtAddress(opc_ip, node, address_repeater, address_size_repeater);
            ERS_INFO(opc_ip << " " << sca_addr << ": Repeater I2C. "
                     << " Readback " << rep << ": 0x" << std::hex << unsigned(val[0]));
            usleep(10000);

            // GPIO disable
            sendGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep, 0);
        }
    } else {
        ERS_INFO("Skipping configuration of repeaters of " << opc_ip << " " << sca_addr);
    }

    // VTTX
    if (obj.ConfigVTTx()) {
        size_t address_size = 1;
        size_t data_size    = 2;
        uint8_t address[]   = {0x0};
        uint8_t data_data[] = {0x0, 0xC7};
        std::vector<std::string> vttxs = {"1", "2"};

        // 2.0 VTTX
        std::cout << "VTTx I2C: Writing " << std::hex << unsigned(data_data[1])
                  << " to address 0x" << unsigned(data_data[0])<< std::dec << std::endl;
        for (auto vttx: vttxs) {
            std::string node = sca_addr + ".vttx" + vttx + ".vttx" + vttx;
            sendI2cRaw(opc_ip, node, data_data, data_size);
            usleep(100000);
        }

        // 2.1 Read them
        for (auto vttx: vttxs) {
            std::string node = sca_addr + ".vttx" + vttx + ".vttx" + vttx;
            auto val = readI2cAtAddress(opc_ip, node, address, address_size);
            std::cout << " Readback " << vttx << ": " << std::hex << unsigned(val[0]) << std::dec << std::endl;
            usleep(100000);
        }
    } else {
        ERS_INFO("Skipping configuration of VTTx of " << opc_ip << " " << sca_addr);
    }

    // Control register
    if (obj.ConfigControlRegister()) {
      sendPadTriggerSCAControlRegister(obj);
    } else {
        ERS_INFO("Skipping configuration of control register " << opc_ip << " " << sca_addr);
    }
}

void nsw::ConfigSender::sendRouterConfig(const nsw::RouterConfig& obj) {
    sendRouterSoftReset(obj);
    sendRouterWaitGPIO(obj);
    sendRouterSetSCAID(obj);
}

void nsw::ConfigSender::sendRouterSoftReset(const nsw::RouterConfig& obj,
                                            std::chrono::seconds reset_hold,
                                            std::chrono::seconds reset_sleep) {

    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();
    ERS_LOG(sca_addr << ": toggling soft reset");

    // Set Router control mode to SCA mode: Line 17 in excel
    auto ctrlMod0 = sca_addr + ".gpio.ctrlMod0";
    auto ctrlMod1 = sca_addr + ".gpio.ctrlMod1";
    sendGPIO(opc_ip, ctrlMod0, 0);
    sendGPIO(opc_ip, ctrlMod1, 0);
    ERS_LOG(ctrlMod0 << " " << readGPIO(opc_ip, ctrlMod0));
    ERS_LOG(ctrlMod1 << " " << readGPIO(opc_ip, ctrlMod1));

    // Send Soft_RST: Line 11 in excel
    auto soft_reset = sca_addr + ".gpio.softReset";
    sendGPIO(opc_ip, soft_reset, 1);
    ERS_LOG(soft_reset << " " << readGPIO(opc_ip, soft_reset));
    std::this_thread::sleep_for(reset_hold);
    sendGPIO(opc_ip, soft_reset, 0);
    ERS_LOG(soft_reset << " " << readGPIO(opc_ip, soft_reset));
    std::this_thread::sleep_for(reset_sleep);
}

void nsw::ConfigSender::sendRouterWaitGPIO(const nsw::RouterConfig& obj) {
    //
    // check GPIOs
    //
    size_t checks = 0;
    while (checks < nsw::router::MAX_GPIO_CHECKS) {
      if (sendRouterCheckGPIO(obj)) {
          return;
      }
      ERS_INFO("Waiting for "
               << obj.getOpcServerIp()
               << "/"
               << obj.getAddress());
      std::this_thread::sleep_for(nsw::router::RESET_PAUSE);
      checks++;
    }

    //
    // alert the user
    //
    if (obj.CrashOnConfigFailure()) {
        const auto msg = obj.getOpcServerIp()
                       + "/"
                       + obj.getAddress()
                       + " Configuration error. Crashing.";
        nsw::RouterConfigIssue issue(ERS_HERE, msg.c_str());
        ers::fatal(issue);
        throw std::runtime_error(msg);
    } else {
        const auto msg = "Giving up on "
                       + obj.getOpcServerIp()
                       + "/"
                       + obj.getAddress();
        ERS_INFO(msg);
    }
}

bool nsw::ConfigSender::sendRouterCheckGPIO(const nsw::RouterConfig& obj) {
    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();
    bool ok = true;

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
    for (auto kv : check) {
        auto bit = sca_addr + ".gpio." + kv.first;
        bool exp = kv.second;
        bool obs = readGPIO(opc_ip, bit);
        bool yay = obs==exp;
        std::stringstream msg;
        msg << std::left << std::setw(34) << bit
            << " ::"
            << " Expected = " << exp
            << " Observed = " << obs
            << " -> " << (yay ? "Good" : "Bad");
        if (yay) {
          ERS_LOG(msg.str());
        } else {
          ERS_INFO(msg.str());
          ok = false;
        }
    }

    return ok;
}

void nsw::ConfigSender::sendRouterSetSCAID(const nsw::RouterConfig& obj) {
    auto opc_ip   = obj.getOpcServerIp();
    auto sca_addr = obj.getAddress();

    // Set ID
    auto scaid = (uint)(obj.id());
    auto id_sector_str = nsw::bitString((uint)(obj.id_sector()), 4);
    auto id_layer_str  = nsw::bitString((uint)(obj.id_layer()),  3);
    auto id_endcap_str = nsw::bitString((uint)(obj.id_endcap()), 1);
    ERS_LOG (sca_addr << ": ID (sector) = 0b" << id_sector_str);
    ERS_LOG (sca_addr << ": ID (layer)  = 0b" << id_layer_str);
    ERS_LOG (sca_addr << ": ID (endcap) = 0b" << id_endcap_str);
    ERS_INFO(sca_addr << ": -> ID"
             << " = 0b" << id_sector_str << id_layer_str << id_endcap_str
             << " = 0x" << std::hex << scaid << std::dec
             << " = "   << scaid);
    for (std::size_t bit = 0; bit < NUM_BITS_IN_BYTE; bit++) {
      bool this_bit = ((scaid >> bit) & 0b1);
      auto gpio = sca_addr + ".gpio.routerId" + std::to_string(bit);
      sendGPIO(opc_ip, gpio, this_bit);
      ERS_LOG(sca_addr << ": Set ID bit " << bit
              << " = " << this_bit
              << " => Readback = " << readGPIO(opc_ip, gpio));
    }
}

std::vector<short unsigned int> nsw::ConfigSender::readAnalogInputConsecutiveSamples(const std::string& opcserver_ipport,
    const std::string& node, size_t n_samples) {
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



int nsw::ConfigSender::readSCAID(FEBConfig& feb) {

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
    const auto configConverter = ConfigConverter(tree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::REGISTER_BASED);
    const auto translatedPtree = configConverter.getFlatRegisterBasedConfig(feb.getRocDigital());
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
