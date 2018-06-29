// Sample program to read configuration from json and send to MMFE8

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/MMFE8Config.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    bool configure_vmm;
    bool configure_roc;
    bool create_pulses;
    bool reset_roc;
    std::string config_filename;
    po::options_description desc("This program configures ROC/VMM with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure also all the VMMs on the ROC(Default: False)")
        ("configure-roc,r", po::bool_switch(&configure_roc)->default_value(false),
        "Configure the ROC(Default: False)")
        ("create-pulses,p", po::bool_switch(&create_pulses)->default_value(false),
        "Create 10 test pulses in ROC by modifying TPInv register(Default: False)")
        ("reset,R", po::bool_switch(&reset_roc)->default_value(false),
        "Reset the ROC via SCA. This option can't be used with -r or -v");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if ((!configure_roc && !configure_vmm) && !reset_roc) {
        std::cout << "Please chose at least one of -r, -v or -R command line options to configure ROC/VMM." << "\n";
        std::cout << desc << "\n";
        return 1;
    }

    if ((configure_roc || configure_vmm) && reset_roc) {
        std::cout << "Please chose either -R or (-v,-r) options\n";
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    // ROC Config
    auto mmfe8config = reader1.readConfig("integration_setup.MMFE8-0001");
    nsw::MMFE8Config mmfe8(mmfe8config);
    write_json(std::cout, mmfe8config);

    /*

    nsw::ConfigSender cs;

    // cs.sendMmfe8Config(mmfe8);  // Sends all VMM/ROC configuration

    if (reset_roc) {
        std::cout << "Only resetting ROC" << std::endl;
        auto opc_ip = mmfe8.getOpcServerIp();
        cs.sendGPIO(opc_ip, mmfe8.getAddress() + ".gpio.rocCoreResetN", 0);
        sleep(1);
        cs.sendGPIO(opc_ip, mmfe8.getAddress() + ".gpio.rocCoreResetN", 1);
        return 0;
    }

    // Send all ROC config
    if (configure_roc) {
        // mmfe8.dump();
        cs.sendRocConfig(mmfe8);
    }

    if (configure_vmm) {

        // cs.sendVmmConfig(mmfe8); // Sends configuration to all vmm
                                    // Equivalent to the block below

        // Inverse VMM enable to get VMM into config mode
        std::vector<uint8_t> data = {0xff};
        auto opc_ip = mmfe8.getOpcServerIp();
        auto sca_roc_address_analog = mmfe8.getAddress() + "." + mmfe8.analog.getName();
        cs.sendI2c(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data);

        for (auto vmm : mmfe8.getVmms()) {
            nsw::VMMConfig vmm(vmmconfig);
            cs.sendVmmConfig(vmm);
        }

        // sleep(1);

        // Set back the register
        data[0] = {static_cast<uint8_t>(0x0)};
        cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);
    }

    if (create_pulses) {
        auto opc_ip = mmfe8.getOpcServerIp();
        auto sca_roc_address_analog = mmfe8.getAddress() + "." + mmfe8.analog.getName();
        uint8_t data[] = {0};
        for (int i = 0; i < 10; i++) {
            std::cout << "Creating 10 test pulse" << std::endl;
            data[0] = 0xff;
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg124vmmTpInv", data, 1);
            // sleep(1);

            data[0] = 0x0;
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg124vmmTpInv", data, 1);
            // sleep(1);
        }
    }
    */

    return 0;
}
