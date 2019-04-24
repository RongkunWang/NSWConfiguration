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

    int n_samples;
    int thdac;
    int targeted_vmm_id;
    int targeted_channel_id;
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
        ("samples,s", po::value<int>(&n_samples)->
        default_value(10), "Number of samples to read")
        ("thdac", po::value<int>(&thdac)->
        default_value(-1), "Threshold DAC")
        ("vmm,V", po::value<int>(&targeted_vmm_id)->
        default_value(-1), "VMM id (0-7) to read (otherwise: loop)")
        ("channel,C", po::value<int>(&targeted_channel_id)->
        default_value(-1), "VMM channel to read (otherwise: loop)")
      ;

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
    }
    catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::set<std::string> frontend_names;
    if (fe_name != "")
      frontend_names.emplace(fe_name);
    else
      frontend_names = reader1.getAllElementNames();

    std::vector<nsw::FEBConfig> frontend_configs;
    for (auto & name : frontend_names) {
      try {
        frontend_configs.emplace_back(reader1.readConfig(name));
      } 
      catch (std::exception & e) {
        std::cout << name << " - ERROR: Skipping this FE!"
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
      }
    }

    nsw::ConfigSender cs;
    int VMMS  = 8;
    int CHS   = 64;
    int tpdac = -1;
    int dummy = -1;
    int channel_trim = -1;

    for (auto & feb : frontend_configs) {

      auto & vmms = feb.getVmms();

      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

        for (int channel_id = 0; channel_id < CHS; channel_id++) {

          if (targeted_vmm_id     != -1 && vmm_id     != targeted_vmm_id)     continue;
          if (targeted_channel_id != -1 && channel_id != targeted_channel_id) continue;

          // send the channel threshold to the MO, and set the thdac
          vmms[vmm_id].setChannelRegisterAllChannels("channel_smx", 0);             // Channel monitor mode: analog for everything else
          vmms[vmm_id].setChannelRegisterOneChannel ("channel_smx", 1, channel_id); // Channel monitor mode: trim threshold for this channel
          if (thdac != -1)
            vmms[vmm_id].setGlobalRegister          ("sdt_dac", thdac);             // Threshold DAC

          std::cout << "INFO "
                    << feb.getAddress() << " "
                    << vmm_id << " "
                    << channel_id << " "
                    << tpdac << " "
                    << thdac << " "
                    << channel_trim  << " "
                    << std::endl;

          auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, dummy, tpdac, channel_trim, n_samples);

          for (unsigned i = 0; i < results.size(); i++){
            std::cout << "DATA "
                      << feb.getAddress() << " "
                      << vmm_id << " "
                      << channel_id << " "
                      << tpdac << " "
                      << thdac << " "
                      << channel_trim  << " "
                      << results[i] << std::endl;
          }
        }
      }

      // reset the MO for all channels
      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++)
        vmms[vmm_id].setChannelRegisterAllChannels("channel_smx", 0);

    }

    return 0;
}

