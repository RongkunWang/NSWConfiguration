// Program to set Pad Trigger configuration

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <boost/program_options.hpp>
#include <fmt/core.h>

namespace po = boost::program_options;

bool file_exists(std::string fname);
double xilinx_temperature_conversion(uint32_t temp);

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;
    std::string bitstream;
    std::string gpio_name;
    std::string i2c_reg;
    std::string i2c_val;
    bool do_config;
    bool do_control;
    int val;

    // options
    po::options_description desc(std::string("Pad trigger configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(config_files+"config_json/191/A12/padTrigger.json"), "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("bitstream,b", po::value<std::string>(&bitstream)
         ->default_value(""), "Bitstream name to write to the FPGA. WARNING: EXPERIMENTAL.")
        ("gpio", po::value<std::string>(&gpio_name)
         ->default_value(""), "GPIO name to read/write (check the xml for valid names).")
        ("do_config", po::bool_switch()->
         default_value(false), "Option to send predefined configuration")
        ("do_control", po::bool_switch()->
         default_value(false), "Option to send Pad Trigger control register (built from json)")
        ("val", po::value<int>(&val)
         ->default_value(-1), "Multi-purpose value for reading and writing. If no value given, will read-only.")
        ("i2c_reg", po::value<std::string>(&i2c_reg)
         ->default_value(""), "i2c register for SCA communication (0xHEX).")
        ("i2c_val", po::value<std::string>(&i2c_val)
         ->default_value(""), "i2c value to write (0xHEX)")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    do_config  = vm["do_config"] .as<bool>();
    do_control = vm["do_control"].as<bool>();
    if (vm.count("help") > 0) {
        std::cout << desc << "\n";
        return 0;
    }

    // make objects from json
    const auto configs = nsw::ConfigReader::makeObjects<nsw::PadTriggerSCAConfig>
      (fmt::format("json://{}", config_filename), "PadTriggerSCA", board_name);
    for (const auto& cfg: configs) {
      std::cout << fmt::format("Found {} @ {} with {} -> T = {}",
                               cfg.getAddress(),
                               cfg.getOpcServerIp(),
                               cfg.firmware(),
                               cfg.firmware_dateword()) << std::endl;
    }

    // the hw objects
    std::vector<nsw::hw::PadTrigger> hws;
    for (const auto& cfg: configs) {
      hws.emplace_back(nsw::hw::PadTrigger(cfg));
    }

    // fpga bitstream
    if (bitstream != "") {
      if (!file_exists(bitstream)) {
        throw std::runtime_error(fmt::format("File doesnt exist: {}", bitstream));
      }
      std::cout << "FPGA bitstreams not supported at this time!" << std::endl;
      return 1;
      // for (auto & board: configs)
      //   cs.sendFPGA(board.getOpcServerIp(), board.getAddress(), bitstream);
    }

    // GPIO read/write
    if (gpio_name != "") {
      for (const auto& hw: hws) {
        if (val != -1) {
          std::cout << fmt::format("{} -> write {}", hw.name(), val) << std::endl;
          hw.writeGPIO(gpio_name, static_cast<bool>(val));
        }
        std::cout << fmt::format("{} -> read {}", hw.name(), hw.readGPIO(gpio_name)) << std::endl;
      }
    }

    // pad i2c
    if (i2c_reg != "") {
      const auto i2c_reg_08 = static_cast<uint8_t>(std::stoul(i2c_reg, nullptr, nsw::BASE_HEX));
      for (const auto& hw: hws) {
        if (i2c_val != "") {
          const auto i2c_val_32 = static_cast<uint32_t>(std::stol(i2c_val, nullptr, nsw::BASE_HEX));
          std::cout << fmt::format("Writing {}: register address {} with value {}",
                                   hw.name(),i2c_reg, i2c_val) << std::endl;
          hw.writeFPGARegister(i2c_reg_08, i2c_val_32);
        }
        const auto val = hw.readFPGARegister(i2c_reg_08);
        std::cout << fmt::format(" Readback {}: {}", i2c_reg_08, val) << std::endl;
        if (i2c_reg_08 == nsw::padtrigger::REG_STATUS) {
          const auto temp = val & 0xfff;
          std::cout << fmt::format(" -> {}C", xilinx_temperature_conversion(temp)) << std::endl;
        }
      }
    }

    // config
    if (do_config) {
      for (const auto& hw: hws) {
        hw.writeConfiguration();
      }
    }

    // control
    if (do_control) {
      for (const auto& hw: hws) {
        hw.writeFPGAConfiguration();
      }
    }

    return 0;
}

double xilinx_temperature_conversion(uint32_t temp) {
  // www.xilinx.com/support/documentation/user_guides/ug480_7Series_XADC.pdf
  return (temp * 503.975 / 4096) - 273.15;
}

bool file_exists(std::string fname) {
    std::ifstream fi(fname.c_str());
    return fi.good();
}

