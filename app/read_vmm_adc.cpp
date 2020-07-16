// Sample program to read multiple ADC values from a channel of VMM

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

    int vmm_id;
    int channel_id;
    int n_samples;
    std::string config_filename;
    std::string fe_name;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value(""),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB).\n"
        "If this option is left empty, all front end elements in the config file will be scanned.")
        ("vmm,V", po::value<int>(&vmm_id)->
        default_value(0), "VMM id (0-7)")
        ("channel,C", po::value<int>(&channel_id)->
        default_value(0), "VMM channel")
        ("samples,s", po::value<int>(&n_samples)->
        default_value(10), "Number of samples to read");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
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

    std::set<std::string> frontend_names;
    if (fe_name != "") {
      frontend_names.emplace(fe_name);
    } else {  // If no name is given, find all elements
      frontend_names = reader1.getAllElementNames();
    }

    std::vector<nsw::FEBConfig> frontend_configs;

    std::cout << "\nFollowing front ends will be configured:\n";
    std::cout <<   "========================================\n";
    for (auto & name : frontend_names) {
      try {
        frontend_configs.emplace_back(reader1.readConfig(name));
        std::cout << name << std::endl;
      } catch (std::exception & e) {
        std::cout << name << " - ERROR: Skipping this FE!"
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
      }
      // frontend_configs.back().dump();
    }

    std::cout << "\n";

    nsw::ConfigSender cs;

    for (auto & feb : frontend_configs) {
        // Read pdo of the certain channel n_samples times.
        // This function will also configure VMM with correct parameters
        feb.getVmm(vmm_id).setMonitorOutput(channel_id, nsw::vmm::ChannelMonitor);
        feb.getVmm(vmm_id).setChannelMOMode(channel_id, nsw::vmm::ChannelAnalogOutput);
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);

        double sum = std::accumulate(results.begin(), results.end(), 0.0);
        double mean = sum / results.size();

        double sq_sum = std::inner_product(results.begin(), results.end(), results.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / results.size() - mean * mean);

        std::cout << feb.getAddress() << " vmm" << vmm_id << ", channel " << channel_id
                  << " - mean: " << mean << " , stdev: " << stdev << std::endl;

        // Print first 10
        for (unsigned i = 0; i < 10; i++) {
            if (i >= results.size()) break;
            std::cout << results[i] << ", ";
        }
        std::cout << "\n";
    }

    return 0;
}
