// Sample program to read multiple ADC values from a channel of VMM

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

    std::string description = "This program reads SCA Info of any frontend board";

    std::string config_filename;
    std::string fe_name;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value(""),
        "The name of frontend to read SCA ID.\n"
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
    uint8_t roc_address_value;

    for (auto & feb : frontend_configs) {
    auto opc_ip = feb.getOpcServerIp();

    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocPllResetN", 0);
    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocSResetN", 0);
    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 0);
    usleep(1);
    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocPllResetN", 1);
    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocSResetN", 1);
    cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 1);


	roc_address_value=cs.readBackRoc(opc_ip,feb.getAddress()+".gpio.bitBanger",17,18,0,2);
	
    std::cout << feb.getAddress() << "\t"<< unsigned(roc_address_value)<<"(dec)" << " | "<< std::hex << unsigned(roc_address_value) << "(hex)" <<std::endl;

    }



}

