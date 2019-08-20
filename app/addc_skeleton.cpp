// Program to set ADDC configuration?

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/Utility.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) 
{
    std::string config_filename;
    std::string board_name;

    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/addc_test_art_common_config.json"),
         "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)->
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
    std::set<std::string> board_names;
    if (board_name != ""){
        if (std::count(board_name.begin(), board_name.end(), ',')){
            std::istringstream ss(board_name);
            while(!ss.eof()){
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    board_names.emplace(buf);
            }
        }
        else
            board_names.emplace(board_name);
    }
    else
        board_names = reader1.getAllElementNames();

    // make ADDC objects
    std::vector<nsw::ADDCConfig> addc_configs;
    for (auto & name : board_names) {
        try {
            if (nsw::getElementType(name) == "ADDC") {
                addc_configs.emplace_back(reader1.readConfig(name));
                std::cout << "Adding: " << name << std::endl;
            }
            else
                std::cout << "Skipping: " << name
                          << " because its a " << nsw::getElementType(name)
                          << std::endl;
        }
        catch (std::exception & e) {
            std::cout << name << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }

    // the sender
    nsw::ConfigSender cs;

    // announce
    for (auto & addc: addc_configs){
        std::cout << "Found " << addc.getAddress() << " @ " << addc.getOpcServerIp() << std::endl;
        for (auto art: addc.getARTs()){
            std::cout << "Found " << art.getName()
                      << " with JSON::art_test "<< art.register0_test_00()
                      << " and " << art.art_core_cfg_deser_flagmask() << std::endl;
        }
        std::cout << std::endl;
        std::cout << std::endl;
        cs.sendAddcConfig(addc);
        std::cout << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

