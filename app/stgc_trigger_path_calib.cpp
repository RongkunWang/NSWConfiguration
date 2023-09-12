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
      if (nsw::getElementType(name) != "SFEB6" and nsw::getElementType(name) != "SFEB8")
	 continue;
      deviceManager.add(reader.readConfig(name));
      std::cout << "test out 0" << std::endl;
    }
  //
    for (const auto& dev: deviceManager.getFebs()) {
      int i_tds = 0;
      for (const auto& tds : dev.getTdss()) {
        i_tds++;
        if (bcid_offset_val > 0xFFF){
           std::cout << "BCID offset larger than expected (12b) " << std::endl;
           return 1;       
        }
        std::cout << "test out for TDS: " << i_tds << "=============" << std::endl;
        std::cout <<  tds.readValue("BCID_Offset") << std::endl;
        std::cout << "test out 2" << std::endl;
        tds.writeValue("BCID_Offset", 1234);
	std::cout <<  tds.readValue("BCID_Offset") << std::endl;
      
	//std::string fe_name;
	//fe_name = "SFEB6_L1Q1_IPL";
	//nsw::ConfigReader reader1("json://" + config_filename);
    	//auto config1 = reader1.readConfig();
    	//auto feb_config_tree = reader1.readConfig(fe_name);
    	//nsw::FEBConfig feb(feb_config_tree);

	auto & roc = dev.getRoc();
        for (unsigned int phase = 0; phase < 128; phase++)
        {
		std::cout <<  tds.readValue("strip_trigger_bcid") << std::endl;
		roc.writeValue("ePllTdc.ePllPhase40MHz_0", phase);
		roc.writeValue("ePllTdc.ePllPhase40MHz_1", phase);
		roc.writeValue("ePllTdc.ePllPhase40MHz_2", phase);
		roc.writeValue("ePllTdc.ePllPhase40MHz_3", phase);
		sleep(1);
        	std::cout <<  tds.readValue("strip_trigger_bcid") << std::endl;
        	std::cout << roc.readValue("ePllTdc.ePllPhase40MHz_0") << std::endl;
        	std::cout << roc.readValue("ePllTdc.ePllPhase40MHz_1") << std::endl;
        	std::cout << roc.readValue("ePllTdc.ePllPhase40MHz_2") << std::endl;
        	std::cout << roc.readValue("ePllTdc.ePllPhase40MHz_3") << std::endl;
        }



		

	}    
    }

    return 0;
}

bool file_exists(std::string fname) {
    std::ifstream fi(fname.c_str());
    return fi.good();
}

