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
    po::options_description desc("This program configures TDS/ROC with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "sTGC_config.json"),
        "Configuration file path")
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure all the VMMs(Default: False)")
        ("configure-roc,r", po::bool_switch(&configure_roc)->default_value(true),
        "Configure the ROC(Default: True)")
        ("configure-tds,t", po::bool_switch(&configure_tds)->default_value(true),
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

    std::vector<uint8_t> readback {0x0, 0xa, 0xf8, 0x9f};
    tds.tds.decodeVector("register0", readback);

    return 0;

    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);

    auto opc_ip = roc0.getOpcServerIp();
    std::string sca_tds_address =  roc0.getAddress() + ".tds";
    std::vector<uint8_t> data;
    data.reserve(16);
    // Send all ROC config
    if (configure_roc) {
        cs.sendRocConfig(roc0);
    }


    if (configure_tds) {
        std::cout << "Sending TDS config" << std::endl;
        data = {0x0, 0x0a, 0xf8, 0x9f};
        cs.sendI2c(opc_ip, sca_tds_address + ".register0", data);
        data = {0x0, 0x0};
        cs.sendI2c(opc_ip, sca_tds_address + ".register1", data);
        data = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        cs.sendI2c(opc_ip, sca_tds_address + ".register2", data);
        data = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        cs.sendI2c(opc_ip, sca_tds_address + ".register3", data);
        data = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        cs.sendI2c(opc_ip, sca_tds_address + ".register4", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register5", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register6", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register7", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register8", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register9", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register10", data);
        data = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};
        cs.sendI2c(opc_ip, sca_tds_address + ".register11", data);
        data = {0xff, 0xf0, 0x00, 0x00};
        cs.sendI2c(opc_ip, sca_tds_address + ".register12", data);


        for (int i = 0; i < 13; i++) {
            auto dataread = cs.readI2c(opc_ip, sca_tds_address + ".register" + std::to_string(i));
            std::cout << "Read back: register" + std::to_string(i) << std::endl;
            for (auto val : dataread) {
                std::cout << static_cast<uint32_t>(val) << ", ";
            }
            std::cout << "\n";
        }


        // Remaining 3 addresses are readonly
    }

    // TODO(cyildiz): Read back TDS I2c

    if (configure_vmm) {
        // Inverse VMM enable to get VMM into config mode
        uint8_t data[] = {0xff};
        size_t size = 1;
        auto sca_roc_address_analog = roc0.getAddress() + "." + roc0.analog.getName();
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);

        std::vector<std::string> vmmids = {"0", "1", "2", "3", "4", "5", "6", "7"};  // TODO(cyildiz): Up to 3?
        for (auto vmmid : vmmids) {
            auto vmmconfig = reader1.readConfig("A01.VMM_L01_M01_0" + vmmid);
            nsw::VMMConfig vmm(vmmconfig);
            // auto vec = vmm.getByteVector();  /// Create a vector of bytes
            cs.sendVmmConfig(vmm);

            // for (auto el : vec) {
            //    std::cout << "0x" << std::hex << unsigned(el) << ", ";
            //}
            // std::cout << std::dec << std::endl;;
        }

        // sleep(1);

        // Set back the register
        data[0] = {static_cast<uint8_t>(0x0)};
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);
    }

    return 0;
}
