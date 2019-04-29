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

int NCH_PER_VMM = 64; //10//64; //10;//64;
int RMS_CUTOFF = 30; // in mV
int BASELINE_CUTOFF = 30; // in mV
int TRIM_HI = 31;
int TRIM_LO = 0;
int TRIM_MID = 14;
float SLOPE_CHECK = 1/1.5/1000.;
int NCH_ABOVE_THRESH_CUTOFF = 1;
bool debug = true;

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

std::pair<float,float> get_slopes(float ch_lo,
                                  float ch_mid,
                                  float ch_hi,
                                  int trim_hi  = TRIM_HI,
                                  int trim_mid = TRIM_MID,
                                  int trim_lo  = TRIM_LO){
  float m1 = (ch_hi - ch_mid)/(trim_hi-trim_mid);
  float m2 = (ch_mid - ch_lo)/(trim_mid-trim_lo);
  return std::make_pair(m1,m2);
}

bool check_slopes(float m1, float m2){
  if ( fabs(m1 - m2) > SLOPE_CHECK )
    return false;
  return true;
}


int calculate_thdac_value(nsw::ConfigSender & cs,
                          nsw::FEBConfig & feb,
                          int vmm_id,
                          int n_samples,
                          int thdac_central_guess,
                          std::vector<int> & thdac_guess_variations){
  std::vector<float> thdac_guesses_sample;

  for (unsigned int i = 0; i < thdac_guess_variations.size(); i++){
    if (thdac_guess_variations[i]>1023){
      if (debug) std::cout << "WARNING: Capping THDAC value out of range (>1023): " << thdac_guess_variations[i] << std::endl;
      thdac_guess_variations[i] = 1023;
    } else if (thdac_guess_variations[i]<0){
      if (debug) std::cout << "WARNING: Capping THDAC value out of range (<0): " << thdac_guess_variations[i] << std::endl;
      thdac_guess_variations[i] = 0;
    }

    auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, 0, thdac_guess_variations[i], -1, -1, n_samples);
    float sum = std::accumulate(results.begin(), results.end(), 0.0);
    float mean = sum / results.size();
    thdac_guesses_sample.push_back(mean);
    if (debug)
      std::cout << "INFO "
          << feb.getAddress()
          << " vmm" << vmm_id
          << ", thdac " << thdac_guess_variations[i]
          << ", thdac (mV) " << sample_to_mV(mean)
          << std::endl;
  }

  // do fit to a line
  float thdac_guess_mean = std::accumulate(thdac_guess_variations.begin(), thdac_guess_variations.end(), 0.0)/thdac_guess_variations.size();
  float thdac_guess_sample_mean = std::accumulate(thdac_guesses_sample.begin(), thdac_guesses_sample.end(), 0.0)/thdac_guesses_sample.size();

  float num = 0.;
  float denom = 0.;
  for (unsigned int i = 0; i < thdac_guess_variations.size(); i++){
    num += (thdac_guess_variations[i]-thdac_guess_mean) * (thdac_guesses_sample[i]-thdac_guess_sample_mean);
    denom += pow((thdac_guess_variations[i]-thdac_guess_mean),2);
  }

  float thdac_slope = num/denom;
  float thdac_intercept = thdac_guess_sample_mean - thdac_slope * thdac_guess_mean;

  // (y-b) / m = x
  int thdac = (mV_to_sample(thdac_central_guess) - thdac_intercept)/thdac_slope;

  return thdac;
}


std::pair<float,int> find_linear_region_slope(nsw::ConfigSender & cs,
                            nsw::FEBConfig & feb,
                            int vmm_id,
                            int channel_id,
                            int thdac,
                            int tpdac,
                            int n_samples,
                            float ch_baseline_med,
                            float ch_baseline_rms,
                            float & tmp_min_eff_threshold,
                            float & tmp_mid_eff_threshold,
                            float & tmp_max_eff_threshold,
                            int & nch_base_above_thresh,
                            int trim_hi=TRIM_HI,
                            int trim_mid=TRIM_MID,
                            int trim_lo=TRIM_LO
                            ){

      if (trim_hi <= trim_mid) return std::make_pair(0,0);
      if (trim_mid <= trim_lo) return std::make_pair(0,0);

      float channel_mid_eff_thresh=0., channel_max_eff_thresh=0., channel_min_eff_thresh=0.;
      float channel_mid_eff_thresh_err=0., channel_max_eff_thresh_err=0., channel_min_eff_thresh_err=0.;

      // loop through trims to get full range, then then middle of range
      std::vector<int> trims;

      trims.push_back(trim_lo );
      trims.push_back(trim_hi );
      trims.push_back(trim_mid);

      for (auto trim : trims) {

        // This function will also configure VMM with correct parameters
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, trim, n_samples);

        float sum = std::accumulate(results.begin(), results.end(), 0.0);
        float mean = sum / results.size();
        float stdev = take_rms(results,mean);
        float median = take_median(results);

        // calculate "signal" --> threshold - channel
        float eff_thresh = median - ch_baseline_med;

        if (debug) std::cout << "INFO " << feb.getAddress()
                                        << " vmm"
                                        << vmm_id
                                        << ", channel "
                                        << channel_id
                                        << ", trim "
                                        << trim
                                        << " - med_ch_thresh: "
                                        << sample_to_mV(median)
                                        << ", rms_ch_thresh: "
                                        << sample_to_mV(stdev)
                                        << " , med_ch_bl: "
                                        << sample_to_mV(ch_baseline_med)
                                        << std::endl;

        if (trim == trim_mid){
          channel_mid_eff_thresh = eff_thresh;
          channel_mid_eff_thresh_err = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
          // std::cout << "DATA "
          //     << feb.getAddress()
          //     << " " << vmm_id
          //     << " " << channel_id
          //     << " " << tpdac
          //     << " " << thdac
          //     << " " << trim
          //     << " " << eff_thresh << std::endl;
        }
        else if (trim == trim_lo){
          channel_max_eff_thresh = eff_thresh;
          channel_max_eff_thresh_err = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
        }
        else if (trim == trim_hi){
          channel_min_eff_thresh = eff_thresh;
          channel_min_eff_thresh_err = std::sqrt( pow(stdev,2.) + pow(ch_baseline_rms,2.) );
        }
      } // end of trim loop

      float min = channel_min_eff_thresh;
      float mid = channel_mid_eff_thresh;
      float max = channel_max_eff_thresh;

      tmp_min_eff_threshold = min;
      tmp_mid_eff_threshold = mid;
      tmp_max_eff_threshold = max;

      // channel threshold values, not eff. thresh
      float raw_min = min + ch_baseline_med;
      float raw_mid = mid + ch_baseline_med;
      float raw_max = max + ch_baseline_med;

      std::pair<float,float> slopes = get_slopes(min,mid,max);
      float m1 = slopes.first;
      float m2 = slopes.second;
      float avg_m = (m1+m2)/2.;

      std::pair<float,float> rawSlopes = get_slopes(raw_min,raw_mid,raw_max);
      float raw_m1 = rawSlopes.first;
      float raw_m2 = rawSlopes.second;
      float avg_raw_m = (raw_m1+raw_m2)/2.;


      if(debug) std::cout << "INFO " << feb.getAddress() << " vmm" << vmm_id << ", channel " << channel_id << ", avg_m " << avg_m << ", m1 " << m1 << ", m2 " << m2 << std::endl;
      if (!check_slopes(raw_m1,raw_m2)){
        return find_linear_region_slope(cs,
                            feb,
                            vmm_id,
                            channel_id,
                            thdac,
                            tpdac,
                            n_samples,
                            ch_baseline_med,
                            ch_baseline_rms,
                            tmp_min_eff_threshold,
                            tmp_mid_eff_threshold,
                            tmp_max_eff_threshold,
                            nch_base_above_thresh,
                            trim_hi-1,
                            trim_mid,
                            trim_lo
                            );
      }

      // Got to a consistent set of two slopes!
      if (channel_mid_eff_thresh < 0.) nch_base_above_thresh++;
      return std::make_pair(avg_m,trim_hi);
}





int main(int ac, const char *av[]) {
  std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

  std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

  int vmm_id;
  int n_samples;
  int rms_factor;
  int tpdac = -1;
  int channel_trim = -1;
  std::string config_filename;
  std::string fe_name;
  po::options_description desc(description);
  desc.add_options()
      ("help,h", "produce help message")
      ("configfile,c", po::value<std::string>(&config_filename)->default_value(base_folder + "integration_config.json"),
        "Configuration file path")
      ("name,n", po::value<std::string>(&fe_name)->default_value(""),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB).\n"
        "If this option is left empty, all front end elements in the config file will be scanned.")
      ("vmm,V", po::value<int>(&vmm_id)->default_value(0), "VMM id (0-7)")
      ("samples,s", po::value<int>(&n_samples)->default_value(10), "Number of samples to read")
      ("rms_factor", po::value<int>(&rms_factor)->default_value(-1), "RMS Factor")
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

  int offset_center = 14; // center the threshold in the middle of the window

  std::vector <float> fe_samples_tmp;
  std::vector <float> fe_samples_pruned;

  // first read baselines

  // calculate thdac
  std::map<std::string,int> thdacs;
  std::map<std::string,float> thdacs_sample;

  // then, take trimmer values


  std::map< std::pair< std::string,int>, float> channel_max_eff_thresh;
  std::map< std::pair< std::string,int>, float> channel_min_eff_thresh;
  std::map< std::pair< std::string,int>, float> channel_mid_eff_thresh;

  std::map< std::pair< std::string,int>, float> channel_max_eff_thresh_err;
  std::map< std::pair< std::string,int>, float> channel_min_eff_thresh_err;
  std::map< std::pair< std::string,int>, float> channel_mid_eff_thresh_err;

  std::map< std::string,float > vmm_mid_eff_thresh;
  std::map< std::pair< std::string,int>, float> channel_eff_thresh_slope;
  std::map< std::pair< std::string,int>, float> channel_trimmer_max;


  std::cout << "\nTaking baselines\n" << std::endl;

  for (auto & feb : frontend_configs) { // big feb loop

    //////////////////////////////////
    // VMM-level calculations

    fe_samples_tmp.clear();
    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){// channel loop
      // Read pdo of the certain channel n_samples times.
      // This function will also configure VMM with correct parameters
      auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, -1, -1, -1, n_samples*10);

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
        std::cout << "INFO "      << feb.getAddress()
                  << " vmm"       << vmm_id
                  << ", channel " << channel_id
                  << " - mean: "  << sample_to_mV(mean)
                  << " , stdev: " << sample_to_mV(stdev)
                  << std::endl;

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

    // remove samples which are highly suspicious
    // this can happen when a channel has a very high or low baseline, but with small RMS
    float tmp_median = take_median(fe_samples_tmp);
    fe_samples_pruned.clear();
    for (auto sample: fe_samples_tmp)
      if (fabs(sample_to_mV(sample - tmp_median)) < BASELINE_CUTOFF)
        fe_samples_pruned.push_back(sample);

    // calculate VMM median baseline and rms
    float vmm_sum    = std::accumulate(fe_samples_pruned.begin(), fe_samples_pruned.end(), 0.0);
    float vmm_mean   = vmm_sum / fe_samples_pruned.size();
    float vmm_stdev  = take_rms(fe_samples_pruned, vmm_mean);
    float vmm_median = take_median(fe_samples_pruned);

    // add medians, baseline to MMFE8 --> med, stdev map
    vmm_baseline_med[feb.getAddress()] = vmm_median;
    vmm_baseline_rms[feb.getAddress()] = vmm_stdev;

    fe_samples_pruned.clear();
    fe_samples_tmp.clear();
    //
    //////////////////////////////////



    //////////////////////////////////
    // Global Threshold Calculations

    int thdac_central_guess = rms_factor * sample_to_mV(vmm_baseline_rms[feb.getAddress()]) + sample_to_mV(vmm_baseline_med[feb.getAddress()]) + offset_center;

    if (debug)
      std::cout << "INFO - baseline_mean, baseline_med, baseline_rms, rms_factor: "
                << feb.getAddress() << " vmm"
                << vmm_id << ", "
                << sample_to_mV(vmm_mean) << ", "
                << sample_to_mV(vmm_baseline_med[feb.getAddress()]) << ", "
                << sample_to_mV(vmm_baseline_rms[feb.getAddress()]) << ", "
                << rms_factor
                << std::endl;

    if (debug)
      std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << thdac_central_guess << " in mV, desired" << std::endl;

    std::vector<int> thdac_guess_variations;

    thdac_guess_variations.push_back(thdac_central_guess - 20);
    thdac_guess_variations.push_back(thdac_central_guess - 10);
    thdac_guess_variations.push_back(thdac_central_guess);
    thdac_guess_variations.push_back(thdac_central_guess + 10);
    thdac_guess_variations.push_back(thdac_central_guess + 20);

    int thdac = calculate_thdac_value(cs,feb,vmm_id,n_samples,thdac_central_guess,thdac_guess_variations);

    thdacs[feb.getAddress()] = thdac;

    if (debug)
      std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << thdac << std::endl;

    auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, 0, thdac, -1, -1, n_samples);
    float sum = std::accumulate(results.begin(), results.end(), 0.0);
    float mean = sum / results.size();
    thdacs_sample[feb.getAddress()] = mean;

    if (debug)
      std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << sample_to_mV(mean) << " in mV" <<  std::endl;

    //
    //////////////////////////////////


    //////////////////////////////////
    // Get VMM-level averages.
    fe_samples_tmp.clear();
    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
      thdac = thdacs[feb.getAddress()];
      auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, TRIM_MID, n_samples);

      // add samples to the vector for a given fe
      for (unsigned int i = 0; i < results.size(); i++) {
        fe_samples_tmp.push_back(results[i]);
      }
    }

    // find the median eff_thresh value for a given FE, vmm
    size_t vmm_n = fe_samples_tmp.size() / 2;
    std::nth_element(fe_samples_tmp.begin(), fe_samples_tmp.begin()+vmm_n, fe_samples_tmp.end());
    float vmm_median_trim_mid = take_median(fe_samples_tmp);
    float vmm_eff_thresh = vmm_median_trim_mid - vmm_baseline_med[feb.getAddress()];

    vmm_mid_eff_thresh[feb.getAddress()] = vmm_eff_thresh;

    if (debug)
      std::cout << "INFO - VMM, vmm_median_trim_mid, vmm_median, vmm_eff_thresh "
                << vmm_id << ", "
                << sample_to_mV(vmm_median_trim_mid) << ", "
                << sample_to_mV(vmm_median)          << ", "
                << sample_to_mV(vmm_eff_thresh)      << ", "
                << std::endl;

    //
    //////////////////////////////////



    //////////////////////////////////
    // Scanning trimmers

    // count how many channels have baselines above the threshold
    int nch_base_above_thresh = 0;

    std::cout << "\nTaking trimmers\n" << std::endl;

    bool flag_trim_in_range = false;

    int good_chs = 0;
    int tot_chs = NCH_PER_VMM;

    thdac = thdacs[feb.getAddress()];

    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){

      // check if channel has a weird RMS or baseline

      std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);
      float ch_baseline_rms = channel_baseline_rms[feb_ch];
      float ch_baseline_med = channel_baseline_med[feb_ch];

      if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_baseline_med[feb.getAddress()]))
        continue;

      /////////////////////////////////////
      float tmp_min_eff_threshold = 0.;
      float tmp_mid_eff_threshold = 0.;
      float tmp_max_eff_threshold = 0.;

      std::pair<float,int> slopeAndMax = find_linear_region_slope(cs,
                            feb,
                            vmm_id,
                            channel_id,
                            thdac,
                            tpdac,
                            n_samples,
                            ch_baseline_med,
                            ch_baseline_rms,
                            tmp_min_eff_threshold,
                            tmp_mid_eff_threshold,
                            tmp_max_eff_threshold,
                            nch_base_above_thresh,
                            TRIM_HI,
                            TRIM_MID,
                            TRIM_LO
                            );

      if(slopeAndMax.first==0){
        std::cout << "Failed to find a linear region" << std::endl;
        tot_chs--;
        continue;
      }

      /////////////////////////////////////////

      channel_eff_thresh_slope[feb_ch] = slopeAndMax.first;
      channel_trimmer_max[feb_ch]      = slopeAndMax.second;
      channel_mid_eff_thresh[feb_ch]   = tmp_mid_eff_threshold;

      ch_baseline_rms = channel_baseline_rms[std::make_pair(feb.getAddress(), channel_id)];
      ch_baseline_med = channel_baseline_med[std::make_pair(feb.getAddress(), channel_id)];
      if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_baseline_med[feb.getAddress()])){
        tot_chs--;
        continue;
      }

      if (debug) std::cout << "INFO min "
                            << sample_to_mV(tmp_min_eff_threshold)
                            << ", max "
                            << sample_to_mV(tmp_max_eff_threshold)
                            << ", vmm_eff_thresh "
                            << sample_to_mV(vmm_eff_thresh) << std::endl;

      if ( vmm_eff_thresh < tmp_min_eff_threshold || vmm_eff_thresh > tmp_max_eff_threshold ){
        if (debug) std::cout << "INFO sad! channel " << channel_id << " can't be equalized!" << std::endl;
      }
      else {
        good_chs++;
        if (debug) std::cout << "INFO :) channel " << channel_id << " is okay!" << std::endl;
      }



    } // end of channel loop

    // catch any cases where channel thresh - channel baseline is negative
    // note that this is for a channel trim value @ TRIM_MID
    // if channel decides on another channel trim value, eff_thresh could still be neg.

    if (nch_base_above_thresh > NCH_ABOVE_THRESH_CUTOFF){
      std::cout << "Error: Threshold is too low!" << std::endl;
      std::cout << "N(ch) where eff. thresh is negative: " << nch_base_above_thresh << std::endl;
      return 0;
    }

    //
    //////////////////////////////////



    //////////////////////////////////
    // Trimmer Analysis

    if (debug) std::cout << "INFO " << good_chs << " out of " << tot_chs << " are okay!" << std::endl;
    if ((good_chs-tot_chs) < 1){
      if (debug) std::cout << "INFO set trim flag true!" << std::endl;
      flag_trim_in_range = true;
    }

    std::map< std::pair< std::string,int>, int> best_channel_trim;
    if (flag_trim_in_range){ // trimmed thresholds are equalizable!
      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){ // channel loop
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
        //      << ", slope: " << channel_eff_thresh_slope[feb_ch] << std::endl;
        int trim_guess = TRIM_MID + std::round(delta / channel_eff_thresh_slope[feb_ch]);
        //std::cout << "trim_guess: " << trim_guess << std::endl;

        // cap off the guess for trimmer value to avoid non-linear region.
        best_channel_trim[feb_ch] = trim_guess > channel_trimmer_max[feb_ch] ? channel_trimmer_max[feb_ch] : trim_guess;
        best_channel_trim[feb_ch] = best_channel_trim[feb_ch] < 0            ? 0 : best_channel_trim[feb_ch];

        int thdac = thdacs[feb.getAddress()];
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, channel_id, thdac, tpdac, best_channel_trim[feb_ch], n_samples);

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
            << " " << TRIM_MID
            << " " << channel_mid_eff_thresh[feb_ch]
            << " " << best_channel_trim[feb_ch]
            << " " << eff_thresh << std::endl;
      }
    } else {
      std::cout << "Must change the Global VMM THDAC values" << std::endl;
    }
  }

  return 0;
}







