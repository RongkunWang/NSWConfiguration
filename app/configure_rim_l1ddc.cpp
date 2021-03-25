// Sample program to read configuration from db/json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"

#include "boost/program_options.hpp"

int main(int ac, const char *av[]) {
    std::string description = "This program is for reading/writing GPIO on the Rim L1DDC";
    bool sel   = false;
    bool sel0  = false;
    bool sel1  = false;
    bool sim   = false;
    bool clock = false;
    bool feast = false;
    try {
      boost::program_options::options_description desc(description);
      desc.add_options()
        ("help,h", "produce help message")
        ("sel",   boost::program_options::value<bool>(&sel) ->default_value(false), "Bool to set sel")
        ("sel0",  boost::program_options::value<bool>(&sel0)->default_value(false), "Bool to set sel0")
        ("sel1",  boost::program_options::value<bool>(&sel1)->default_value(false), "Bool to set sel1")
        ("feast", boost::program_options::bool_switch()->default_value(false), "Option to send feast comands")
        ("clock", boost::program_options::bool_switch()->default_value(false), "Option to send clock comands")
        ("sim",   boost::program_options::bool_switch()->default_value(false), "Option to NOT send configurations")
        ;
      boost::program_options::variables_map vm;
      boost::program_options::store(boost::program_options::parse_command_line(ac, av, desc), vm);
      boost::program_options::notify(vm);
      sim   = vm.at("sim")  .as<bool>();
      feast = vm.at("feast").as<bool>();
      clock = vm.at("clock").as<bool>();
      if (vm.count("help") > 0) {
        std::cout << desc << std::endl;
        return 1;
      }
    } catch (const std::exception& ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      return 1;
    }

    nsw::ConfigSender cs;

    std::string opc_ip    = "pcatlnswfelix01.cern.ch:48020";
    std::string sca_addr  = "RimL1DDC_PRI";
    std::string gpio_pad  = sca_addr + ".gpio.feast_en1_pad";
    std::string gpio_r1   = sca_addr + ".gpio.feast_en1_r1";
    std::string gpio_r2   = sca_addr + ".gpio.feast_en1_r2";
    std::string gpio_r3   = sca_addr + ".gpio.feast_en1_r3";
    std::string gpio_r4   = sca_addr + ".gpio.feast_en1_r4";
    std::string gpio_r5   = sca_addr + ".gpio.feast_en1_r5";
    std::string gpio_r6   = sca_addr + ".gpio.feast_en1_r6";
    std::string gpio_r7   = sca_addr + ".gpio.feast_en1_r7";
    std::string gpio_r8   = sca_addr + ".gpio.feast_en1_r8";
    std::string gpio_sel  = sca_addr + ".gpio.sca1_sel";
    std::string gpio_sel0 = sca_addr + ".gpio.sca1_sel0";
    std::string gpio_sel1 = sca_addr + ".gpio.sca1_sel1";

    if (feast) {
      try {
        std::cout << "Reading " << gpio_pad << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_pad))) << std::endl;
        std::cout << "Reading " << gpio_r1  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r1)))  << std::endl;
        std::cout << "Reading " << gpio_r2  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r2)))  << std::endl;
        std::cout << "Reading " << gpio_r3  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r3)))  << std::endl;
        std::cout << "Reading " << gpio_r4  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r4)))  << std::endl;
        std::cout << "Reading " << gpio_r5  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r5)))  << std::endl;
        std::cout << "Reading " << gpio_r6  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r6)))  << std::endl;
        std::cout << "Reading " << gpio_r7  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r7)))  << std::endl;
        std::cout << "Reading " << gpio_r8  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r8)))  << std::endl;
        std::cout << "Writing " << gpio_pad << ": " << 1 << std::endl;
        if (!sim) {
          cs.sendGPIO(opc_ip, gpio_pad, true);
          cs.sendGPIO(opc_ip, gpio_r1,  false);
          cs.sendGPIO(opc_ip, gpio_r2,  false);
          cs.sendGPIO(opc_ip, gpio_r3,  false);
          cs.sendGPIO(opc_ip, gpio_r4,  false);
          cs.sendGPIO(opc_ip, gpio_r5,  false);
          cs.sendGPIO(opc_ip, gpio_r6,  false);
          cs.sendGPIO(opc_ip, gpio_r7,  false);
          cs.sendGPIO(opc_ip, gpio_r8,  false);
        }
        std::cout << "Reading " << gpio_pad << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_pad))) << std::endl;
        std::cout << "Reading " << gpio_r1  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r1)))  << std::endl;
        std::cout << "Reading " << gpio_r2  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r2)))  << std::endl;
        std::cout << "Reading " << gpio_r3  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r3)))  << std::endl;
        std::cout << "Reading " << gpio_r4  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r4)))  << std::endl;
        std::cout << "Reading " << gpio_r5  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r5)))  << std::endl;
        std::cout << "Reading " << gpio_r6  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r6)))  << std::endl;
        std::cout << "Reading " << gpio_r7  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r7)))  << std::endl;
        std::cout << "Reading " << gpio_r8  << ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_r8)))  << std::endl;
      } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return 1;
      }
    }

    if (clock) {
      try {
        std::cout << "Reading " << gpio_sel  << " : " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel)))  << std::endl;
        std::cout << "Reading " << gpio_sel0 <<  ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel0))) << std::endl;
        std::cout << "Reading " << gpio_sel1 <<  ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel1))) << std::endl;
        std::cout << "Writing " << gpio_sel  << " : " << sel  << std::endl;
        std::cout << "Writing " << gpio_sel0 <<  ": " << sel0 << std::endl;
        std::cout << "Writing " << gpio_sel1 <<  ": " << sel1 << std::endl;
        if (!sim) {
          cs.sendGPIO(opc_ip, gpio_sel,  sel);
          cs.sendGPIO(opc_ip, gpio_sel0, sel0);
          cs.sendGPIO(opc_ip, gpio_sel1, sel1);
        }
        std::cout << "Reading " << gpio_sel  << " : " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel)))  << std::endl;
        std::cout << "Reading " << gpio_sel0 <<  ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel0))) << std::endl;
        std::cout << "Reading " << gpio_sel1 <<  ": " << (sim ? -1 : static_cast<int>(cs.readGPIO(opc_ip, gpio_sel1))) << std::endl;
      } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return 1;
      }
    }

    return 0;
}

