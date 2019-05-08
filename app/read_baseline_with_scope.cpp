// Sample program to read multiple ADC values from a channel of VMM

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>
#include <ctime> 
#include <chrono> 

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po  = boost::program_options;
namespace chr = std::chrono;

int main(int ac, const char *av[]) {

    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";
    std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

    bool dump;
    int n_samples;
    int sleep_time;
    int targeted_vmm_id;
    int targeted_channel_id;
    int scope_channel;
    int scope_n;
    int scope_overwrite;
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
        default_value(2500), "Number of samples to read")
        ("vmm,V", po::value<int>(&targeted_vmm_id)->
        default_value(-1), "VMM id (0-7) to read (otherwise: loop)")
        ("channel,C", po::value<int>(&targeted_channel_id)->
        default_value(-1), "VMM channel to read (otherwise: loop)")
        ("sleep", po::value<int>(&sleep_time)->
        default_value(1), "Time for sleeping between measurements")
        ("screens", po::value<int>(&scope_n)->
        default_value(1), "Number of scope screenshots to read")
        ("scope_channel", po::value<int>(&scope_channel)->
        default_value(2), "Which scope channel to read")
        ("scope_overwrite", po::value<int>(&scope_overwrite)->
        default_value(1), "The output file of scope measurements: dont overwrite (0) or overwrite (1)")
        ("dump", po::bool_switch()->
        default_value(false), "Dump information to the screen")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    dump = vm["dump"].as<bool>();

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

    // If no name is given, find all elements
    std::set<std::string> frontend_names;
    if (fe_name != "")
      frontend_names.emplace(fe_name);
    else
      frontend_names = reader1.getAllElementNames();

    // Create config objects
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
    int thdac        = -1;
    int tpdac        = -1;
    int channel_trim = -1;
    int time_now_ms  = -1;
    int NSAMP_PER_SHOT = 2500;

    for (auto & feb : frontend_configs) {

      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

        for (int channel_id = 0; channel_id < CHS; channel_id++) {

          if (targeted_vmm_id     != -1 && vmm_id     != targeted_vmm_id)     continue;
          if (targeted_channel_id != -1 && channel_id != targeted_channel_id) continue;

          time_now_ms = chr::duration_cast<chr::milliseconds>
            (chr::system_clock::now().time_since_epoch()).count();

          if (dump)
            std::cout << "INFO"
                      << " " << "Start configure"
                      << " " << time_now_ms
                      << " " << feb.getAddress()
                      << " " << vmm_id
                      << " " << channel_id
                      << " " << tpdac
                      << " " << thdac
                      << " " << channel_trim
                      << std::endl;

          //
          // configure the VMM
          //
          cs.setVmmMonitorOutput (feb, vmm_id, channel_id, nsw::vmm::ChannelMonitor,      false);
          cs.setVmmChannelMOMode (feb, vmm_id, channel_id, nsw::vmm::ChannelAnalogOutput, false);

          //
          // read the SCA as many times as you expect to read the scope
          //

          int ntaken = 0;
          while (ntaken < scope_n*NSAMP_PER_SHOT) {
            for (auto result: cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, n_samples)) {
              ntaken++;
              if (dump)
                std::cout << "DATA"
                          << " " << feb.getAddress()
                          << " " << vmm_id
                          << " " << channel_id
                          << " " << tpdac
                          << " " << thdac
                          << " " << channel_trim 
                          << " " << result
                          << std::endl;
            }
            if(n_samples<=1000) break;
          }

          //
          // read the scope
          //

          boost::format ch_fmt("%02d");
          ch_fmt % channel_id;

          std::string n_screens  = " -n " + std::to_string(scope_n);
          std::string sc_channel = " -c " + std::to_string(scope_channel);
          std::string overwrite  = scope_overwrite ? " -r " : "";
          std::string outputfile = "scope_" + feb.getAddress() 
            + "_VMM_" + std::to_string(vmm_id) 
            + "_CH_"  + ch_fmt.str() 
            + ".dat";
          std::string output_ops = " -o " + outputfile;
          std::string ops        = n_screens + sc_channel + overwrite + output_ops;
          std::string cmd        = "python bb5_analysis/scripts/BB5ScopeReadout.py " + ops;
          std::cout << "RUN: " << cmd << std::endl;
          system(cmd.c_str());

          time_now_ms = chr::duration_cast<chr::milliseconds>
            (chr::system_clock::now().time_since_epoch()).count();

          if (dump)
            std::cout << "INFO"
                      << " " << "  End configure"
                      << " " << time_now_ms
                      << " " << feb.getAddress()
                      << " " << vmm_id
                      << " " << channel_id
                      << " " << tpdac
                      << " " << thdac
                      << " " << channel_trim
                      << std::endl;

          sleep(sleep_time);

        }
      }
    }

    return 0;
}

