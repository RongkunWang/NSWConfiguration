// Sample program to read configuration from db/json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPConfig.h"

#include "boost/program_options.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace po = boost::program_options;
namespace pt = boost::property_tree;
using boost::property_tree::ptree;

int main(int ac, const char *av[]) {
    std::string description = "This program is for sending/receiving messages from the SCAX on the TP.";
    std::string confdir = "/afs/cern.ch/user/n/nswdaq/public/sw1/config-ttc/config-files/";
    std::string config_filename;
    std::string tp_name;
    bool dry_run;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(confdir + "full_small_sector_14_internalPulser_ADDC.json"),
        "Configuration file path")
        ("dry_run",   po::bool_switch()->default_value(false), "Option to NOT send configurations")
        ("tp,t", po::value<std::string>(&tp_name)->
        default_value("MMTP_A14"),
        "Name of trigger processor");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    dry_run = vm["dry_run"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    // Testing json parsing
    nsw::ConfigReader reader_tp("json://" + config_filename);
    reader_tp.readConfig();
    auto tp_config_tree = pt::ptree();
    try {
      tp_config_tree = reader_tp.readConfig(tp_name);
    }
    catch (std::exception &e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::cout << "Parsed ptree, about to build TPConfig" << std::endl;

    nsw::TPConfig tp(tp_config_tree);
    tp.dump();

    // setRegisterValue(std::string master, std::string slave, uint32_t value);
    // getRegisterValue(std::string master, std::string slave);

    nsw::ConfigSender cs;  // in principle the config sender is all that is needed for now

    std::cout << "Created a ConfigSender" << std::endl;
    if (!dry_run)
        cs.sendTPConfig(tp);

    std::cout << "... Done with configure_tp" << std::endl;

    return 0;
}

