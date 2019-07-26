// Program to set ADDC configuration?

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ADDCConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) 
{
    std::string config_filename;
    std::string fe_name;

    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/addc_test_art_common_config.json"),
         "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
         default_value("ADDC_config_00,ADDC_config_01"),
         "The name of frontend to configure (should contain ADDC).");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // create a json reader
    nsw::ConfigReader reader1("json://" + config_filename);
    try {
        auto config1 = reader1.readConfig();
    }
    catch (std::exception & e) {
        std::cout << "Make sure the json is formed correctly. "
                  << "Can't read config file due to : " << e.what() << std::endl;
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }

    // parse input names
    std::set<std::string> frontend_names;
    if (fe_name != ""){
        if (std::count(fe_name.begin(), fe_name.end(), ',')){
            std::istringstream ss(fe_name);
            while(!ss.eof()){
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    frontend_names.emplace(buf);
            }
        }
        else
            frontend_names.emplace(fe_name);
    }
    else
        frontend_names = reader1.getAllElementNames();

    // make FE objects
    std::vector<nsw::ADDCConfig> frontend_configs;
    for (auto & name : frontend_names) {
        try {
            if (name.find("ADDC") != std::string::npos)
                frontend_configs.emplace_back(reader1.readConfig(name));
        }
        catch (std::exception & e) {
            std::cout << name << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }

    // announce
    for (auto & feb: frontend_configs){
        std::cout << "Found " << feb.getAddress() << " @ " << feb.getOpcServerIp() << std::endl;
        for (auto art: feb.getARTs()){
            std::cout << "Found " << art.getName() << " with JSON::art_test " << art.register0_test_00() << " and " << art.art_core_cfg_deser_flagmask() << std::endl;
        }
    }

    return 0;
}

