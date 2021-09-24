// Sample program to read configuration from json and send to any front end module

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program configures selected or all MMFE8/PFEB/SFEB";
    description += "The name of the front end will be used to determine how many VMM and TDS the board contains.";

    std::string config_filename;
    std::string fe_name;
    std::string tds_i2c_address;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value(""), "The name of frontend to configure (must contain SFEB or PFEB).\n")
        ("i2c-address,a", po::value<std::string>(&tds_i2c_address)->
        default_value("register15"), "I2c register address, register1-register15");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (fe_name.empty()) {
      std::cout << "Please chose name of frontend\n";
      std::cout << desc << "\n";
      return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    try {
      auto config1 = reader1.readConfig();
    } catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::unique_ptr<nsw::FEBConfig> feb;

    try {
      feb = std::make_unique<nsw::FEBConfig>(reader1.readConfig(fe_name));
    } catch (std::exception & e) {
      std::cout << fe_name << " - ERROR: Can't create FEBConfig"
                << " - Problem constructing configuration due to : " << e.what() << std::endl;
    }

    std::cout << "\n";

    nsw::ConfigSender cs;

    std::vector<std::string> readonly({"register13", "register14", "register15"});
    std::cout << "Reading back TDS" << std::endl;
    std::cout << "\nFEB: " << feb->getAddress() << std::endl;
    while (true) {
      auto opc_ip = feb->getOpcServerIp();
      auto feb_address = feb->getAddress();
      for (auto tds : feb->getTdss()) {  // Each tds is I2cMasterConfig
        std::cout << "\nTDS: " << tds.getName() << std::endl;
        std::string full_node_name = feb_address + "." + tds.getName()  + "." + tds_i2c_address;
        auto size_in_bytes = tds.getTotalSize(tds_i2c_address)/8;
        auto dataread = cs.readI2c(opc_ip, full_node_name, size_in_bytes);

        // Maybe different than tds_i2c_address if it's one of readonly addresses
        std::string address_to_decode = tds_i2c_address;
        if (std::find(readonly.begin(), readonly.end(), tds_i2c_address) != readonly.end()) {
          address_to_decode = address_to_decode + "_READONLY";
        }
        tds.decodeVector(address_to_decode, dataread);
        std::cout << "Readback as bytes: ";
        for (auto val : dataread) {
          std::cout << "0x" << std::hex << static_cast<uint32_t>(val) << std::dec << ", ";
        }
        std::cout << "\n";
      }
    }

    return 0;
}
