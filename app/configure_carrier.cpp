//
// Simple program to read configuration from db/json
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/TPCarrier.h"
#include "NSWConfiguration/TPConstants.h"

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

using boost::property_tree::ptree;

int main(int ac, const char *av[]) {

  //
  // CL args
  //
  const std::string description   = "Reading/writing TP Carrier SCAX registers";
  const std::string config_files  = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
  const std::string default_fname = config_files + "config_json/VS/tpcarrier.json";
  const std::string default_name  = "TPCarrier";
  std::string config_filename;
  std::string name;
  bool simulation;
  po::options_description desc(description);
  desc.add_options()
    ("help,h", "produce help message")
    ("config,c", po::value<std::string>(&config_filename)
     ->default_value(default_fname),
     "Configuration file path")
    ("sim", po::bool_switch()
     ->default_value(false),
     "Option to NOT send configurations")
    ("writeConfig", po::bool_switch()->default_value(false), "STGCTP option: write configuration")
    ("readConfig", po::bool_switch()->default_value(false), "STGCTP option: read configuration")
    ("name,n", po::value<std::string>(&name)
     ->default_value(default_name),
     "Name of the carrier")
    ;
  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);
  simulation = vm.at("sim").as<bool>();
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  const auto writeConfig = vm["writeConfig"].as<bool>();
  const auto readConfig  = vm["readConfig"] .as<bool>();

  //
  // parse config
  //
  const auto json_filename = fmt::format("json://{}", config_filename);
  const auto carriers = nsw::ConfigReader::makeObjects<boost::property_tree::ptree>
    (json_filename, "TPCarrier", name);

  //
  // send config
  //
  nsw::OpcManager manager{};
  for (const auto& carrier_cfg: carriers) {
    nsw::hw::TPCarrier carrier_hw(manager, carrier_cfg);
    std::cout << std::endl;
    std::cout << ": RJOutSel = " << carrier_hw.RJOutSel() << std::endl;
    std::cout << std::endl;
    if (!simulation) {
      if (writeConfig) {
        carrier_hw.writeConfiguration();
      } 
      if (readConfig) {
        for (const auto& [reg, val]: carrier_hw.readConfiguration()) {
          std::string regName("");
          for (const auto & [regNameRef, regRef]: nsw::carrier::REGS) {
            if (regRef != reg) continue;
            regName = regNameRef;
            break;
}
          std::cout << 
            fmt::format("Reg {:<20}[{:#04x}]:", regName, reg) << 
            fmt::format("val = {:#010x}", val) << std::endl;
        }
      }
    }
  }

  return 0;
}

