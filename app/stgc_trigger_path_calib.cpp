//
// Program to read/write Pad Trigger configuration
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/hw/FEB.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <boost/program_options.hpp>
#include <fmt/core.h>

namespace po = boost::program_options;

bool file_exists(std::string fname);

int main(int argc, const char *argv[]) 
{
    std::string config_filename{""};
    unsigned int bcid_offset_val{0};

    // options
    po::options_description desc(std::string("stgc_trigger_path configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(""), "Config file path. REQUIRED. Can also set by `export JSON=XXX`")
        ("BCID_offset_val,o", po::value<unsigned int>(&bcid_offset_val)->default_value(1234), "Desired bcid offset")
         ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help") > 0) {
        std::cout << desc << "\n";
        return 0;
    }
   // po::value<int>(&gpio_val)
    
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


    nsw::hw::DeviceManager deviceManager;
    nsw::ConfigReader reader{fmt::format("json://{}", config_filename)};
    for (const auto& name: reader.getAllElementNames()) {
       deviceManager.add(reader.readConfig(name));
    }
  //
    for (const auto& dev: deviceManager.getFebs()) {
      if (nsw::getElementType(dev.getScaAddress()) != "PFEB") {
        continue;
      }
      for (const auto& tds : dev.getTdss()) {
        if (bcid_offset_val > 0xFFF){
           std::cout << "BCID offset larger than expected (12b) " << std::endl;
           return 1;
        }
        tds.writeValue("BCID_Offset", 1234);
      }    
    }

    return 0;
}

bool file_exists(std::string fname) {
    std::ifstream fi(fname.c_str());
    return fi.good();
}

