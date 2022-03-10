// Program to set Router configuration

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;
    bool read_all_gpio;
    bool no_config;

    po::options_description desc(std::string("Router configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(config_files+"config_json/VS/VS_3routers.json"), "Configuration file path")
        ("name,n",        po::value<std::string>(&board_name)
         ->default_value(""), "Name of desired router (e.g. Router_L0).")
        ("gpio", po::bool_switch()->default_value(false), "Option to read all GPIOs")
        ("no_config", po::bool_switch()->default_value(false), "Option to disable config")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    read_all_gpio = vm["gpio"]     .as<bool>();
    no_config     = vm["no_config"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // make opc manager
    nsw::OpcManager opcManager{};

    // make router objects
    auto board_configs = nsw::ConfigReader::makeObjects<nsw::RouterConfig>
      ("json://" + config_filename, "Router", board_name);

    // announce
    for (auto & config : board_configs) {
        if (no_config)
            continue;
        std::cout << "Found " << config.getAddress() << " @ " << config.getOpcServerIp() << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        nsw::hw::Router router_hw(opcManager, config);
        router_hw.writeConfiguration();
        std::cout << std::endl;
        std::cout << std::endl;
    }

    // read all GPIO as desired
    if (read_all_gpio) {
        for (auto & config : board_configs) {
          nsw::hw::Router router_hw(opcManager, config);
          const auto name_and_val = router_hw.readConfiguration();
          for (const auto& [name, val]: name_and_val) {
            std::cout << std::left << std::setw(40) << (config.getAddress() + " " + name)
                      << " = " << val
                      << std::endl;
          }
          std::cout << std::endl;
        }
    }

    return 0;
}

