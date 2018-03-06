// Sample program to check a gpio

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    bool configure_vmm;
    bool configure_roc;
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
        "Configure the ROC(Default: False)");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (!configure_roc && !configure_vmm) {
        std::cout << "Please chose at least one of -r and -v command line options to configure ROC/VMM." << "\n";
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
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);

    auto opc_ip = roc0.getOpcServerIp();
    auto sca_address = roc0.getAddress();

    nsw::ConfigSender cs;


    bool rocError = 0;
    bool rocError_prev = 0;

    while (true) {
        rocError = cs.readGPIO(opc_ip, sca_address + ".gpio.rocError");
        if (rocError!= rocError_prev) {
            const time_t ctt = time(0);
            char buf[100];
            ctime_r(&ctt, buf);
            std::cout << buf;
            if (rocError) {
               std::cout << " -- ERROR! ERROR! -- ";
            }
            std::cout << "rocError value changed: " << rocError << std::endl;
            rocError_prev = rocError;
        }
    }

    return 0;
}
