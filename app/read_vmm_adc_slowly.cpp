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

namespace po = boost::program_options;

std::string now_in_milliseconds() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  auto timer = system_clock::to_time_t(now);
  std::tm bt = *std::localtime(&timer);
  std::ostringstream oss;
  oss << std::put_time(&bt, "%Y_%m_%d_%H_%M_%S");
  oss << '_' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
  //std::cout << "INFO :: Start configure"
  //<< " " << oss.str()
  //<< std::endl;
}

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

    int n_samples;
    int thdac;
    int sleep_time;
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
        ("sleep", po::value<int>(&sleep_time)->
        default_value(1), "Time for sleeping between measurements")
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
    } catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    // If no name is given, find all elements
    std::set<std::string> frontend_names;
    if (fe_name != "") {
      frontend_names.emplace(fe_name);
    } 
    else {
      frontend_names = reader1.getAllElementNames();
    }

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
    int tpdac        = -1;
    int channel_trim = -1;
    int time_now_ms  = -1;

    for (auto & feb : frontend_configs) {

      for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

        for (int channel_id = 0; channel_id < CHS; channel_id++) {

          if (targeted_vmm_id       != -1 && vmm_id       != targeted_vmm_id)     continue;
          if (targeted_channel_id   != -1 && channel_id   != targeted_channel_id) continue;

          // en.cppreference.com/w/cpp/io/manip/put_time
          std::time_t  t =  std::time(nullptr);
          std::tm     tm = *std::localtime(&t);

          // using namespace std::chrono;
          // auto now = system_clock::now();
          // auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
          // auto timer = system_clock::to_time_t(now);
          // std::tm bt = *std::localtime(&timer);
          // std::ostringstream oss;
          // oss << std::put_time(&bt, "%Y_%m_%d_%H_%M_%S");
          // oss << '_' << std::setfill('0') << std::setw(3) << ms.count();
          // std::cout << "INFO :: Start configure"
          //           << " " << oss.str()
          //           << std::endl;


          using namespace std::chrono;
          time_now_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

          std::cout << "INFO"
                    << " " << "Start configure"
                  //<< " " << now_in_milliseconds()
                    << " " << time_now_ms
                    << " " << feb.getAddress()
                    << " " << vmm_id
                    << " " << channel_id
                    << " " << tpdac
                    << " " << thdac
                    << " " << channel_trim
                    << std::endl;

          // std::cout << "INFO"
          //           << " " << &tm
          //           << " " << std::put_time(&tm, "%Y_%m_%d_%Hh_%Mm_%Ss")
          //           << " " << feb.getAddress()
          //           << " " << vmm_id
          //           << " " << channel_id
          //           << " " << tpdac
          //           << " " << thdac
          //           << " " << channel_trim
          //           << std::endl;
          //sleep(60);

          //cs.sendVmmConfigSingle(feb, vmm_id);
          //cs.sendVmmConfigSingle(feb, vmm_id);
          //cs.sendVmmConfigSingle(feb, vmm_id);
          //cs.sendVmmConfigSingle(feb, vmm_id);
          //cs.sendVmmConfigSingle(feb, vmm_id);
          auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, channel_trim, n_samples);

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

          time_now_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
          std::cout << "INFO"
                    << " " << "  End configure"
                  //<< " " << now_in_milliseconds()
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

