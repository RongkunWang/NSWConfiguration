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

    bool dump;
    bool baseline;
    bool threshold;
    int n_samples;
    int thdac;
    int channel_trim;
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
        ("trim,T", po::value<int>(&channel_trim)->
        default_value(-1), "Overwrite the trim value")
        ("threshold", po::bool_switch()->
        default_value(false), "Read the channel trimmed threshold")
        ("baseline", po::bool_switch()->
        default_value(false), "Read the channel analog output (baseline)")
        ("dump", po::bool_switch()->
        default_value(false), "Dump information to the screen")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    dump      = vm["dump"]     .as<bool>();
    baseline  = vm["baseline"] .as<bool>();
    threshold = vm["threshold"].as<bool>();

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    if ( (!baseline and !threshold) || (baseline and threshold) ) {
        std::cout << "Please read either --baseline or --threshold" << std::endl;
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

    for (auto & feb : frontend_configs) {

      auto & vmms = feb.getVmms();

      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

        for (int channel_id = 0; channel_id < CHS; channel_id++) {

          if (targeted_vmm_id     != -1 && vmm_id     != targeted_vmm_id)     continue;
          if (targeted_channel_id != -1 && channel_id != targeted_channel_id) continue;

          if (dump)
            std::cout << "INFO "
                      << feb.getAddress() << " "
                      << vmm_id << " "
                      << channel_id << " "
                      << tpdac << " "
                      << thdac << " "
                      << channel_trim  << " "
                      << std::endl;

          // configure the VMM
          if (true)              feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
          if (threshold)         feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
          if (baseline)          feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelAnalogOutput);
          if (channel_trim >= 0) feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(channel_trim));
          if (thdac >= 0)        feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));

          auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);

          if (dump)
            for (auto result: results)
              std::cout << "DATA "
                        << feb.getAddress() << " "
                        << vmm_id << " "
                        << channel_id << " "
                        << tpdac << " "
                        << thdac << " "
                        << channel_trim  << " "
                        << result << std::endl;
          
        }
      }

      // reset the MO for all channels
      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++)
        vmms[vmm_id].setChannelRegisterAllChannels("channel_smx", 0);

    }

    return 0;
}

