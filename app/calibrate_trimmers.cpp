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

int NCH_PER_VMM = 64; //10;//64;
int RMS_CUTOFF = 30; // in mV
int TRIM_HI = 31;
int TRIM_LO = 0;
int TRIM_MID = 14;
float SLOPE_CHECK = 1/1.5/1000.; 

bool debug = false;

float take_median(std::vector<float> &v) {
  size_t n = v.size() / 2;
  std::nth_element(v.begin(), v.begin()+n, v.end());
  float median = v[n];
  return median;
}

float sample_to_mV(float sample){
  return sample * 1000. * 1.5; //1.5 is due to a resistor
}

float mV_to_sample(float mV_read){
  return mV_read / 1000. / 1.5; //1.5 is due to a resistor
}

float take_rms(std::vector<float> &v, float mean) {
  float sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
  float stdev = std::sqrt(sq_sum / v.size() - mean * mean);
  return stdev;
}

bool check_channel(float ch_baseline_med, float ch_baseline_rms, float vmm_baseline_med){
  if (sample_to_mV(ch_baseline_rms) > RMS_CUTOFF)
    return false;
  // if (fabs(sample_to_mV(ch_baseline_med - vmm_baseline_med))  > RMS_CUTOFF)
  //   return false;
  return true;
}

std::pair<float,float> get_slopes(float ch_lo, float ch_mid, float ch_hi){
  float m1 = (ch_hi - ch_mid)/(TRIM_HI-TRIM_MID);
  float m2 = (ch_mid - ch_lo)/(TRIM_MID-TRIM_LO);
  return std::make_pair(m1,m2);
}

bool check_slopes(float m1, float m2){
  if ( fabs(m1 - m2) > SLOPE_CHECK )
    return false;
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

    std::cout << std::endl;
    std::cout << "Taking baselines" << std::endl;
    std::cout << std::endl;

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

	std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);
	// add medians, baseline to (MMFE8, CH) map
	channel_baseline_med[feb_ch] = median;
	channel_baseline_rms[feb_ch] = stdev;
	if (debug)
	  std::cout << feb.getAddress() << " vmm" << vmm_id << ", channel " << channel_id << " - mean: " << sample_to_mV(mean) << " , stdev: " << sample_to_mV(stdev) << std::endl;

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

    trims.push_back(TRIM_LO);
    trims.push_back(TRIM_HI);
    trims.push_back(TRIM_MID);

    std::map< std::pair< std::string,int>, float> channel_max_eff_thresh;
    std::map< std::pair< std::string,int>, float> channel_min_eff_thresh;
    std::map< std::pair< std::string,int>, float> channel_mid_eff_thresh;

    std::map< std::pair< std::string,int>, float> channel_max_eff_thresh_err;
    std::map< std::pair< std::string,int>, float> channel_min_eff_thresh_err;
    std::map< std::pair< std::string,int>, float> channel_mid_eff_thresh_err;
    
    std::map< std::string,float > vmm_mid_eff_thresh;
    std::map< std::pair< std::string,int>, float> channel_eff_thresh_slope;

    // missing step --> calculate threshold to be baseline_med + N x baseline_RMS + channel_trim_offset


    std::cout << std::endl;
    std::cout << "Taking trimmers" << std::endl;
    std::cout << std::endl;

    bool flag_trim_in_range = false;

    for (auto & feb : frontend_configs) {
      fe_samples_tmp.clear();
      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
	for (auto trim : trims) {
	  // This function will also configure VMM with correct parameters
	  auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, trim, n_samples);

	  float sum = std::accumulate(results.begin(), results.end(), 0.0);
	  float mean = sum / results.size();
	  float stdev = take_rms(results,mean);
	  float median = take_median(results);

	  // check if channel has a weird RMS or baseline

	  std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);
	  float ch_baseline_rms = channel_baseline_rms[feb_ch];
	  float ch_baseline_med = channel_baseline_med[feb_ch];

	  if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_baseline_med[feb.getAddress()]))
	    continue;

	  // calculate "signal" --> threshold - channel
	  float eff_thresh = median - channel_baseline_med[feb_ch]; 

	  if (debug)
	    std::cout << feb.getAddress() << " vmm" << vmm_id << ", channel " << channel_id << ", trim " << trim << " - med_ch_thresh: " << sample_to_mV(median) << ", rms_ch_thresh: " << sample_to_mV(stdev) << " , med_ch_bl: " << sample_to_mV(channel_baseline_med[feb_ch]) << std::endl;
	   
	  if (trim == TRIM_MID){
	    channel_mid_eff_thresh[feb_ch] = eff_thresh;
	    channel_mid_eff_thresh_err[feb_ch] = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
	    std::cout << "DATA " 
		      << feb.getAddress() 
		      << " " << vmm_id 
		      << " " << channel_id 
		      << " " << tpdac 
		      << " " << thdac 
		      << " " << trim
		      << " " << eff_thresh << std::endl;
	  }
	  else if (trim == TRIM_LO){
	    channel_max_eff_thresh[feb_ch] = eff_thresh;
	    channel_max_eff_thresh_err[feb_ch] = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
	  }
	  else if (trim == TRIM_HI){
	    channel_min_eff_thresh[feb_ch] = eff_thresh;
	    channel_min_eff_thresh_err[feb_ch] = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
	  }
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
      float vmm_eff_thresh = vmm_median - vmm_baseline_med[feb.getAddress()];

      vmm_mid_eff_thresh[feb.getAddress()] = vmm_eff_thresh;

      int good_chs = 0;
      int tot_chs = NCH_PER_VMM;

      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
	std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);
	float min = channel_min_eff_thresh[feb_ch];
	float mid = channel_mid_eff_thresh[feb_ch];
	float max = channel_max_eff_thresh[feb_ch];

	// channel threshold values, not eff. thresh
	float raw_min = min + channel_baseline_med[feb_ch];
	float raw_mid = mid + channel_baseline_med[feb_ch];
	float raw_max = max + channel_baseline_med[feb_ch];

	float m1 = get_slopes(min,mid,max).first;
	float m2 = get_slopes(min,mid,max).second;
	float avg_m = (m1+m2)/2.;

	float raw_m1 = get_slopes(raw_min,raw_mid,raw_max).first;
	float raw_m2 = get_slopes(raw_min,raw_mid,raw_max).second;
	float avg_raw_m = (raw_m1+raw_m2)/2.;


	if(debug)
	  std::cout << feb.getAddress() << " vmm" << vmm_id << ", channel " << channel_id << ", avg_m " << avg_m << ", m1 " << m1 << ", m2 " << m2 << std::endl;
	if (!check_slopes(raw_m1,raw_m2)){
	  avg_m = 0.;
	  tot_chs--;
	  continue;
	}

	channel_eff_thresh_slope[feb_ch] = avg_m;

	float ch_baseline_rms = channel_baseline_rms[std::make_pair(feb.getAddress(), channel_id)];
	float ch_baseline_med = channel_baseline_med[std::make_pair(feb.getAddress(), channel_id)];
	if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_baseline_med[feb.getAddress()])){
	  tot_chs--;
	  continue;
	}
	
	if (debug)
	  std::cout << "min " << sample_to_mV(min) << ", max " << sample_to_mV(max) << ", vmm_eff_thresh " << sample_to_mV(vmm_eff_thresh) << std::endl;
	if ( vmm_eff_thresh < min || vmm_eff_thresh > max ){
	  if (debug)
	    std::cout << "sad! channel " << channel_id << " can't be equalized!" << std::endl;
	}
	else {
	  good_chs++;
	  if (debug)
	    std::cout << ":) channel " << channel_id << " is okay!" << std::endl;
	}	  
      } //end of channel loop
      
      if (debug)
	std::cout << good_chs << " out of " << tot_chs << " are okay!" << std::endl;
      if ((good_chs-tot_chs) < 1){
	if (debug)
	  std::cout << "set trim flag true!" << std::endl;
	flag_trim_in_range = true;
      }
    }

    // trimmed thresholds are equalizable! 
    std::map< std::pair< std::string,int>, int> best_channel_trim;
    if (flag_trim_in_range)
      for (auto & feb : frontend_configs) {
	for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
	  std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);

	  // again check if channel is sensible
	  if (fabs(channel_eff_thresh_slope[feb_ch]) < pow(10,-9.)) {
	    continue;
	  }

	  float ch_baseline_rms = channel_baseline_rms[feb_ch];
	  float ch_baseline_med = channel_baseline_med[feb_ch];
	  if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_baseline_med[feb.getAddress()])){
	    continue;
	  }
	  
	  // guess at a trim value 
	  float delta = channel_mid_eff_thresh[feb_ch] - vmm_mid_eff_thresh[feb.getAddress()];
	  // std::cout << "mid: " << sample_to_mV(channel_mid_eff_thresh[feb_ch]) << std::endl;
	  // std::cout << "min: " << sample_to_mV(channel_min_eff_thresh[feb_ch]) << std::endl;
	  // std::cout << "max: " << sample_to_mV(channel_max_eff_thresh[feb_ch]) << std::endl;
	  // std::cout << "vmm: " << sample_to_mV(vmm_mid_eff_thresh[feb.getAddress()]) << std::endl;
	  // std::cout << "delta: " << sample_to_mV(delta) 
	  // 	    << ", slope: " << channel_eff_thresh_slope[feb_ch] << std::endl;
	  int trim_guess = TRIM_MID + std::round(delta / channel_eff_thresh_slope[feb_ch]);
	  //std::cout << "trim_guess: " << trim_guess << std::endl;
	  best_channel_trim[feb_ch] = trim_guess;


	  auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, trim_guess, n_samples);

	  float sum = std::accumulate(results.begin(), results.end(), 0.0);
	  float mean = sum / results.size();
	  float stdev = take_rms(results,mean);
	  float median = take_median(results);
	  float eff_thresh = median - channel_baseline_med[feb_ch];
	  std::cout << "DATA " 
		    << feb.getAddress() 
		    << " " << vmm_id 
		    << " " << channel_id 
		    << " " << tpdac 
		    << " " << thdac 
		    << " " << trim_guess 
		    << " " << eff_thresh << std::endl;

	}
      }
    else {
      std::cout << "Must change the Global VMM THDAC values" << std::endl;
    }
    return 0;
}