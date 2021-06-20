//
// Simple program to read configuration from db/json
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPCarrierConfig.h"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

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
    ("c", po::value<std::string>(&config_filename)
     ->default_value(default_fname),
     "Configuration file path")
    ("sim", po::bool_switch()
     ->default_value(false),
     "Option to NOT send configurations")
    ("name,n", po::value<std::string>(&name)
     ->default_value(default_name),
     "Name of trigger processor")
    ;
  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);
  simulation = vm.at("sim").as<bool>();
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  //
  // parse config
  //
  auto cfg = "json://" + config_filename;
  auto carriers = nsw::ConfigReader::makeObjects
    <nsw::TPCarrierConfig>
    (cfg, "TPCarrier", name);

  //
  // send config
  //
  nsw::ConfigSender cs;
  for (const auto& carrier: carriers) {
    if (!simulation) {
      cs.sendTPCarrierConfig(carrier);
    }
  }

  return 0;
}

