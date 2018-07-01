// Sample program to read configuration from json and send to any front end module

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program configures ROC/VMM/TDS on a front end board";
    description += "The name of the front end will be used to determine how many VMM and TDS the board contains.";

    bool configure_vmm;
    bool configure_roc;
    bool configure_tds;
    bool create_pulses;
    bool reset_roc;
    std::string config_filename;
    std::string fe_name;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value("integration_setup.MMFE8-0001"),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB)")
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure all the VMMs on the FE(Default: False)")
        ("configure-roc,r", po::bool_switch(&configure_roc)->default_value(false),
        "Configure the ROC on the FE(Default: False)")
        ("configure-tds,t", po::bool_switch(&configure_tds)->default_value(false),
        "Configure all the TDSs on the FE(Default: False)")
        ("create-pulses,p", po::bool_switch(&create_pulses)->default_value(false),
        "Create 10 test pulses in ROC by modifying TPInv register(Default: False)")
        ("reset,R", po::bool_switch(&reset_roc)->default_value(false),
        "Reset the ROC via SCA. This option can't be used with -r or -v");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if ((!configure_roc && !configure_vmm && !configure_tds) && !reset_roc) {
        std::cout << "Please chose at least one of -r, -v, -t or -R command line options to configure ROC/VMM/TDS\n";
        std::cout << desc << "\n";
        return 1;
    }

    if ((configure_roc || configure_vmm || configure_tds) && reset_roc) {
        std::cout << "Please chose either -R or (-v,-r, -t) options\n";
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    auto feb_config_tree = reader1.readConfig(fe_name);
    nsw::FEBConfig feb(feb_config_tree);
    feb.dump();

    nsw::ConfigSender cs;

    if (reset_roc) {
        std::cout << "Only resetting ROC" << std::endl;
        auto opc_ip = feb.getOpcServerIp();
        cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 0);
        sleep(1);
        cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 1);
        return 0;
    }

    // Send all ROC config
    if (configure_roc) {
        // feb.dump();
        cs.sendRocConfig(feb);
    }

    if (configure_vmm) {
        cs.sendVmmConfig(feb);  // Sends configuration to all vmm
    }

    if (configure_tds) {
        cs.sendTdsConfig(feb);  // Sends configuration to all tds
    }

    if (create_pulses) {
        auto opc_ip = feb.getOpcServerIp();
        auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
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

    return 0;
}
