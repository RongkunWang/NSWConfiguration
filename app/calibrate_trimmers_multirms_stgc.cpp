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
int RMS_CUTOFF = 10;      // in mV
int BASELINE_CUTOFF = 10; // in mV
int TRIM_HI = 31;
int TRIM_LO = 0;
int TRIM_MID = 14;
float SLOPE_CHECK = 1/1.5/1000. * 4095.0;
int NCH_ABOVE_THRESH_CUTOFF = 63;
bool isMM = false;
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
    float factor = (isMM) ? 1.5 : 1.0;
    return sample * 1000. * factor / 4095.0;
    // return sample * 1000. * 1.0 / 4095.0;
    // return sample * 1000. * 1.5 / 4095.0; //1.5 is due to a resistor
}

float sample_to_mV(short unsigned int sample){
    float factor = (isMM) ? 1.5 : 1.0;
    return sample * 1000. * factor / 4095.0;
    // return sample * 1000. * 1.0 / 4095.0;
    // return sample * 1000. * 1.5 / 4095.0; //1.5 is due to a resistor
}

float mV_to_sample(float mV_read){
    float factor = (isMM) ? 1.5 : 1.0;
    return mV_read / 1000. / factor * 4095.0;
    // return mV_read / 1000. / 1.0 * 4095.0;
    // return mV_read / 1000. / 1.5 * 4095.0; //1.5 is due to a resistor
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

//--------------------------------------------------------------------//
//              Calculate all the thdac for all FEBs
//--------------------------------------------------------------------//
//std::map< std::pair< std::string,int>, int> calculate_thdac_value(nsw::ConfigSender & cs,
int calculate_thdac_value(nsw::ConfigSender & cs,
                          std::vector<nsw::FEBConfig> frontend_configs,
                          int vmm_id,
                          int n_samples,
                          std::map< std::pair< std::string,int>, int> thdac_central_guesses, 
                          std::vector<int> variations){

    /*

    std::map< std::pair<std::string,int>, std::vector<float> > thdac_guesses_vars_okay;
    std::map< std::pair<std::string,int>, std::vector<float> > thdac_guesses_sample;

    //--------------------------------------------------------------------//
    //        Get all thdac quess variation on all vmms on all boards
    //--------------------------------------------------------------------//

    std::map< std::pair<std::string,int>, std::vector<int> > thdac_central_variations_all_febs;

    for (auto & feb : frontend_configs) {

        for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {
            
            std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);
            int thdac_central_guess = thdac_central_guesses[feb_vmm];

            std::vector<int> thdac_guess_variations;
            
            for ( uint ivar=0; ivar<variation.size(); ivar++ ) {   
                thdac_guess_variations.push_back(thdac_central_guess + variations.at(ivar) );
            }

            thdac_central_variations_all_febs[feb_vmm] = thdac_guess_variations;

        }
    }


    //--------------------------------------------------------------------//
    //        Get all thdac quess variation on all vmms on all boards
    //--------------------------------------------------------------------//

    for ( uint ivar=0; ivar<variation.size(); ivar++ ) {

        //---------------------------------------------------//
        //          Configure all vmms on all febs
        //---------------------------------------------------//

        for (auto & feb : frontend_configs) {
            
            for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

                std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);
                
                if (thdac_central_variations_all_febs[feb_vmm].at(ivar)>1023){
                    if (debug) std::cout << "WARNING: Capping THDAC value out of range (>1023): " 
                                         << thdac_guess_variations_all_febs[feb_vmm].at(ivar) << std::endl;
                    thdac_guess_variations_all_febs[feb_vmm].at(ivar) = 1023;
                } else if (thdac_guess_variations_all_febs[feb_vmm].at(ivar)<0){
                    if (debug) std::cout << "WARNING: Capping THDAC value out of range (<0): " 
                                         << thdac_guess_variations_all_febs[feb_vmm].at(ivar) << std::endl;
                    thdac_guess_variations_all_febs[feb_vmm].at(ivar) = 0;
                }

                feb.getVmm(vmm_id).setMonitorOutput(nsw::vmm::ThresholdDAC, nsw::vmm::CommonMonitor);
                feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac_guess_variations_all_febs[feb_vmm].at(ivar)));

                cs.configVmmForPdoConsecutiveSamples(feb, vmm_id);

            }
        }

        usleep(10000);

        //---------------------------------------------------/
        //          query all vmms on all febs
        //---------------------------------------------------/

        for (auto & feb : frontend_configs) {

            for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

                std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);

                auto results = cs.queryVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);

                // use the median, and
                // avoid suspicious threshold measurements
                float median = take_median(results);
                bool measurement_ok = (median > 10);
                if (measurement_ok) {
                    thdac_guesses_vars_okay[feb_vmm].push_back((float)(thdac_guess_variations[feb_vmm].at(ivar)));
                    thdac_guesses_sample[feb_vmm].push_back(median);
                }
                if (debug)
                    std::cout << "INFO "
                              << feb.getAddress()
                              << " vmm" << vmm_id
                              << ", thdac " << thdac_guess_variations[feb_vmm].at(ivar)
                              << ", thdac (mV) " << sample_to_mV(median)
                              << (measurement_ok ? "" : "       <- Skipping suspicious measurement")
                              << std::endl;
            }
        }
    }

    //--------------------------------------------------------------------------//
    //                     Fit to a line of the results
    //--------------------------------------------------------------------------//

    for (auto & feb : frontend_configs) {

        for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

  // do fit to a line
  float thdac_guess_mean = std::accumulate(thdac_guesses_vars_okay.begin(), thdac_guesses_vars_okay.end(), 0.0)/thdac_guesses_vars_okay.size();
  float thdac_guess_sample_mean = std::accumulate(thdac_guesses_sample.begin(), thdac_guesses_sample.end(), 0.0)/thdac_guesses_sample.size();

  float num = 0.;
  float denom = 0.;
  for (unsigned int i = 0; i < thdac_guesses_vars_okay.size(); i++){
    num += (thdac_guesses_vars_okay[i]-thdac_guess_mean) * (thdac_guesses_sample[i]-thdac_guess_sample_mean);
    denom += pow((thdac_guesses_vars_okay[i]-thdac_guess_mean),2);
  }

  float thdac_slope = num/denom;
  float thdac_intercept = thdac_guess_sample_mean - thdac_slope * thdac_guess_mean;

  // (y-b) / m = x
  int thdac = (mV_to_sample(thdac_central_guess) - thdac_intercept)/thdac_slope;
    */
    int thdac=0;
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





int main(int ac, const char *av[]) {
  std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

  std::string description = "This program reads ADC values from a selected VMM in MMFE8/PFEB/SFEB";

  int vmm_id;
  int n_samples;
  int tpdac = -1;
  bool isSTGC = 0;
  std::string config_filename;
  std::string fe_name;
  std::string rms_str;
  po::options_description desc(description);
  desc.add_options()
      ("help,h", "produce help message")
      ("configfile,c", po::value<std::string>(&config_filename)->default_value(base_folder + "integration_config.json"),
        "Configuration file path")
      ("name,n", po::value<std::string>(&fe_name)->default_value(""),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB).\n"
        "If this option is left empty, all front end elements in the config file will be scanned.")
      ("vmm,V", po::value<int>(&vmm_id)->default_value(0), "VMM id (0-7)")
      ("samples,s", po::value<int>(&n_samples)->default_value(1000), "Number of samples to read")
      ("rms_factor", po::value<std::string>(&rms_str)->default_value("-1"), "RMS Factor")
      ("isSTGC", po::bool_switch()->default_value(false), "Use the sTGC configuration")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);
  isSTGC = vm["isSTGC"].as<bool>();
  isMM = !isSTGC;
  std::cout << std::endl;
  std::cout << "The detector under test is " << ((isMM) ? "MM" : "sTGC") << std::endl;
  std::cout << std::endl;

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

  std::vector<std::pair<nsw::FEBConfig, int>> frontend_configs;

  std::cout << "\nFollowing front ends will be configured:\n";
  std::cout <<   "========================================\n";
  for (auto & name : frontend_names) {
    try {

        int nvmm = 8;

        if (  name.find("PFEB") != std::string::npos ) {
            nvmm = 3;
        }
        else {
            nvmm = 8;
        }

        nsw::FEBConfig config(reader1.readConfig(name));

        std::pair<nsw::FEBConfig, int> FEB_config_nvmm = std::make_pair( config, nvmm );
        //frontend_configs.emplace_back(reader1.readConfig(name));
        
        frontend_configs.emplace_back( FEB_config_nvmm );

      std::cout << name << std::endl;
    } catch (std::exception & e) {
      std::cout << name << " - ERROR: Skipping this FE!"
                << " - Problem constructing configuration due to : " << e.what() << std::endl;
    }
    // frontend_configs.back().dump();
  }

  std::cout << "\n";

  nsw::ConfigSender cs;

  std::map< std::pair< std::string, std::pair<int,int> >, float> channel_baseline_med;
  std::map< std::pair< std::string, std::pair<int,int> >, float> channel_baseline_rms;

  std::map< std::string, float> vmm_baseline_med;
  std::map< std::string, float> vmm_baseline_rms;

  int offset_center = 14; // center the threshold in the middle of the window

  std::map< std::pair<std::string,int>, std::vector <float> > fe_samples_tmp;
  std::map< std::pair<std::string,int>, std::vector <float> > fe_samples_pruned;

  // first read baselines

  // calculate thdac
  std::map< std::pair< std::string,int>, int>   thdacs;
  std::map< std::pair< std::string,int>, float> thdacs_sample;

  std::map< std::pair< std::string,int>, int>   thdac_central_guesses;

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

  //--------------------------------------------------------------------//
  //                 Open All Output Files
  //--------------------------------------------------------------------//

  std::cout << "\nConfiguring All VMMs\n" << std::endl;

  std::map< std::pair< std::string, int>, std::ofstream > myfile;
  std::map< std::pair< std::string, int>, std::ofstream > myfile_summary;

  //--------------------------------------------------------------------//

  std::vector<float> blank;
  blank.clear();

  if ( debug ) std::cout << "running calibrate_trimmers_multirms_stgc.cpp" << std::endl;

  for (auto & feb_nvmms : frontend_configs) {
      
      uint n_vmms = feb_nvmms.second;
      nsw::FEBConfig feb = feb_nvmms.first;

      for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

          std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);

          //-------------------------------------------------------//
          //            Open File for one FEB
          //-------------------------------------------------------//
          
          // write baseline
          //std::ofstream myfile_i;
          //myfile_i.open("baselines_" + feb.getAddress() + "_VMM" + std::to_string(vmm_id) + ".txt");
          
          // write Summary (@patmasid - Prachi)
          //std::ofstream myfile_summary_i;
          //myfile_summary_i.open("summary_baselines_" + feb.getAddress() + "_VMM" + std::to_string(vmm_id) + ".txt");
          
          //-------------------------------------------------------//
          //           Append output to vector of output files
          //-------------------------------------------------------//

          myfile[feb_vmm]         = std::ofstream("baselines_" + feb.getAddress() + "_VMM" + std::to_string(vmm_id) + ".txt");
          myfile_summary[feb_vmm] = std::ofstream("summary_baselines_" + feb.getAddress() + "_VMM" + std::to_string(vmm_id) + ".txt");
          fe_samples_tmp[feb_vmm] = blank;
      }
      
  }

  //-----------------------------------------------------//
  //                Loop over all channels
  //-----------------------------------------------------//

  for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){

      std::cout << "\nTaking baselines channel " << channel_id << "\n" << std::endl;

      //------------------------------------------------//
      //             First Configure All VMMs
      //------------------------------------------------//

      for (auto & feb_nvmms : frontend_configs) {

          uint n_vmms = feb_nvmms.second;
          nsw::FEBConfig feb = feb_nvmms.first;

          for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

              // Read pdo of the certain channel n_samples times.
              feb.getVmm(vmm_id).setMonitorOutput(channel_id, nsw::vmm::ChannelMonitor);
              feb.getVmm(vmm_id).setChannelMOMode(channel_id, nsw::vmm::ChannelAnalogOutput);
              cs.configVmmForPdoConsecutiveSamples(feb, vmm_id);
          }

      }

      //   Wait until configuration finished to query adc
      //      usleep(1e7); // sleep 10 seconds (1e7 microseconds)

      //-------------------------------------------------------------------//
      //     Loop over FEBs and Query VMM's baseline for channel_id
      //-------------------------------------------------------------------//

      for (auto & feb_nvmms : frontend_configs) { // big feb loop

          uint n_vmms = feb_nvmms.second;
          nsw::FEBConfig feb = feb_nvmms.first;

          //-------------------------------------------------------//
          //                  Build RMS Factors 
          //-------------------------------------------------------//
          // announce
          std::vector<int> rms_factors = {};
          if (std::count(rms_str.begin(), rms_str.end(), ',')){
              std::istringstream ss(rms_str);
              while(!ss.eof()){
                  std::string buf;
                  std::getline(ss, buf, ',');
                  if (buf != "")
                      rms_factors.push_back( std::stoi(buf) );
              }
          }
          else
              rms_factors.push_back( std::stoi(rms_str) );
          for (auto rms: rms_factors)
              std::cout << "INFO - Found rms_factor = " << rms << std::endl;


          //---------------------------------------------------------------//
          //                  Loop over VMM ID
          //---------------------------------------------------------------//

          for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

              std::pair<std::string,int> feb_vmm = std::make_pair( feb.getAddress(), vmm_id);
              std::pair<std::string, std::pair<int,int>> feb_ch  = std::make_pair( feb.getAddress(), std::make_pair(vmm_id,channel_id) );

              //------------------------------------------------------//
              //      query baseline results for vmm_id, channel_id
              //------------------------------------------------------//

              std::vector<short unsigned int> results;

              for ( uint i=0; i<1; i++ ) {
                  auto results_temp = cs.queryVmmPdoConsecutiveSamples(feb, vmm_id, 100);//n_samples*10);
                  results.insert(results.end(), results_temp.begin(), results_temp.end());
              }

              std::vector<short unsigned int> results_late;
              results_late.resize(0);

              for (uint i=results.size()*9/10; i<results.size(); i++ ) {
                  results_late.push_back( results.at(i) );
              }

              std::cout << "late size " << results_late.size() << std::endl;


              //------------------------------------------------------//
              //             write baseline results to file
              //------------------------------------------------------//
              for (auto result: results) {
                  //std::cout << result << std::endl;
                  
                  myfile[feb_vmm] << "DATA"
                                  << " " << feb.getAddress()
                                  << " " << vmm_id
                                  << " " << channel_id
                                  << " " << tpdac
                                  << " " << -1
                                  << " " << -1
                                  << " " << result
                                  << std::endl;
              }

              //------------------------------------------------------//
              //      calculate channel level baseline median, rms
              //------------------------------------------------------//
              
              float sum    = std::accumulate(results.begin(), results.end(), 0.0);
              float mean   = sum / results.size();
              float stdev  = take_rms(results, mean);
              float median = take_median(results);
              
              float sum_late    = std::accumulate(results_late.begin(), results_late.end(), 0.0);
              float mean_late   = sum_late / results_late.size();
              float stdev_late  = take_rms(results_late, mean_late);
              float median_late = take_median(results_late);


              // add medians, baseline to (MMFE8, CH) map
              channel_baseline_med[feb_ch] = median;
              channel_baseline_rms[feb_ch] = stdev;
              if (debug)
                  std::cout << "INFO "      << feb.getAddress()
                            << " vmm"       << vmm_id
                            << ", channel " << channel_id
                            << " - mean: "  << sample_to_mV(mean)
                            << " , median: "<< sample_to_mV(median)
                            << " , stdev: " << sample_to_mV(stdev)
                            << std::endl;
              
              if (debug)
                  std::cout << "INFO late "      << feb.getAddress()
                            << " vmm"            << vmm_id
                            << ", channel "      << channel_id
                            << " - mean: "       << sample_to_mV(mean_late)
                            << " , median: "     << sample_to_mV(median_late)
                            << " , stdev: "      << sample_to_mV(stdev_late)
                            << std::endl;


              //------------------------------------------------------//
              //               Also write the summary
              //------------------------------------------------------//
              
              //@patmasid - Prachi
              myfile_summary[feb_vmm] << "SUMMARY"
                                      << " " << feb.getAddress()
                                      << " vmm " << vmm_id
                                      << " channel " << channel_id
                                      << " mean " << mean
                                      << " stdev " << stdev
                                      << " median " << median
                                      << std::endl;
              
              float mean_mV = mean*1000.0/4095.0;
              float stdev_mV = stdev*1000.0/4095.0;
              float median_mV = median*1000.0/4095.0;
              
              //@patmasid - Prachi
              /*if(mean_mV < 150 || mean_mV > 200){          
                myfile_baseline_outside_150_200mV << "BASELINE_OUTSIDE_150_200_MV"
                << " " << feb.getAddress()
                << " vmm " << vmm_id
                << " channel " << channel_id
                << " mean " << mean_mV
                << " stdev " << stdev_mV
                << " median " << median_mV
                << std::endl;
                }*/
              
              // if RMS of channel is too large, it's probably crap, ignore it
              // potentially output a list of channels to mask?
              if ( sample_to_mV(stdev) > RMS_CUTOFF )
                  continue;
              // add samples to the vector for a given fe which will be used
              // to calculate a vmm-level median, rms
              for (unsigned int i = 0; i < results.size(); i++) {
                  fe_samples_tmp[feb_vmm].push_back(results[i]);
              }
          } // end of loop over vmm_id
 
      } // end of loop over each FEB

      //usleep(1e7);
  } // end of loop over channel
  
  //---------------------------------------------------------//
  //             Close all Baseline output files
  //---------------------------------------------------------//

  for (auto & feb_nvmms : frontend_configs) {

      uint n_vmms = feb_nvmms.second;
      nsw::FEBConfig feb = feb_nvmms.first;

      for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

          std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);

          //-------------------------------------------------------//
          //           close each file in vector of output files
          //-------------------------------------------------------//
          
          myfile[feb_vmm].close();
          myfile_summary[feb_vmm].close();
      }

  }

  myfile.clear();
  myfile_summary.clear();

  /*

  //-----------------------------------------------------------//
  //                Process all baseline results
  //-----------------------------------------------------------//

  for (auto & feb : frontend_configs) {

      for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

          std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);

          //-------------------------------------------------------//
          //        Move onto cleaning data for trimming
          //-------------------------------------------------------//
          
          //myfile_baseline_outside_150_200mV.close();
          
          // remove samples which are highly suspicious
          // this can happen when a channel has a very high or low baseline, but with small RMS
          float tmp_median = take_median(fe_samples_tmp[feb_vmm]);
          
          fe_samples_pruned[feb_vmm].clear();
          for (auto sample: fe_samples_tmp[feb_vmm])
              if (fabs(sample_to_mV(sample - tmp_median)) < BASELINE_CUTOFF)
                  fe_samples_pruned[feb_vmm].push_back(sample);

          // calculate VMM median baseline and rms, 2019
          std::vector<float> channel_baseline_med_flattened = {};
          std::vector<float> channel_baseline_rms_flattened = {};
          for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
              std::pair<std::string,int> feb_ch(feb.getAddress(), std::make_pair(vmm_id,channel_id) )
              channel_baseline_med_flattened.push_back( channel_baseline_med[feb_ch] );
              channel_baseline_rms_flattened.push_back( channel_baseline_rms[feb_ch] );
          }
          float vmm_mean   = mV_to_sample(-1.0);
          float vmm_median = take_median(channel_baseline_med_flattened);
          float vmm_stdev  = take_median(channel_baseline_rms_flattened);

          // calculate VMM median baseline and rms, 2018
          // float vmm_sum    = std::accumulate(fe_samples_pruned.begin(), fe_samples_pruned.end(), 0.0);
          // float vmm_mean   = vmm_sum / fe_samples_pruned.size();
          // float vmm_stdev  = take_rms(fe_samples_pruned, vmm_mean);
          // float vmm_median = take_median(fe_samples_pruned);
      
          // add medians, baseline to MMFE8 --> med, stdev map

          vmm_baseline_med[feb_vmm] = vmm_median;
          vmm_baseline_rms[feb_vmm] = vmm_stdev;
      
          fe_samples_pruned[feb_vmm].clear();
          fe_samples_tmp[feb_vmm].clear();
      }// end of loop over ivmm

  }// end of loop over febs

  //----------------------------------------------------------//
  //     Baseline Scan Done, get global thresholds guesses
  //----------------------------------------------------------//

  for (auto & feb : frontend_configs) {

      for ( uint vmm_id=0; vmm_id < n_vmms; vmm_id++ ) {

          std::pair<std::string,int> feb_vmm(feb.getAddress(), vmm_id);

          //////////////////////////////////
          // Global Threshold Calculations
          
          bool first = true;
          bool flag_trim_in_range = false;
          int good_chs = 0;
          int tot_chs = NCH_PER_VMM;
          int nch_base_above_thresh = 0;
          std::map< std::pair< std::string,int>, int> best_channel_trim;
      
          //--------------------------------------------------------------//
          //           Will Not Loop over RMS factors for sTGC
          //--------------------------------------------------------------//

          //          for (auto rms_factor: rms_factors){          
          //std::cout << "INFO - rms_factor = " << rms_factor << std::endl;

          //--------------------------------------------------------------//
          //            Calculate the central guess value
          //--------------------------------------------------------------//

          //int thdac_central_guess = rms_factor * sample_to_mV(vmm_baseline_rms[feb.getAddress()]) + sample_to_mV(vmm_baseline_med[feb.getAddress()]) + offset_center;
          // int thdac_central_guess = rms_factor * sample_to_mV(vmm_baseline_rms[feb.getAddress()]) + sample_to_mV(vmm_baseline_med[feb.getAddress()]) + (int)(offset_center * 50.0/32.0);

          // @patmasid
          int thdac_central_guess = 30 + sample_to_mV(vmm_baseline_med[feb_vmm]) + offset_center;
          
          if (debug)
              std::cout << "INFO - baseline_mean, baseline_med, baseline_rms, rms_factor: "
                        << feb.getAddress() << " vmm"
                        << vmm_id << ", "
                        << sample_to_mV(vmm_mean) << ", "
                        << sample_to_mV(vmm_baseline_med[feb_vmm]) << ", "
                        << sample_to_mV(vmm_baseline_rms[feb_vmm]) << ", " //<< rms_factor
                        << std::endl;
          
          if (debug)
              std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << thdac_central_guess << " in mV, desired" << std::endl;

          thdac_central_guesses[feb_vmm] = thdac_central_guess;

      }
  }

  //-------------------------------------------------------------//
  //             Calculate thdac for each FEB
  //-------------------------------------------------------------//

  std::vector<int> thdac_guess_variations;
  thdac_guess_variations.push_back(thdac_central_guess - 20);
  thdac_guess_variations.push_back(thdac_central_guess - 10);
  thdac_guess_variations.push_back(thdac_central_guess + 20);
  thdac_guess_variations.push_back(thdac_central_guess + 30);
  thdac_guess_variations.push_back(thdac_central_guess + 40);
  thdac_guess_variations.push_back(thdac_central_guess + 50);
  thdac_guess_variations.push_back(thdac_central_guess + 60);
  thdacs = calculate_thdac_value(cs,frontend_configs,vmm_id,n_samples,thdac_central_guesses,thdac_guess_variations);
          
          thdacs[feb_vmm] = thdac;
          
          if (debug)
              std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << thdac << std::endl;
          
          feb.getVmm(vmm_id).setMonitorOutput  (nsw::vmm::ThresholdDAC, nsw::vmm::CommonMonitor);
          feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
          auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);
          float sum = std::accumulate(results.begin(), results.end(), 0.0);
          float mean = sum / results.size();
          thdacs_sample[feb_vmm] = mean;
          if (debug)
              std::cout << "INFO - Threshold for " << feb.getAddress() << " vmm" << vmm_id << " is " << sample_to_mV(mean) << " in mV" <<  std::endl;

          //
          //////////////////////////////////
      }
  }
          
          //////////////////////////////////
          // Get VMM-level averages.
          fe_samples_tmp.clear();
          for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
              
              thdac = thdacs[feb.getAddress()];
              feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
              feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
              feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(TRIM_MID));
              feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
              auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);
              
              // add samples to the vector for a given fe
              for (unsigned int i = 0; i < results.size(); i++) {
                  fe_samples_tmp.push_back((float)(results[i]));
              }
          }
          
          // find the median eff_thresh value for a given FE, vmm
          float vmm_median_trim_mid = take_median(fe_samples_tmp);
          float vmm_eff_thresh = vmm_median_trim_mid - vmm_baseline_med[feb.getAddress()];
          
          vmm_mid_eff_thresh[feb.getAddress()] = vmm_eff_thresh;
          
          if (debug)
              std::cout << "INFO - VMM, vmm_median_trim_mid, vmm_median, vmm_eff_thresh, rms_factor = "
                        << vmm_id << ", "
                        << sample_to_mV(vmm_median_trim_mid) << ", "
                        << sample_to_mV(vmm_median)          << ", "
                        << sample_to_mV(vmm_eff_thresh)      << ", "
                        << rms_factor                        << ", "
                        << std::endl;
          
          //
          //////////////////////////////////
          
          
          
          //////////////////////////////////
          // Scanning trimmers
          
          // count how many channels have baselines above the threshold
          // only do this for the lowest rms_factor considered
          
          std::cout << "\nTaking trimmers\n" << std::endl;
          
          thdac = thdacs[feb.getAddress()];
          
          if (first) {
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
                  if (debug)
                      std::cout << "INFO Channel " << channel_id
                                << " channel_mid_eff_thresh " << channel_mid_eff_thresh[feb_ch]
                                << " rms_factor " << rms_factor
                                << std::endl;
                  
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
              
          }
          else {
              
              // need to find a new effective threshold at TRIM_MID for different rms
              // slope and trimmer_max are taken from the first rms_factor
              for (int channel_id = 0; channel_id < NCH_PER_VMM; channel_id++){
                  feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
                  feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
                  feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(TRIM_MID));
                  feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
                  auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);
                  
                  std::pair<std::string,int> feb_ch(feb.getAddress(),channel_id);
                  float median = take_median(results);
                  float ch_baseline_med = channel_baseline_med[feb_ch];
                  channel_mid_eff_thresh[feb_ch] = median - ch_baseline_med;
                  if (debug)
                      std::cout << "INFO Channel " << channel_id
                                << " channel_mid_eff_thresh " << channel_mid_eff_thresh[feb_ch]
                                << " rms_factor " << rms_factor
                                << std::endl;
              }
              
          }

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
          
          if (debug)
              std::cout << "INFO " << good_chs << " out of " << tot_chs << " are okay!" << std::endl;
          
          if ((good_chs-tot_chs) < 1){
              if (debug) 
                  std::cout << "INFO set trim flag true!" << std::endl;
              flag_trim_in_range = true;
          }
          
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
                  
                  if (first) {
                      // guess at a trim value
                      float delta = channel_mid_eff_thresh[feb_ch] - vmm_mid_eff_thresh[feb.getAddress()];
                      int trim_guess = TRIM_MID + std::round(delta / channel_eff_thresh_slope[feb_ch]);
                      
                      // cap off the guess for trimmer value to avoid non-linear region.
                      best_channel_trim[feb_ch] = trim_guess > channel_trimmer_max[feb_ch] ? channel_trimmer_max[feb_ch] : trim_guess;
                      best_channel_trim[feb_ch] = best_channel_trim[feb_ch] < 0            ? 0 : best_channel_trim[feb_ch];
                  }
                  
                  int thdac = thdacs[feb.getAddress()];
                  feb.getVmm(vmm_id).setMonitorOutput  (channel_id, nsw::vmm::ChannelMonitor);
                  feb.getVmm(vmm_id).setChannelMOMode  (channel_id, nsw::vmm::ChannelTrimmedThreshold);
                  feb.getVmm(vmm_id).setChannelTrimmer (channel_id, (size_t)(best_channel_trim[feb_ch]));
                  feb.getVmm(vmm_id).setGlobalThreshold((size_t)(thdac));
                  auto results = cs.readVmmPdoConsecutiveSamples(feb, vmm_id, n_samples);
                  
                  // unused for now
                  // float sum = std::accumulate(results.begin(), results.end(), 0.0);
                  // float mean = sum / results.size();
                  // float stdev = take_rms(results,mean);
                  
                  float median = take_median(results);
                  float eff_thresh = median - channel_baseline_med[feb_ch];
                  std::cout << "DATA_x" << rms_factor
                            << " " << feb.getAddress()
                            << " " << vmm_id
                            << " " << channel_id
                            << " " << tpdac
                            << " " << thdac
                            << " " << TRIM_MID
                            << " " << channel_mid_eff_thresh[feb_ch]
                            << " " << best_channel_trim[feb_ch]
                            << " " << eff_thresh << std::endl;
              }
          }

          else{
              std::cout << "Must change the Global VMM THDAC values" << std::endl;
          }
          
          first = false;
      
      }// end of RMS factors

  } // end of front-end configs

  */
  
  return 0;
}






