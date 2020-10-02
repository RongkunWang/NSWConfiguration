//
// A script to read/write a few registers of sFEB TDSs, for trigger testing
// NB: a comma-separated list of sFEB or TDS names can be given.
//

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"
#include "boost/program_options.hpp"

size_t NTDS = 4;
namespace po = boost::program_options;

std::set<std::string> split(std::string str);
std::pair<std::string, std::string> name2gpio(std::string name);
bool setRegisterValue(const nsw::FEBConfig & feb, std::set<std::string> tds_names,
                      std::string subreg, bool enable, bool dry);
bool readRegisterValue(const nsw::FEBConfig & feb, std::set<std::string> tds_names,
                       std::string reg, bool dry);
bool sendReset(const nsw::FEBConfig & feb, std::set<std::string> tds_names, bool dry);

int main(int argc, const char *argv[]) {
  // user arguments
  std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
  std::string vs_filename = config_files + "config_json/VS/VS_sFEB_frame2Router.json";
  std::string config, sfeb_name, tds_name, mode;
  bool enable, disable, reset, read, dry;
  po::options_description desc(std::string("TDS frame2Router script"));
  desc.add_options()
      ("help,h", "produce help message")
      ("config,c", po::value<std::string>(&config)->default_value(std::string(vs_filename)), "Configuration file")
      ("sfeb,s",   po::value<std::string>(&sfeb_name)->default_value(""), "Name of sFEB")
      ("tds,t",    po::value<std::string>(&tds_name)->default_value(""), "Name of TDS")
      ("mode,m",   po::value<std::string>(&mode)->default_value(""), "Mode: bypass_trigger or test_frame2Router_enable")
      ("enable",  po::bool_switch()->default_value(false), "Enable frame2Router")
      ("disable", po::bool_switch()->default_value(false), "Disable frame2Router")
      ("read",    po::bool_switch()->default_value(false), "Read register 14")
      ("reset",   po::bool_switch()->default_value(false), "Send reset to this TDS")
      ("dry",     po::bool_switch()->default_value(false), "Enable a dry run (no commands sent to the hardware)")
      ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  enable  = vm["enable"] .as<bool>();
  disable = vm["disable"].as<bool>();
  read    = vm["read"]   .as<bool>();
  reset   = vm["reset"]  .as<bool>();
  dry     = vm["dry"]    .as<bool>();
  if (vm.count("help")) {
      std::cout << desc << std::endl;;
      return 1;
  }

  // check user arguments
  if (!read && !reset) {
    if (!enable && !disable) {
      std::cout << "Error: Please choose either --enable or --disable" << std::endl;
      return 1;
    } else if (enable && disable) {
      std::cout << "Error: --enable and --disable cannot both be chosen" << std::endl;
      return 1;
    }
    if (mode != "bypass_trigger" && mode != "test_frame2Router_enable") {
      std::cout << "Error: --mode must be bypass_trigger or test_frame2Router_enable" << std::endl;
      std::cout << "You gave: " << (mode == "" ? "(empty)" : mode) << std::endl;
      return 1;
    }
  }

  // make sFEB objects and TDS strings
  auto sfebNs = nsw::ConfigReader::makeObjects<nsw::FEBConfig>("json://" + config, "SFEB",  sfeb_name);
  auto sfeb6s = nsw::ConfigReader::makeObjects<nsw::FEBConfig>("json://" + config, "SFEB6", sfeb_name);
  auto sfeb8s = nsw::ConfigReader::makeObjects<nsw::FEBConfig>("json://" + config, "SFEB8", sfeb_name);
  auto tds_names = split(tds_name);
  std::cout << "Adding:" << std::endl;
  for (auto & obj : tds_names)
      std::cout << " " << obj << std::endl;

  // send the TDS config commands
  if (enable || disable) {
    std::cout << std::endl;
    std::cout << "Setting registers in the TDS:" << std::endl;
    std::cout << std::endl;
    for (auto & feb : sfebNs)
      setRegisterValue(feb, tds_names, mode, enable, dry);
    for (auto & feb : sfeb6s)
      setRegisterValue(feb, tds_names, mode, enable, dry);
    for (auto & feb : sfeb8s)
      setRegisterValue(feb, tds_names, mode, enable, dry);
  }

  // read the monitoring register
  if (read) {
    for (auto & reg : {"register14", "register15"}) {
      for (auto & feb : sfebNs)
        readRegisterValue(feb, tds_names, reg, dry);
      for (auto & feb : sfeb6s)
        readRegisterValue(feb, tds_names, reg, dry);
      for (auto & feb : sfeb8s)
        readRegisterValue(feb, tds_names, reg, dry);
    }
  }

  // send a reset
  if (reset) {
    for (auto & feb : sfebNs)
      sendReset(feb, tds_names, dry);
    for (auto & feb : sfeb6s)
      sendReset(feb, tds_names, dry);
    for (auto & feb : sfeb8s)
      sendReset(feb, tds_names, dry);
  }

  std::cout << std::endl;
  std::cout << "Done! boop" << std::endl;
  std::cout << std::endl;
}

bool setRegisterValue(const nsw::FEBConfig & feb, std::set<std::string> tds_names,
                      std::string subreg, bool enable, bool dry) {
  auto cs = std::make_unique<nsw::ConfigSender>();
  auto opc_ip = feb.getOpcServerIp();
  auto sca_address = feb.getAddress();
  std::string reg = "register12";
  for (auto tds : feb.getTdss()) {
    for (auto name : tds_names) {
      if (tds.getName() != name)
        continue;
      std::cout << " " << sca_address
                << " " << name
                << " " << subreg
                << " = " << (enable ? 1 : 0)
                << std::endl;
      tds.setRegisterValue(reg, subreg, enable ? 1 : 0);
      if (!dry)
        cs->sendI2cMasterSingle(opc_ip, sca_address, tds, reg);
    }
  }
  return 0;
}

bool readRegisterValue(const nsw::FEBConfig & feb, std::set<std::string> tds_names, std::string reg, bool dry) {
  auto cs = std::make_unique<nsw::ConfigSender>();
  auto opc_ip = feb.getOpcServerIp();
  auto sca_address = feb.getAddress();
  for (auto tds : feb.getTdss()) {
    for (auto name : tds_names) {
      if (tds.getName() != name)
        continue;
      std::cout << sca_address << " " << tds.getName() << std::endl;
      std::string address_to_read(reg);
      std::string tds_i2c_address(reg + "_READONLY");
      auto size_in_bytes = tds.getTotalSize(tds_i2c_address) / 8;
      std::string full_node_name = sca_address + "." + tds.getName()  + "." + address_to_read;
      std::vector<uint8_t> dataread(size_in_bytes);
      std::cout << std::hex;
      if (!dry)
        dataread = cs->readI2c(opc_ip, full_node_name , size_in_bytes);
      tds.decodeVector(tds_i2c_address, dataread);
      std::cout << std::dec << std::endl;
    }
  }
  return 0;
}

bool sendReset(const nsw::FEBConfig & feb, std::set<std::string> tds_names, bool dry) {
  auto cs = std::make_unique<nsw::ConfigSender>();
  auto opc_ip = feb.getOpcServerIp();
  auto sca_address = feb.getAddress();
  for (auto tds : feb.getTdss()) {
    for (auto name : tds_names) {
      if (tds.getName() != name)
        continue;
      std::cout << "Send reset: " << sca_address << " " << name << std::endl;
      if (!dry) {
        if      (name == "tds0") cs->sendGPIO(opc_ip, sca_address + ".gpio.tdsaReset", 1);
        else if (name == "tds1") cs->sendGPIO(opc_ip, sca_address + ".gpio.tdsbReset", 1);
        else if (name == "tds2") cs->sendGPIO(opc_ip, sca_address + ".gpio.tdscReset", 1);
        else if (name == "tds3") cs->sendGPIO(opc_ip, sca_address + ".gpio.tdsdReset", 1);
        else
          throw std::runtime_error("BAD TDS BAD");
      }
    }
  }
  return 0;
}

std::pair<std::string, std::string> name2gpio(std::string name) {
  if (name == "tds0") return std::make_pair("tdsaReset", "15");
  if (name == "tds1") return std::make_pair("tdsbReset", "16");
  if (name == "tds2") return std::make_pair("tdscReset", "19");
  if (name == "tds3") return std::make_pair("tdsdReset", "20");
  std::cout << std::endl;
  std::cout << "Cant convert name2gpio for " << name << std::endl;
  std::cout << std::endl;
  throw std::runtime_error("ERS sucks");
  return std::make_pair("", "");
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
