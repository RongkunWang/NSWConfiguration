//
// Program to read/write Pad Trigger configuration
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/OpcManager.h"
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
    std::string config_filename{""};
    std::string board_name{""};
    std::string gpio_name{""};
    std::string i2c_reg{""};
    std::string i2c_val{""};
    int val{0};

    // options
    po::options_description desc(std::string("Pad trigger configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(""), "Config file path. REQUIRED. Can also set by `export JSON=XXX`")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("gpio", po::value<std::string>(&gpio_name)
         ->default_value(""), "GPIO name to read/write (check the xml for valid names).")
        ("dump", po::bool_switch()->
         default_value(false), "Option to dump pad trigger fpga i2c mapping")
        ("uploadBitfile,b", po::bool_switch()->
         default_value(false), "Option to upload bitfile to FPGA via JTAG. WARNING: EXPERIMENTAL")
        ("do_config", po::bool_switch()->
         default_value(false), "Option to send predefined configuration")
        ("do_control", po::bool_switch()->
         default_value(false), "Option to send Pad Trigger control register (built from json)")
        ("toggleOcrEnable", po::bool_switch()->
         default_value(false), "Option to toggle the Pad Trigger OCR enable")
        ("toggleGtReset", po::bool_switch()->
         default_value(false), "Option to toggle the Pad Trigger GT resets")
        ("toggleIdleState", po::bool_switch()->
         default_value(false), "Option to toggle the Pad Trigger idle state in the control register")
        ("read,r", po::bool_switch()->
         default_value(false), "Option to read FPGA configuration and status registers")
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
    const auto dump            = vm["dump"]           .as<bool>();
    const auto do_config       = vm["do_config"]      .as<bool>();
    const auto do_control      = vm["do_control"]     .as<bool>();
    const auto uploadBitfile   = vm["uploadBitfile"]  .as<bool>();
    const auto toggleOcrEnable = vm["toggleOcrEnable"].as<bool>();
    const auto toggleGtReset   = vm["toggleGtReset"]  .as<bool>();
    const auto toggleIdleState = vm["toggleIdleState"].as<bool>();
    const auto read            = vm["read"]           .as<bool>();
    if (vm.count("help") > 0) {
        std::cout << desc << "\n";
        return 0;
    }

    // check for json
    if (config_filename.empty()) {
      config_filename = nsw::getenv("JSON");
      if (config_filename.empty()) {
        std::cout << desc << std::endl;;
        return 1;
      } else {
        std::cout << fmt::format("Using environment JSON: {}",
                                 config_filename) << std::endl;
      }
    }

    // make objects from json
    const auto configs = nsw::ConfigReader::makeObjects<boost::property_tree::ptree>
      (fmt::format("json://{}", config_filename), "PadTrigger", board_name);
    if (configs.empty()) {
      std::cerr << "You provided a JSON with no pad triggers! Exiting." << std::endl;
      return 1;
    }

    // the hw objects
    nsw::OpcManager opcManager{};
    std::vector<nsw::hw::PadTrigger> hws;
    hws.reserve(configs.size());
    for (const auto& cfg: configs) {
      hws.emplace_back(nsw::hw::PadTrigger(opcManager, cfg));
    }

    for (const auto& hw: hws) {
      std::cout << fmt::format("Found {}\n with {} -> T = {}",
                               hw.getName(),
                               hw.firmware(),
                               hw.firmware_dateword()) << std::endl;
    }

    // dump config
    if (dump) {
      for (const auto& hw: hws) {
        hw.getFpga().dump();
      }
    }

    // GPIO read/write
    if (gpio_name != "") {
      for (const auto& hw: hws) {
        if (val != -1) {
          std::cout << fmt::format("{} -> write {}", hw.getName(), val) << std::endl;
          hw.writeGPIO(gpio_name, static_cast<bool>(val));
        }
        std::cout << fmt::format("{} -> read {}", hw.getName(), hw.readGPIO(gpio_name)) << std::endl;
      }
    }

    // pad i2c
    if (i2c_reg != "") {
      const auto i2c_reg_08 = static_cast<uint8_t>(std::stoul(i2c_reg, nullptr, nsw::BASE_HEX));
      for (const auto& hw: hws) {
        if (i2c_val != "") {
          const auto i2c_val_32 = static_cast<uint32_t>(std::stol(i2c_val, nullptr, nsw::BASE_HEX));
          std::cout << fmt::format("Writing {}: register address {} with value {}",
                                   hw.getName(), i2c_reg, i2c_val) << std::endl;
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

    // fpga bitstream
    if (uploadBitfile) {
      for (const auto& hw: hws) {
        hw.writeJTAGBitfileConfiguration();
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

    // GT reset
    if (toggleGtReset) {
      for (const auto& hw: hws) {
        if (not hw.GtReset()) {
          throw std::runtime_error(
            "You asked for toggleGtReset, but PT has GtReset=False! Something is wrong."
          );
        }
        hw.toggleGtReset();
      }
    }

    // idle state
    if (toggleIdleState) {
      for (const auto& hw: hws) {
        if (not hw.Toggle()) {
          throw std::runtime_error(
            "You asked for toggleIdleState, but PT has Toggle=False! Something is wrong."
          );
        }
        hw.toggleIdleState();
      }
    }

    // OCR enable
    if (toggleOcrEnable) {
      for (const auto& hw: hws) {
        if (not hw.OcrEnable()) {
          throw std::runtime_error(
            "You asked for toggleOcrEnable, but PT has OcrEnable=False! Something is wrong."
          );
        }
        hw.toggleOcrEnable();
      }
    }

    // read
    if (read) {
      std::cout << "Read registers:" << std::endl;
      for (const auto& hw: hws) {
        for (const auto& [addr, val]: hw.readConfiguration()) {
          std::cout << fmt::format("{} address {:03x}: {:#010x}", hw.getName(), addr, val)
                    << std::endl;
        }
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

