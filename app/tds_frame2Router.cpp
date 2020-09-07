//
// A script to enable/disable the frame2Router_enable register of sFEB TDSs
// NB: a comma-separated list of sFEB or TDS names can be given.
//

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"
#include "boost/program_options.hpp"

size_t NTDS = 4;
namespace po = boost::program_options;
std::set<std::string> split(std::string str);
bool setRegisterValue(nsw::FEBConfig & feb, std::set<std::string> tds_names, bool enable, bool dry);

int main(int argc, const char *argv[]) {

  // user arguments
  std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
  std::string vs_filename = config_files + "config_json/VS/VS_sFEB_frame2Router.json";
  std::string config;
  std::string sfeb_name;
  std::string tds_name;
  bool frame2Router_enable;
  bool frame2Router_disable;
  bool dry;
  po::options_description desc(std::string("TDS frame2Router script"));
  desc.add_options()
      ("help,h", "produce help message")
      ("config,c", po::value<std::string>(&config)->default_value(std::string(vs_filename)), "Configuration file")
      ("sfeb,s",   po::value<std::string>(&sfeb_name)->default_value(""), "Name of sFEB")
      ("tds,t",    po::value<std::string>(&tds_name)->default_value(""), "Name of TDS")
      ("enable",  po::bool_switch()->default_value(false), "Enable frame2Router")
      ("disable", po::bool_switch()->default_value(false), "Disable frame2Router")
      ("dry",     po::bool_switch()->default_value(false), "Enable a dry run (no commands sent to the hardware)")
      ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  frame2Router_enable  = vm["enable"] .as<bool>();
  frame2Router_disable = vm["disable"].as<bool>();
  dry                  = vm["dry"]    .as<bool>();
  if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
  }    

  // check user arguments
  if (!frame2Router_enable && !frame2Router_disable)
      throw std::runtime_error("Please choose either --enable or --disable");
  if (frame2Router_enable && frame2Router_disable)
      throw std::runtime_error("--enable and --disable cannot both be chosen");

  // make sFEB objects and TDS strings
  auto sfeb6s = nsw::ConfigReader::makeObjects<nsw::FEBConfig>("json://" + config, "SFEB6", sfeb_name);
  auto sfeb8s = nsw::ConfigReader::makeObjects<nsw::FEBConfig>("json://" + config, "SFEB8", sfeb_name);
  auto tds_names = split(tds_name);
  std::cout << "Adding:" << std::endl;
  for (auto & obj: tds_names)
      std::cout << " " << obj << std::endl;

  // send the TDS config commands
  std::cout << std::endl;
  std::cout << "Setting registers in the TDS:" << std::endl;
  std::cout << std::endl;
  for (auto & feb: sfeb6s)
    setRegisterValue(feb, tds_names, frame2Router_enable, dry);
  for (auto & feb: sfeb8s)
    setRegisterValue(feb, tds_names, frame2Router_enable, dry);

  std::cout << std::endl;
  std::cout << "Done! boop" << std::endl;
  std::cout << std::endl;
}

bool setRegisterValue(nsw::FEBConfig & feb, std::set<std::string> tds_names, bool enable, bool dry) {
  auto cs = std::make_unique<nsw::ConfigSender>();
  auto opc_ip = feb.getOpcServerIp();
  auto sca_address = feb.getAddress();
  std::string subreg = "test_frame2Router_enable";
  for (auto & tds : feb.getTdss()) {
    for (auto name: tds_names) {
      if (tds.getName() != name)
        continue;
      std::cout << " " << sca_address
                << " " << name
                << " " << subreg
                << " = " << (enable ? 1 : 0)
                << std::endl;
      tds.setRegisterValue("register12", subreg, enable ? 1 : 0);
      if (!dry)
        cs->sendI2cMasterSingle(opc_ip, sca_address, tds, "register12");
    }
  }
  return 0;
}

std::set<std::string> split(std::string name) {
  std::set<std::string> names;
  if (name != "") {
    if (std::count(name.begin(), name.end(), ',')) {
      std::istringstream ss(name);
      while (!ss.eof()) {
        std::string buf;
        std::getline(ss, buf, ',');
        if (buf != "")
          names.emplace(buf);
      }
    } else {
      names.emplace(name);
    }
  } else {
    for (size_t i_tds = 0; i_tds < NTDS; i_tds++) {
      names.emplace("tds" + std::to_string(i_tds));
    }
  }
  return names;
}
