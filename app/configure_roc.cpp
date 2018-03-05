// Sample program to read configuration from json and send to FE

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    bool configure_vmm;
    std::string config_filename;
    po::options_description desc("This program configures ROC with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path");

    po::options_description vmm("VMM Options");
    vmm.add_options()
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure also all the VMMs on the ROC(Default: False)");

    // Declare an options description instance which will include all the options
    po::options_description all("Allowed options");
    all.add(desc).add(vmm);

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, all), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << all << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    // ROC Config
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);

    nsw::ConfigSender cs;

    // Send all ROC config
    cs.sendRocConfig(roc0);

    if (configure_vmm) {
        // Inverse VMM enable to get VMM into config mode
        uint8_t data[] = {0xff};
        size_t size = 1;
        auto opc_ip = roc0.getOpcServerIp();
        auto sca_roc_address_analog = roc0.getAddress() + "." + roc0.analog.getName();
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);

        std::vector<std::string> vmmids = {"0", "1", "2", "3", "4", "5", "6", "7"};
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