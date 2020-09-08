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
    bool read_all_gpio;
    bool no_config;

    po::options_description desc(std::string("Router configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(config_files+"router.json"), "Configuration file path")
        ("name,n",        po::value<std::string>(&board_name)
         ->default_value("Router_A14_L0"), "Name of desired router (should contain router).")
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

    // make router objects
    auto board_configs = nsw::ConfigReader::makeObjects<nsw::RouterConfig>
      ("json://" + config_filename, "Router", board_name);

    // the sender
    nsw::ConfigSender cs;

    // announce
    for (auto & board : board_configs) {
        if (no_config)
            continue;
        std::cout << "Found " << board.getAddress() << " @ " << board.getOpcServerIp() << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        cs.sendRouterConfig(board);
        std::cout << std::endl;
        std::cout << std::endl;
    }

    // read all GPIO as desired
    if (read_all_gpio) {
        auto pairs = {std::make_pair("fpgaConfigOK",      "0"),
                      std::make_pair("routerId0",         "1"),
                      std::make_pair("routerId1",         "2"),
                      std::make_pair("routerId2",         "3"),
                      std::make_pair("routerId3",         "4"),
                      std::make_pair("routerId4",         "5"),
                      std::make_pair("routerId5",         "6"),
                      std::make_pair("routerId6",         "7"),
                      std::make_pair("routerId7",         "8"),
                      std::make_pair("mmcmBotLock",       "9"),
                      std::make_pair("designNum0",       "10"),
                      std::make_pair("designNum1",       "11"),
                      std::make_pair("designNum2",       "12"),
                      std::make_pair("fpgaInit",         "13"),
                      std::make_pair("mmcmReset",        "14"),
                      std::make_pair("softReset",        "15"),
                      std::make_pair("rxClkReady",       "16"),
                      std::make_pair("txClkReady",       "17"),
                      std::make_pair("cpllTopLock",      "18"),
                      std::make_pair("cpllBotLock",      "19"),
                      std::make_pair("mmcmTopLock",      "20"),
                      std::make_pair("semFatalError",    "21"),
                      std::make_pair("semHeartBeat",     "22"),
                      std::make_pair("debugEnable",      "23"),
                      std::make_pair("notConnected",     "24"),
                      std::make_pair("mtxRst",           "25"),
                      std::make_pair("masterChannel0",   "26"),
                      std::make_pair("masterChannel1",   "27"),
                      std::make_pair("masterChannel2",   "28"),
                      std::make_pair("ctrlMod0",         "29"),
                      std::make_pair("ctrlMod1",         "30"),
                      std::make_pair("multibootTrigger", "31")};
        for (auto & board : board_configs) {
            auto opc_ip   = board.getOpcServerIp();
            auto sca_addr = board.getAddress();
            for (auto pair : pairs) {
                auto gpio_addr = sca_addr + ".gpio." + pair.first;
                std::cout << std::left << std::setw(40) << (gpio_addr + " " + pair.second)
                          << " = " << cs.readGPIO(opc_ip, gpio_addr)
                          << std::endl;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}

