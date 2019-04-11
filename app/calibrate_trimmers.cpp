// Sample program to read multiple ADC values from a channel of VMM
// adapted from read_vmm_adc
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

int NCH_PER_VMM = 64;
int RMS_CUTOFF = 30; // in mV

float take_median(std::vector<float> v) {
  size_t n = v.size() / 2;
  std::nth_element(v.begin(), v.begin()+n, v.end());
  float median = v[n];
  return median;
}

float take_rms(std::vector<float> v, float mean) {
  float sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
  float stdev = std::sqrt(sq_sum / v.size() - mean * mean);
  return stdev;
}

float sample_to_mV(float sample){
  return sample * 1000. * 1.5; //1.5 is due to a resistor
}

bool check_channel(std::string addr, int channel){
  return true;
}

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

    int vmm_id;
    int n_samples;
    int thdac;
    int tpdac;
    int channel_trim;
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
        ("samples,s", po::value<int>(&n_samples)->
        default_value(10), "Number of samples to read")
        ("thdac", po::value<int>(&thdac)->
        default_value(-1), "Threshold DAC")
        ("tpdac", po::value<int>(&tpdac)->
        default_value(-1), "Test pulse DAC")
        ("channel_trim", po::value<int>(&channel_trim)->
        default_value(-1), "Channel trimming DAC")
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

    std::map< std::pair< std::string,int>, float> channel_baseline_med;
    std::map< std::pair< std::string,int>, float> channel_baseline_rms;

    std::map< std::string, float> vmm_baseline_med;
    std::map< std::string, float> vmm_baseline_rms;

    int offset_center = 15; // center the threshold in the middle of the window

    std::vector <float> fe_samples_tmp;
    
    // first read baselines
    for (auto & feb : frontend_configs) {
      fe_samples_tmp.clear(); 
      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
        // Read pdo of the certain channel n_samples times.
        // This function will also configure VMM with correct parameters
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, -1, -1, -1, n_samples);

	// calculate channel level baseline median, rms
        float sum    = std::accumulate(results.begin(), results.end(), 0.0);
        float mean   = sum / results.size();
        float stdev  = take_rms(results, mean);
	float median = take_median(results);

	// add medians, baseline to (MMFE8, CH) map
	channel_baseline_med[std::make_pair(feb.getAddress(),channel_id)] = median;
	channel_baseline_rms[std::make_pair(feb.getAddress(),channel_id)] = stdev;
	// if RMS of channel is too large, it's probably crap, ignore it
	// potentially output a list of channels to mask?
	if ( sample_to_mV(stdev) > RMS_CUTOFF )
	  continue;
	// add samples to the vector for a given fe which will be used
	// to calculate a vmm-level median, rms
	for (unsigned int i = 0; i < results.size(); i++) {
	  fe_samples_tmp.push_back(results[i]);
	}
      }

      // vmm level calculations
      float vmm_sum    = std::accumulate(fe_samples_tmp.begin(), fe_samples_tmp.end(), 0.0);
      float vmm_mean   = vmm_sum / fe_samples_tmp.size();
      float vmm_stdev  = take_rms(fe_samples_tmp, vmm_mean);
      float vmm_median = take_median(fe_samples_tmp);

      // add medians, baseline to MMFE8 --> med, stdev map
      vmm_baseline_med[feb.getAddress()] = vmm_median;
      vmm_baseline_rms[feb.getAddress()] = vmm_stdev;
    }

    // then, take trimmer values

    // loop through trims to get full range, then then middle of range
    std::vector<int> trims;

    int TRIM_HI = 31;
    int TRIM_LO = 0;
    int TRIM_MID = 14;

    trims.push_back(TRIM_LO);
    trims.push_back(TRIM_HI);
    trims.push_back(TRIM_MID);

    std::map< std::pair< std::string,int>, float> channel_max_eff_thresh;
    std::map< std::pair< std::string,int>, float> channel_min_eff_thresh;
    std::map< std::pair< std::string,int>, float> channel_mid_eff_thresh;
    
    for (auto & feb : frontend_configs) {
      fe_samples_tmp.clear();
      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
	for (auto trim : trims) {
	  // This function will also configure VMM with correct parameters
	  auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, trim, n_samples);

	  float sum = std::accumulate(results.begin(), results.end(), 0.0);
	  float mean = sum / results.size();
	  float median = take_median(results);

	  // check if channel has a weird RMS or baseline
	  if (!check_channel(feb.getAddress(), channel_id))
	    continue;

	  // calculate "signal" --> threshold - channel
	  float eff_thresh = median - channel_baseline_med[std::make_pair(feb.getAddress(),channel_id)]; 

	  if (trim == TRIM_MID)
	    channel_mid_eff_thresh[std::make_pair(feb.getAddress(),channel_id)] = eff_thresh;
	  else if (trim == TRIM_LO)
	    channel_min_eff_thresh[std::make_pair(feb.getAddress(),channel_id)] = eff_thresh;
	  else if (trim == TRIM_HI)
	    channel_max_eff_thresh[std::make_pair(feb.getAddress(),channel_id)] = eff_thresh;
	  if (trim == TRIM_MID){
	    // add samples to the vector for a given fe
	    for (unsigned int i = 0; i < results.size(); i++) {
	      fe_samples_tmp.push_back(results[i]);
	    }
	  }

	} // end of trim loop
      } // end of channel loop

      // find the median eff_thresh value for a given FE, vmm
      size_t vmm_n = fe_samples_tmp.size() / 2;
      std::nth_element(fe_samples_tmp.begin(), fe_samples_tmp.begin()+vmm_n, fe_samples_tmp.end());
      float vmm_median = take_median(fe_samples_tmp);

      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
	float min = channel_min_eff_thresh[std::make_pair(feb.getAddress(),channel_id)];
	float max = channel_max_eff_thresh[std::make_pair(feb.getAddress(),channel_id)];
	if ( vmm_median < min && vmm_median > max )
	  std::cout << "sad! channel " << channel_id << " can't be equalized!" << std::endl;
	else
	  std::cout << ":) channel " << channel_id << " is okay!" << std::endl;
	  
      }
    }

    return 0;
}
