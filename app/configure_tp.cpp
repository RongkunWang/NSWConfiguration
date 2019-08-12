// Sample program to read configuration from db/json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPConfig.h"
// #include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/OpcClient.h"

#include "boost/program_options.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace po = boost::program_options;
namespace pt = boost::property_tree;
using boost::property_tree::ptree;

int main(int ac, const char *av[])
{
    std::string description = "This program is for sending/receiving messages from the SCX on the TP.";

    std::string config_filename;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value("/afs/cern.ch/user/b/bbullard/public/nsw/conf/NSWConfiguration/test/TP_testRegisterConfig.json"),
        "Configuration file path");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    // Testing json parsing
    nsw::ConfigReader reader_tp("json://" + config_filename);
    reader_tp.readConfig();
    auto tp_config_tree = pt::ptree();
    try {
      tp_config_tree = reader_tp.readConfig("STGCTP-0001");
    }
    catch (std::exception &e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::ofstream ptree_out;
    ptree_out.open("ptree_testParsingOutput.json");
    pt::write_json(ptree_out, tp_config_tree);
    ptree_out.close();

    std::cout << "Parsed ptree, about to build TPConfig" << std::endl;

    nsw::TPConfig tp(tp_config_tree);
    tp.dump();

    // setRegisterValue(std::string master, std::string slave, uint32_t value);
    // getRegisterValue(std::string master, std::string slave);
    
    nsw::ConfigSender cs; // in principle the config sender is all that is needed for now
    
    std::cout << "Created a ConfigSender" << std::endl;
    cs.sendTpConfig(tp);

    std::cout << "... Done with configure_tp" << std::endl;

    return 0;

}
