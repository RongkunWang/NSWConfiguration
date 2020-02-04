// Sample program to read VMM capture status through GPIO BitBanger 

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

    std::string description = "This program reads VMM capture status of any VMM-frontend board";

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

    std::cout << "\n";

    int vmmCaptureAddressInitial;

    for (auto & feb : frontend_configs) {
    auto opc_ip = feb.getOpcServerIp();

    std::cout << feb.getAddress() <<std::endl;

    vmmCaptureAddressInitial=32;
    	for(int vmm_id=0; vmm_id<=7; vmm_id++)
    	{
	      auto vmm_capture_status=cs.readBackRoc(opc_ip,feb.getAddress()+".gpio.bitBanger",17,18,(uint8_t)vmmCaptureAddressInitial++,2);
   		  std::cout <<"VMM "<<+vmm_id<<": " <<std::bitset<8>(unsigned(vmm_capture_status)).to_string() <<std::endl;
    	}
    std::cout << "\n";


   
    }



}

