// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/TDSConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

#include "boost/program_options.hpp"
#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    bool configure_vmm;
    bool configure_roc;
    bool configure_tds;
    std::string config_filename;
    po::options_description desc("This program configures TDS/ROC/VMM with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "sTGC_config.json"),
        "Configuration file path")
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure all the VMMs(Default: False)")
        ("configure-roc,r", po::bool_switch(&configure_roc)->default_value(false),
        "Configure the ROC(Default: True)")
        ("configure-tds,t", po::bool_switch(&configure_tds)->default_value(false),
        "Configure the TDS(Default: True)");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (!configure_roc && !configure_vmm && !configure_tds) {
        std::cout << "Please chose at least one of -r, -v or -t command line options to configure ROC/VMM/TDS" << "\n";
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigSender cs;

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    // Configuration with Opc parameters
    auto tdsconfig0 = reader1.readConfig("A01.TDS_L01_M01");
    std::cout << "TDS Config: "  << std::endl;
    nsw::TDSConfig tds(tdsconfig0);
    tds.dump();

    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);

    auto opc_ip = roc0.getOpcServerIp();
    std::vector<uint8_t> data;
    data.reserve(16);
    // Send all ROC config
    if (configure_roc) {
        std::cout << "configuring roc" << std::endl;
        cs.sendRocConfig(roc0);
    }

    if (configure_tds) {
        std::cout << "Sending TDS config" << std::endl;
        cs.sendTdsConfig(tds);

        for (auto tds_i2c_address : tds.i2c.getAddresses()) {
            auto address_to_read = nsw::stripReadonly(tds_i2c_address);
            auto dataread = cs.readI2c(opc_ip, tds.getAddress() + ".tds." + address_to_read);
            std::cout << "Read back: " + tds_i2c_address << std::endl;
            for (auto val : dataread) {
                std::cout << std::hex << static_cast<uint32_t>(val) << ", ";
            }
            std::cout << std::dec << "\n";
            tds.i2c.decodeVector(tds_i2c_address, dataread);
        }

        // Example to read single tds i2c slave

        std::string address_to_read = "register0";
        auto dataread = cs.readI2c(opc_ip, tds.getAddress() + ".tds." + address_to_read);

        for (auto val : dataread) {
            std::cout << std::hex << static_cast<uint32_t>(val) << ", ";
        }
        std::cout << std::dec << "\n";
        tds.i2c.decodeVector(address_to_read, dataread);

        address_to_read = "register14";
        dataread = cs.readI2c(opc_ip, tds.getAddress() + ".tds." + address_to_read);

        for (auto val : dataread) {
            std::cout << std::hex << static_cast<uint32_t>(val) << ", ";
        }
        std::cout << std::dec << "\n";
        tds.i2c.decodeVector(address_to_read + "_READONLY", dataread);

    }

    if (configure_vmm) {
        // Inverse VMM enable to get VMM into config mode
        uint8_t data[] = {0xff};
        size_t size = 1;
        auto sca_roc_address_analog = roc0.getAddress() + "." + roc0.analog.getName();
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);

        std::vector<std::string> vmmids = {"0", "1", "2"}; // , "3", "4", "5", "6", "7"};  // TODO(cyildiz): Up to 3?
        for (auto vmmid : vmmids) {
            auto vmmconfig = reader1.readConfig("A01.VMM_L01_M01_0" + vmmid);

            nsw::VMMConfig vmm(vmmconfig);
            // auto vec = vmm.getByteVector();  /// Create a vector of bytes
            // std::cout << nsw::vectorToHexString(vec) << std::endl;
            
            // cs.sendVmmConfig(vmm);

            std::vector<uint8_t> vmmdata = {0x00,0x3b,0x63,0x86,0x3e,0x8e,0x03,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x3d,0xe4,0x10,0x06,0x00,0x69,0x17,0xc0,0x11,0x20,0x21};
            cs.sendSpiRaw(vmm.getOpcServerIp(), vmm.getAddress(), vmmdata.data(), vmmdata.size());
        }

        // sleep(1);

        // Set back the register
        data[0] = {static_cast<uint8_t>(0x0)};
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);
    }

    // GPIO example:
    // cs.sendGPIO(opc_ip, roc0.getAddress() + ".gpio.rocCoreResetN", 0);
    cs.sendGPIO(opc_ip, roc0.getAddress() + ".gpio.rocCoreResetN", 1);

    std::cout << "Readback gpio rocCoreResetN: " << cs.readGPIO(opc_ip, roc0.getAddress() + ".gpio.rocCoreResetN") << std::endl;

    return 0;
}
