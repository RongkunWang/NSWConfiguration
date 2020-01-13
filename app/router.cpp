// Program to set Router configuration

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/Utility.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;

    po::options_description desc(std::string("Router configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,C", po::value<std::string>(&config_filename)->default_value(config_files+"router.json"), "Configuration file path")
        ("name,n",        po::value<std::string>(&board_name)     ->default_value("Router_00"), "Name of desired router (should contain router).")
        ;
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
    std::vector<nsw::RouterConfig> board_configs;
    for (auto & name : board_names) {
        try {
            if (nsw::getElementType(name) == "Router") {
                board_configs.emplace_back(reader1.readConfig(name));
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
    for (auto & board: board_configs){
        std::cout << "Found " << board.getAddress() << " @ " << board.getOpcServerIp() << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        cs.sendRouterConfig(board);
        std::cout << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

