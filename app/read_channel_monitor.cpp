// Sample program to read multiple ADC values from a channel of VMM

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>
#include <thread>
#include <future>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

struct ThreadConfig {
  int targeted_vmm_id;
  int targeted_channel_id;
  int thdac;
  int channel_trim;
  int n_samples;
  bool threshold;
  bool baseline;
  bool dump;
  std::string outdir;
};

int read_channel_monitor(nsw::FEBConfig feb, ThreadConfig cfg);
int active_threads(std::vector< std::future<int> >* threads);
bool too_many_threads(std::vector< std::future<int> >* threads, int max_threads);

int main(int ac, const char *av[]) {

    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";
    std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

    bool dump;
    bool baseline;
    bool threshold;
    int max_threads;
    int n_samples;
    int thdac;
    int channel_trim;
    int targeted_vmm_id;
    int targeted_channel_id;
    std::string config_filename;
    std::string fe_name;
    std::string outdir;
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
        ("outdir,o", po::value<std::string>(&outdir)->
        default_value("./"), "Output directory for dumping text files")
        ("threads", po::value<int>(&max_threads)->
        default_value(16), "Maximum number of threads to run in parallel")
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

    // parse input names
    std::set<std::string> frontend_names;
    if (fe_name != ""){
      if (std::count(fe_name.begin(), fe_name.end(), ',')){
        std::istringstream ss(fe_name);
        while(!ss.eof()){
          std::string buf;
          std::getline(ss, buf, ',');
          if (buf != "")
            frontend_names.emplace(buf);
        }
      }
      else
        frontend_names.emplace(fe_name);
    }
    else
      frontend_names = reader1.getAllElementNames();


    // make FEB objects
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

    // output
    if (dump){
      std::string cmd = "mkdir -p " + outdir;
      system(cmd.c_str());
      std::cout << "Dumping info to " << outdir << std::endl;
    }

    // launch
    auto threads = new std::vector< std::future<int> >();
    for (auto & feb : frontend_configs){
      while(too_many_threads(threads, max_threads))
        sleep(5);
      ThreadConfig cfg;
      cfg.targeted_vmm_id     = targeted_vmm_id;
      cfg.targeted_channel_id = targeted_channel_id;
      cfg.thdac               = thdac;
      cfg.channel_trim        = channel_trim;
      cfg.n_samples           = n_samples;
      cfg.threshold           = threshold;
      cfg.baseline            = baseline;
      cfg.dump                = dump;
      cfg.outdir              = outdir;
      threads->push_back( std::async(std::launch::async, read_channel_monitor, feb, cfg) );
    }

    // wait
    for (auto& thread: *threads)
      thread.get();

    return 0;
}

int read_channel_monitor(nsw::FEBConfig feb, ThreadConfig cfg) {

    nsw::ConfigSender cs;
    // Rongkun: let's not hard-code this
    int VMMS  = feb.getVmms().size();
    int CHS   = 64;
    int tpdac = -1;

    std::ofstream myfile;
    if (cfg.dump)
      myfile.open( cfg.outdir + "/" + (cfg.baseline ? "baselines_" : "thresholds_") + feb.getAddress() + ".txt");

    auto & vmms = feb.getVmms();

    for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

      for (int channel_id = 0; channel_id < CHS; channel_id++) {

        if (cfg.targeted_vmm_id     != -1 && vmm_id     != cfg.targeted_vmm_id)     continue;
        if (cfg.targeted_channel_id != -1 && channel_id != cfg.targeted_channel_id) continue;

        if (cfg.dump)
          std::cout << "INFO "
                    << feb.getAddress() << " "
                    << vmm_id << " "
                    << channel_id << " "
                    << tpdac << " "
                    << cfg.thdac << " "
                    << cfg.channel_trim  << " "
                    << std::endl;

        // configure the VMM
        if (true)                  feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
        if (cfg.threshold)         feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
        if (cfg.baseline)          feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelAnalogOutput);
        if (cfg.channel_trim >= 0) feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(cfg.channel_trim));
        if (cfg.thdac >= 0)        feb.getVmm(vmm_id).setGlobalThreshold((size_t)(cfg.thdac));

        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, cfg.n_samples);

        if (cfg.dump)
          for (auto result: results)
            myfile << "DATA "
                      << feb.getAddress() << " "
                      << vmm_id << " "
                      << channel_id << " "
                      << tpdac << " "
                      << cfg.thdac << " "
                      << cfg.channel_trim  << " "
                      << result << std::endl;

      }
    }

    // reset the MO for all channels
    /*for (int vmm_id = 0; vmm_id < VMMS; vmm_id++)
      vmms[vmm_id].setChannelRegisterAllChannels("channel_smx", 0);
    */
    if (cfg.dump)
      myfile.close();

    return 0;
}

int active_threads(std::vector< std::future<int> >* threads) {
    int nfinished = 0;
    for (auto& thread: *threads)
        if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            nfinished++;
    return (int)(threads->size()) - nfinished;
}

bool too_many_threads(std::vector< std::future<int> >* threads, int max_threads) {
    int nthreads = active_threads(threads);
    bool decision = (nthreads >= max_threads);
    if(decision){
        std::cout << "Too many active threads ("
                  << nthreads
                  << "), waiting for fewer than "
                  << max_threads << std::endl;
    }
    return decision;
}
