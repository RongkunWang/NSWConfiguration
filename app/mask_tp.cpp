// Sample program to configure TP masking

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

std::string strf_time();

int main(int ac, const char *av[]) {

    //
    // CL args
    //
    bool sim;
    std::string conf_dir   = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json";
    std::string conf_bname = "/BB5/A07/full_large_sector_a07_bb5_cosmics_ADDC_TP_x18_OFF_IncreaseThresholdBy200.json";
    std::string config;
    std::string tp_name;
    po::options_description desc("Configure TP masking");
    desc.add_options()
      ("help,h", "produce help message")
      ("config,c", po::value<std::string>(&config)->default_value(conf_dir + conf_bname), "Configuration file path")
      ("tp,t",     po::value<std::string>(&tp_name)-> default_value("MMTP_A07"),          "Name of trigger processor")
      ("sim,s",    po::bool_switch()->default_value(false),                               "Option to NOT send configurations");
    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    sim = vm["sim"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    //
    // Convert json into TPConfig
    //
    auto tp_configs = nsw::ConfigReader::makeObjects<nsw::TPConfig>("json://" + config, "TP", tp_name);

    //
    // Mask
    //
    nsw::ConfigSender cs;
    for (auto & tp: tp_configs)
      cs.maskTp(tp, sim);


}

