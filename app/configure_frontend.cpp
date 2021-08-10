// Sample program to read configuration from json and send to any front end module

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <future>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

struct ThreadConfig {
  bool configure_vmm;
  bool configure_roc;
  bool configure_tds;
  bool create_pulses;
  bool readback_tds;
  bool reset_roc;
  bool reset_vmm;
  bool reset_tds;
  int vmm_to_unmask;
  int channel_to_unmask;
};

int active_threads(std::vector< std::future<int> >* threads);
int configure_frontend(nsw::FEBConfig feb, ThreadConfig cfg);
void readTDS(nsw::FEBConfig & feb) {}

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program configures selected or all MMFE8/PFEB/SFEB";
    description += "The name of the front end will be used to determine how many VMM and TDS the board contains.";

    bool configure_vmm;
    bool configure_roc;
    bool configure_tds;
    bool create_pulses;
    bool readback_tds;
    bool reset_roc;
    bool reset_vmm;
    bool reset_tds;
    int vmm_to_unmask;
    int channel_to_unmask;
    int max_threads;
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
        "If this option is left empty, all front end elements in the config file is configured.")
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure all the VMMs on the FE(Default: False)")
        ("configure-roc,r", po::bool_switch(&configure_roc)->default_value(false),
        "Configure the ROC on the FE(Default: False)")
        ("configure-tds,t", po::bool_switch(&configure_tds)->default_value(false),
        "Configure all the TDSs on the FE(Default: False)")
        ("create-pulses,p", po::bool_switch(&create_pulses)->default_value(false),
        "Create 10 test pulses in ROC by modifying TPInv register(Default: False)")
        ("readback-tds,T", po::bool_switch(&readback_tds)->default_value(false),
        "Readback and decode TDS values(Default: False)")
        ("vmmtounmask,V", po::value<int>(&vmm_to_unmask)->
        default_value(-1), "VMM to unmask (0-7) (Used for ADDC testing)")
        ("channeltounmask,C", po::value<int>(&channel_to_unmask)->
        default_value(-1), "VMM channel to umask (0-63) (Used for ADDC testing)")
        ("max_threads,m", po::value<int>(&max_threads)->
        default_value(-1), "Maximum number of threads to run")
        ("reset,R", po::bool_switch(&reset_roc)->default_value(false),
        "Reset the ROC via SCA. This option can't be used with -r or -v")
        ("resetvmm", po::bool_switch(&reset_vmm)->default_value(false),
        "Hard reset vmm before configuration. Need to be used with -v")
        ("resettds", po::bool_switch(&reset_tds)->default_value(false),
        "Reset TDS SER, logic, ePLL after configuring tds. Need to be used with -t");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if ((!configure_roc && !configure_vmm && !configure_tds) && !reset_roc && !readback_tds) {
        std::cout << "Please chose one of -r, -v, -t, -T or -R command line options to configure ROC/VMM/TDS\n";
        std::cout << desc << "\n";
        return 1;
    }

    if ((configure_roc || configure_vmm || configure_tds) && reset_roc) {
        std::cout << "Please chose either -R or (-v,-r, -t) options\n";
        std::cout << desc << "\n";
        return 1;
    }

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

    // launch threads
    auto threads = new std::vector< std::future<int> >();

    for (auto & feb : frontend_configs){

      if (max_threads > 0) {
        int n_active = active_threads(threads);
        while(n_active >= max_threads) {
          std::cout << "Too many active threads (" << n_active << "), waiting for fewer than " << max_threads << std::endl;
          sleep(2);
          n_active = active_threads(threads);
        }
      }

      ThreadConfig cfg;
      cfg.configure_vmm     = configure_vmm;
      cfg.configure_roc     = configure_roc;
      cfg.configure_tds     = configure_tds;
      cfg.create_pulses     = create_pulses;
      cfg.readback_tds      = readback_tds;
      cfg.reset_roc         = reset_roc;
      cfg.reset_vmm         = reset_vmm;
      cfg.reset_tds         = reset_tds;
      cfg.vmm_to_unmask     = vmm_to_unmask;
      cfg.channel_to_unmask = channel_to_unmask;
      threads->push_back( std::async(std::launch::async, configure_frontend, feb, cfg) );
    }

    // wait
    for (auto& thread: *threads) {
        try {
            thread.get();
        } catch (ers::Issue & ex) {
            ERS_LOG("Configuration failed due to ers::Issue: " << ex.what());
        } catch (std::exception & ex) {
            ERS_LOG("Configuration failed due to std::exception: " << ex.what());
        }
    }

    return 0;

}

int active_threads(std::vector< std::future<int> >* threads){
  int nfinished = 0;
  for (auto& thread: *threads)
    if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      nfinished++;
  return (int)(threads->size()) - nfinished;
}

int configure_frontend(nsw::FEBConfig feb, ThreadConfig cfg) {

    std::cout << "New thread in configure_frontend for " << feb.getAddress() << std::endl;

    nsw::ConfigSender cs;

    // Reset ROC and exit
    if (cfg.reset_roc) {
        std::cout << "Only resetting ROC" << std::endl;
        auto opc_ip = feb.getOpcServerIp();
        cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 0);
        sleep(1);
        cs.sendGPIO(opc_ip, feb.getAddress() + ".gpio.rocCoreResetN", 1);
        return 0;
    }

    // Send all ROC config
    if (cfg.configure_roc) {
        cs.sendRocConfig(feb);
    }

    // Send all VMM config
    if (cfg.configure_vmm) {
      /// This options are used for ADDC testing
        auto & vmms = feb.getVmms();
        if (cfg.channel_to_unmask != -1 && cfg.vmm_to_unmask != -1) {
            std::cout << "Unmasking channel " << cfg.channel_to_unmask << " in vmm " << cfg.vmm_to_unmask << std::endl;
            vmms[cfg.vmm_to_unmask].setChannelRegisterOneChannel("channel_sm", 0, cfg.channel_to_unmask);
            vmms[cfg.vmm_to_unmask].setGlobalRegister("sm", cfg.channel_to_unmask);
        }

        if (cfg.reset_vmm)
        {
          std::vector <unsigned> reset_ori;
          for (auto & vmm : vmms) {
            reset_ori.push_back(vmm.getGlobalRegister("reset"));  // Set reset bits to 1
            vmm.setGlobalRegister("reset", 3);  // Set reset bits to 1
          }

          cs.sendVmmConfig(feb);  // Sends configuration to all vmm


          size_t i = 0;
          for (auto & vmm : vmms) {
            vmm.setGlobalRegister("reset", reset_ori[i++]);  // Set reset bits to original
          }
        }

        cs.sendVmmConfig(feb);  // Sends configuration to all vmm
        // std::cout << "Vmm:\n" << nsw::bitstringToHexString(vmms[0].getBitString()) << std::endl;
    }

    if (cfg.configure_tds) {
        cs.sendTdsConfig(feb, cfg.reset_tds);  // Sends configuration to all tds
    }

    if (cfg.readback_tds) {
        std::cout << "Reading back TDS. FEB: " << feb.getAddress() << std::endl;
        auto opc_ip = feb.getOpcServerIp();
        auto feb_address = feb.getAddress();
        for (auto tds : feb.getTdss()) {  // Each tds is I2cMasterConfig
            std::cout << "\nTDS: " << tds.getName() << std::endl;
            for (auto tds_i2c_address : tds.getAddresses()) {
                auto address_to_read = nsw::stripReadonly(tds_i2c_address);
                auto size_in_bytes = tds.getTotalSize(tds_i2c_address)/8;
                std::string full_node_name = feb_address + "." + tds.getName()  + "." + address_to_read;
                auto dataread = cs.readI2c(opc_ip, full_node_name , size_in_bytes);
                std::cout << std::dec << "\n";
                tds.decodeVector(tds_i2c_address, dataread);
                std::cout << "Readback as bytes: ";
                for (auto val : dataread) {
                    std::cout << "0x" << std::hex << static_cast<uint32_t>(val) << ", ";
                }
                std::cout << "\n";
            }
        }
    }

    if (cfg.create_pulses) {
        auto opc_ip = feb.getOpcServerIp();
        auto sca_roc_address_analog = feb.getAddress() + "." + feb.getRocAnalog().getName();
        uint8_t data[] = {0};
        for (int i = 0; i < 10; i++) {
            std::cout << "Creating 10 test pulse" << std::endl;
            data[0] = 0xff;
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg124vmmTpInv", data, 1);
            // sleep(1);

            data[0] = 0x0;
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg124vmmTpInv", data, 1);
            // sleep(1);
        }
    }

    return 0;
}
