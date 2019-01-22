// Sample program to check a gpio

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    bool configure_roc;
    std::string config_filename;
    po::options_description desc("This program configures ROC/VMM with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    // ROC Config
    auto febconfig0 = reader1.readConfig("MMFE8-0001");
    nsw::FEBConfig feb(febconfig0);

    auto opc_ip = feb.getOpcServerIp();
    auto sca_address = feb.getAddress();

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
