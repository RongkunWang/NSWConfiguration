// Sample program to read multiple ADC values from a channel of VMM
// adapted from read_vmm_adc
#include <iostream>
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

int VMMS = 8;
int NCH_PER_VMM = 64; //10//64; //10;//64;
int RMS_CUTOFF = 30; // in mV
int BASELINE_CUTOFF = 30; // in mV
int TRIM_HI = 31;
int TRIM_LO = 0;
int TRIM_MID = 14;
int OFFSET_CENTER = 14; // center the threshold in the middle of the window                                                     
float SLOPE_CHECK = 1/1.5/1000. * 4095.0;
int NCH_ABOVE_THRESH_CUTOFF = 63;
bool debug = true;

float take_median(std::vector<float> &v) {
  size_t n = v.size() / 2;
  std::nth_element(v.begin(), v.begin()+n, v.end());
  float median = v[n];
  return median;
}

float take_median(std::vector<short unsigned int> &v) {
  size_t n = v.size() / 2;
  std::nth_element(v.begin(), v.begin()+n, v.end());
  float median = v[n];
  return median;
}

float sample_to_mV(float sample){
  return sample * 1000. * 1.5 / 4095.0; //1.5 is due to a resistor
}

float sample_to_mV(short unsigned int sample){
  return sample * 1000. * 1.5 / 4095.0; //1.5 is due to a resistor
}

float mV_to_sample(float mV_read){
  return mV_read / 1000. / 1.5 * 4095.0; //1.5 is due to a resistor
}

float take_rms(std::vector<float> &v, float mean) {
  float sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
  float stdev = std::sqrt(sq_sum / v.size() - mean * mean);
  return stdev;
}

float take_rms(std::vector<short unsigned int> &v, float mean) {
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
      return 1023;
    } else if (thdac_guess_variations[i]<0){
      if (debug) std::cout << "WARNING: Capping THDAC value out of range (<0): " << thdac_guess_variations[i] << std::endl;
      thdac_guess_variations[i] = 0;
      return 1023;
    }

    feb.getVmm(vmm_id).setMonitorOutput(nsw::vmm::ThresholdDAC, nsw::vmm::CommonMonitor);
    feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac_guess_variations[i]));
    auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);

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

        feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
        feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
        feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(trim));
        feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);

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

      if(debug)
        std::cout << "Threshold errors (min_thr, mid_thr, max_thr)"
                  << " " << channel_min_eff_thresh_err
                  << " " << channel_mid_eff_thresh_err
                  << " " << channel_max_eff_thresh_err
                  << std::endl;

      float min = channel_min_eff_thresh;
      float mid = channel_mid_eff_thresh;
      float max = channel_max_eff_thresh;

      tmp_min_eff_threshold = min;
      tmp_mid_eff_threshold = mid;
      tmp_max_eff_threshold = max;

      std::pair<float,float> slopes = get_slopes(min,mid,max);
      float m1 = slopes.first;
      float m2 = slopes.second;
      float avg_m = (m1+m2)/2.;

      if(debug) 
        std::cout << "INFO "      << feb.getAddress() 
                  << ", vmm"      << vmm_id 
                  << ", channel " << channel_id 
                  << ", avg_m "   << avg_m 
                  << ", m1 "      << m1 
                  << ", m2 "      << m2 
                  << std::endl;

      if (!check_slopes(m1,m2)){
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

struct ThreadConfig {
  int targeted_vmm_id;
  int n_samples;
  int rms_factor;
  std::string outdir;
};

int calibrate_trimmers(nsw::FEBConfig feb, ThreadConfig cfg);
int active_threads(std::vector< std::future<int> >* threads);
bool too_many_threads(std::vector< std::future<int> >* threads, int max_threads);

int main(int ac, const char *av[]) {
  std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

  std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

  int vmm_id;
  int n_samples;
  int rms_factor;
  int max_threads;
  std::string config_filename;
  std::string fe_name;
  std::string outdir;
  po::options_description desc(description);
  desc.add_options()
      ("help,h", "produce help message")
      ("configfile,c", po::value<std::string>(&config_filename)->default_value(base_folder + "integration_config.json"),
        "Configuration file path")
      ("name,n", po::value<std::string>(&fe_name)->default_value(""),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB).\n"
        "If this option is left empty, all front end elements in the config file will be scanned.")
      ("vmm,V", po::value<int>(&vmm_id)->default_value(-1), "VMM id (0-7)")
      ("samples,s", po::value<int>(&n_samples)->default_value(10), "Number of samples to read")
      ("rms_factor", po::value<int>(&rms_factor)->default_value(-1), "RMS Factor")
      ("outdir,o", po::value<std::string>(&outdir)->default_value("./"), "Output directory for dumping text files")
      ("threads", po::value<int>(&max_threads)->default_value(16), "Maximum number of threads to run in parallel")
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


  std::vector<nsw::FEBConfig> frontend_configs;

  std::cout << "\nFollowing front ends will be configured:\n";
  std::cout <<   "========================================\n";
  for (auto & name : frontend_names) {
    try {
      frontend_configs.emplace_back(reader1.readConfig(name));
      std::cout << name << std::endl;
    } 
    catch (std::exception & e) {
      std::cout << name << " - ERROR: Skipping this FE!"
                << " - Problem constructing configuration due to : " << e.what() << std::endl;
    }
  }

  // output
  std::string cmd = "mkdir -p " + outdir;
  system(cmd.c_str());
  std::cout << "Dumping info to " << outdir << std::endl;

  // launch threads
  auto threads = new std::vector< std::future<int> >();
  for (auto & feb : frontend_configs) {
    while(too_many_threads(threads, max_threads))
      sleep(5);
    ThreadConfig cfg;
    cfg.targeted_vmm_id = vmm_id;
    cfg.n_samples       = n_samples;
    cfg.rms_factor      = rms_factor;
    cfg.outdir          = outdir;
    threads->push_back( std::async(std::launch::async, calibrate_trimmers, feb, cfg) );
  }

  // wait
  for (auto& thread: *threads)
    thread.get();


}

int calibrate_trimmers(nsw::FEBConfig feb, ThreadConfig cfg) {

  nsw::ConfigSender cs;

  // output file
  std::ofstream myfile;
  myfile.open( cfg.outdir + "/" + "trimmers_" + feb.getAddress() + ".txt");

  if (debug) {
    std::ostringstream oss;
    oss << "\nTaking baselines\n" << std::endl;
    std::cout << oss.str();
    myfile    << oss.str();
  }

  int tpdac = -1;

  for (int vmm_id = 0; vmm_id < VMMS; vmm_id++) {

    if (cfg.targeted_vmm_id != -1 && vmm_id != cfg.targeted_vmm_id)
      continue;

    std::map<int, float> channel_baseline_med;
    std::map<int, float> channel_baseline_rms;
    std::map<int, float> channel_mid_eff_thresh;
    std::map<int, int>   channel_trim;
    std::map<int, float> channel_eff_thresh_slope;
    std::map<int, float> channel_trimmer_max;
    std::vector<float> fe_samples_tmp;
    std::vector<float> fe_samples_pruned;
    fe_samples_tmp.clear();
    fe_samples_pruned.clear();

    //////////////////////////////////
    // VMM-level calculations

    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){// channel loop

      // Read pdo of the certain channel n_samples times.
      feb.getVmm(vmm_id).setMonitorOutput(channel_id, nsw::vmm::ChannelMonitor);
      feb.getVmm(vmm_id).setChannelMOMode(channel_id, nsw::vmm::ChannelAnalogOutput);
      auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, cfg.n_samples*10);

      // calculate channel level baseline median, rms
      float sum    = std::accumulate(results.begin(), results.end(), 0.0);
      float mean   = sum / results.size();
      float stdev  = take_rms(results, mean);
      float median = take_median(results);

      // store medians, baseline
      channel_baseline_med[channel_id] = median;
      channel_baseline_rms[channel_id] = stdev;
      if (debug) {
        std::ostringstream oss;
        oss << "INFO "      << feb.getAddress()
            << " vmm"       << vmm_id
            << ", channel " << channel_id
            << " - mean: "  << sample_to_mV(mean)
            << " , stdev: " << sample_to_mV(stdev)
            << std::endl;
        std::cout << oss.str();
        myfile    << oss.str();
      }

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

    fe_samples_pruned.clear();
    fe_samples_tmp.clear();

    //
    //////////////////////////////////

    //////////////////////////////////
    // Global Threshold Calculations

    int thdac_central_guess = cfg.rms_factor * sample_to_mV(vmm_stdev) + sample_to_mV(vmm_median) + OFFSET_CENTER;

    if (debug) {
      std::ostringstream oss;
      oss << "INFO - baseline_mean, baseline_med, baseline_rms, rms_factor: "
          << feb.getAddress() << " vmm"
          << vmm_id << ", "
          << sample_to_mV(vmm_mean) << ", "
          << sample_to_mV(vmm_median) << ", "
          << sample_to_mV(vmm_stdev) << ", "
          << cfg.rms_factor
          << std::endl;
      std::cout << oss.str();
      myfile    << oss.str();
    }

    if (debug) {
      std::ostringstream oss;
      oss << "INFO - Threshold for " << feb.getAddress()
          << " vmm" << vmm_id
          << " is " << thdac_central_guess
          << " in mV, desired"
          << std::endl;
      std::cout << oss.str();
      myfile    << oss.str();
    }

    std::vector<int> thdac_guess_variations;
    thdac_guess_variations.push_back(thdac_central_guess - 20);
    thdac_guess_variations.push_back(thdac_central_guess - 10);
    thdac_guess_variations.push_back(thdac_central_guess);
    thdac_guess_variations.push_back(thdac_central_guess + 10);
    thdac_guess_variations.push_back(thdac_central_guess + 20);

    //
    // the VMM threshold. cherish it.
    //
    int thdac = calculate_thdac_value(cs,feb,vmm_id,cfg.n_samples,thdac_central_guess,thdac_guess_variations);

    if (debug) {
      std::ostringstream oss;
      oss << "INFO - Threshold for " << feb.getAddress()
          << " vmm" << vmm_id
          << " is " << thdac 
          << std::endl;
      std::cout << oss.str();
      myfile    << oss.str();
    }

    // check the threshold
    feb.getVmm(vmm_id).setMonitorOutput  (nsw::vmm::ThresholdDAC, nsw::vmm::CommonMonitor);
    feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
    auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, cfg.n_samples);
    float sum = std::accumulate(results.begin(), results.end(), 0.0);
    float mean = sum / results.size();
    if (debug) {
      std::ostringstream oss;
      oss << "INFO - Threshold for " << feb.getAddress()
          << " vmm" << vmm_id
          << " is " << sample_to_mV(mean) << " in mV"
          << std::endl;
      std::cout << oss.str();
      myfile    << oss.str();
    }

    //
    //////////////////////////////////


    //////////////////////////////////
    // Get VMM-level averages.
    fe_samples_tmp.clear();
    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){

      feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
      feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
      feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(TRIM_MID));
      feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
      auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, cfg.n_samples);

      // add samples to the vector for a given fe
      for (unsigned int i = 0; i < results.size(); i++) {
        fe_samples_tmp.push_back((float)(results[i]));
      }
    }

    // find the median eff_thresh value for a given FE, vmm
    // size_t vmm_n = fe_samples_tmp.size() / 2;
    // std::nth_element(fe_samples_tmp.begin(), fe_samples_tmp.begin()+vmm_n, fe_samples_tmp.end());
    float vmm_median_threshold_trim_mid = take_median(fe_samples_tmp);
    float vmm_eff_thresh = vmm_median_threshold_trim_mid - vmm_median;

    if (debug) {
      std::ostringstream oss;
      oss << "INFO - VMM, vmm_median_threshold_trim_mid, vmm_baseline_median, vmm_eff_thresh "
          << vmm_id                                      << ", "
          << sample_to_mV(vmm_median_threshold_trim_mid) << ", "
          << sample_to_mV(vmm_median)                    << ", "
          << sample_to_mV(vmm_eff_thresh)                << ", "
          << std::endl;
      std::cout << oss.str();
      myfile    << oss.str();
    }

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

    for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){

      // check if channel has a weird RMS or baseline
      float ch_baseline_rms = channel_baseline_rms[channel_id];
      float ch_baseline_med = channel_baseline_med[channel_id];

      if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_median))
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
                            cfg.n_samples,
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
        std::ostringstream oss;
        oss << "Failed to find a linear region" << std::endl;
        std::cout << oss.str();
        myfile    << oss.str();
        tot_chs--;
        continue;
      }

      /////////////////////////////////////////

      channel_eff_thresh_slope[channel_id] = slopeAndMax.first;
      channel_trimmer_max[channel_id]      = slopeAndMax.second;
      channel_mid_eff_thresh[channel_id]   = tmp_mid_eff_threshold;

      ch_baseline_rms = channel_baseline_rms[channel_id];
      ch_baseline_med = channel_baseline_med[channel_id];
      if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_median)){
        tot_chs--;
        continue;
      }

      if (debug) {
        std::ostringstream oss;
        oss << "INFO min "         << sample_to_mV(tmp_min_eff_threshold)
            << ", max "            << sample_to_mV(tmp_max_eff_threshold)
            << ", vmm_eff_thresh " << sample_to_mV(vmm_eff_thresh)
            << std::endl;
        std::cout << oss.str();
        myfile    << oss.str();
      }

      if ( vmm_eff_thresh < tmp_min_eff_threshold || vmm_eff_thresh > tmp_max_eff_threshold ){
        if (debug) {
          std::ostringstream oss;
          oss << "INFO sad! channel " << channel_id << " can't be equalized!" << std::endl;
          std::cout << oss.str();
          myfile    << oss.str();
        }
      }
      else {
        good_chs++;
        if (debug) {
          std::ostringstream oss;
          oss << "INFO :) channel " << channel_id << " is okay!" << std::endl;
          std::cout << oss.str();
          myfile    << oss.str();
        }
      }



    } // end of channel loop

    // catch any cases where channel thresh - channel baseline is negative
    // note that this is for a channel trim value @ TRIM_MID
    // if channel decides on another channel trim value, eff_thresh could still be neg.

    if (nch_base_above_thresh > NCH_ABOVE_THRESH_CUTOFF){
      std::ostringstream oss;
      oss << "Error: Threshold is too low! ";
      oss << "N(ch) where eff. thresh is negative: " << nch_base_above_thresh;
      std::cout << oss.str() << std::endl;
      myfile    << oss.str() << std::endl;
      return 0;
    }

    //
    //////////////////////////////////



    //////////////////////////////////
    // Trimmer Analysis

    if (debug) {
      std::ostringstream oss;
      oss << "INFO " << good_chs << " out of " << tot_chs << " are okay!";
      std::cout << oss.str() << std::endl;
      myfile    << oss.str() << std::endl;
    }
    if ((good_chs-tot_chs) < 1){
      if (debug) {
        std::ostringstream oss;
        oss << "INFO set trim flag true!";
        std::cout << oss.str() << std::endl;
        myfile    << oss.str() << std::endl;
      }
      flag_trim_in_range = true;
    }

    if (flag_trim_in_range){ // trimmed thresholds are equalizable!
      for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){ // channel loop

        // again check if channel is sensible
        if (fabs(channel_eff_thresh_slope[channel_id]) < pow(10,-9.)) {
          continue;
        }

        float ch_baseline_rms = channel_baseline_rms[channel_id];
        float ch_baseline_med = channel_baseline_med[channel_id];
        if (!check_channel(ch_baseline_med, ch_baseline_rms, vmm_median)){
          continue;
        }

        // guess at a trim value
        float delta = channel_mid_eff_thresh[channel_id] - vmm_eff_thresh;
        int trim_guess = TRIM_MID + std::round(delta / channel_eff_thresh_slope[channel_id]);

        // cap off the guess for trimmer value to avoid non-linear region.
        if (trim_guess > channel_trimmer_max[channel_id])
          channel_trim[channel_id] = channel_trimmer_max[channel_id];
        else if (trim_guess < 0)
          channel_trim[channel_id] = 0;
        else
          channel_trim[channel_id] = trim_guess;

        //channel_trim[channel_id] = trim_guess > channel_trimmer_max[channel_id] ? channel_trimmer_max[channel_id] : trim_guess;
        //channel_trim[channel_id] = channel_trim[channel_id] < 0                 ? 0 : channel_trim[channel_id];

        feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
        feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
        feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(channel_trim[channel_id]));
        feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
        auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, cfg.n_samples);

        // unused for now
        // float sum = std::accumulate(results.begin(), results.end(), 0.0);
        // float mean = sum / results.size();
        // float stdev = take_rms(results,mean);

        float median = take_median(results);
        float eff_thresh = median - channel_baseline_med[channel_id];
        std::ostringstream oss;
        oss << "DATA "
            << feb.getAddress()
            << " " << vmm_id
            << " " << channel_id
            << " " << tpdac
            << " " << thdac
            << " " << TRIM_MID
            << " " << channel_mid_eff_thresh[channel_id]
            << " " << channel_trim[channel_id]
            << " " << eff_thresh
            << std::endl;
        std::cout << oss.str();
        myfile    << oss.str();

      }
    }
    else {
      std::cout << "Must change the Global VMM THDAC values" << std::endl;
    }
  }

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
