// Sample program to read ROC register values through GPIO BitBanger

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {

    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program reads ROC registers of any frontend board";

    std::string config_filename;
    std::string fe_name;
    int registerAddress;

    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("registerAddress,a", po::value<int>(&registerAddress)->
        default_value(0),
        "The register address of the ROC to read .\n"
        "If this option is left empty, the default adress 0 will be read (ROC ID).")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value(""),
        "The name of frontend to read ROC register.\n"
        "If this option is left empty, all front end elements in the config file will be scanned.");


    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    try {
      auto config1 = reader1.readConfig();
    } catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::set<std::string> frontend_names;
    if (fe_name != "") {
      frontend_names.emplace(fe_name);
    } else {  // If no name is given, find all elements
      frontend_names = reader1.getAllElementNames();
    }

    std::vector<nsw::FEBConfig> frontend_configs;

    std::cout << "\nFollowing front ends will be readed:\n";
    std::cout <<   "========================================\n";
    for (auto & name : frontend_names) {
      try {
        frontend_configs.emplace_back(reader1.readConfig(name));
        std::cout << name << std::endl;
      } catch (std::exception & e) {
        std::cout << name << " - ERROR: Skipping this FE!"
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
      }
      // frontend_configs.back().dump();
    }

    std::cout << "\n";

    nsw::ConfigSender cs;

    std::cout << "***** Reading ROC register address: " << registerAddress << std::endl;
    std::cout << "\n";

    for (auto & feb : frontend_configs) {
    auto opc_ip = feb.getOpcServerIp();

    auto roc_address_value = cs.readBackRoc(opc_ip, feb.getAddress()+".gpio.bitBanger", 17, 18, (uint8_t)registerAddress, 2);

    std::cout << feb.getAddress() << "\t" << unsigned(roc_address_value) << "(dec)" << " | 0x" 
    << std::hex << unsigned(roc_address_value) << "(hex)" << " | " << std::bitset<8>(unsigned(roc_address_value)).to_string() << "(bin)" << std::endl;
    }
}

