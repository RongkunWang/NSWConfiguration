//
// Program to test pulse triggerable patterns, MM edition
//

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/Utility.h"
#include "boost/program_options.hpp"

namespace po = boost::program_options;

int active_threads(std::vector< std::future<int> >* threads);
int wait_until_done(std::vector< std::future<int> >* threads);
int wait_until_fewer(std::vector< std::future<int> >* threads, int max_threads);
int configure_frontend(nsw::ConfigSender* cs, nsw::FEBConfig feb, bool do_roc, bool do_reset, bool do_vmm);
int configure_vmm(nsw::ConfigSender* cs, nsw::FEBConfig feb, int vmmid, bool reset);
int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, std::vector<int> vmmids, bool reset);
int configure_art_input_phase(nsw::ConfigSender* cs, nsw::ADDCConfig addc, uint phase);
std::vector<nsw::FEBConfig> parse_feb_name(std::string name, std::string cfg);
std::vector<nsw::ADDCConfig> parse_addc_name(std::string name, std::string cfg);
std::vector< std::vector< std::tuple<std::string, int, int> > > patterns();
std::string strf_time();
int minutes_remaining(double time_diff, int nprocessed, int ntotal);

int main(int argc, const char *argv[]) {
    std::string config_filename;
    std::string addc_filename;
    std::string board_name_mmfe8;
    std::string board_name_addc;
    std::string system_cmd0;
    std::string system_cmd1;
    std::string system_cmd2;
    std::string system_cmd3;
    int max_threads;
    bool dry_run;
    bool mask_all;
    bool reset_vmm;

    // CL options
    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/full_small_sector_12.json"),
         "Configuration file path")
        ("addc_file,a", po::value<std::string>(&addc_filename)->
         default_value(""), "ADDC config file path")
        ("dry_run", po::bool_switch()->
         default_value(false), "Option to NOT send configurations")
        ("mask_all", po::bool_switch()->
         default_value(false), "Option to mask everything first")
        ("reset_vmm", po::bool_switch()->
         default_value(false), "Option to reset VMM at every configuration")
        ("max_threads,m", po::value<int>(&max_threads)->
         default_value(-1), "Maximum number of threads to run")
        ("mmfe8", po::value<std::string>(&board_name_mmfe8)->
         default_value(""), "The name of frontend to configure (should start with MMFE8_).")
        ("addc", po::value<std::string>(&board_name_addc)->
         default_value(""), "The name of ADDC to configure (should start with ADDC_).")
        ("ttc0", po::value<std::string>(&system_cmd0)->
         default_value("echo 'I could be ECR.' >> /dev/null"), "Path to TTC command 0 (e.g. ecr)")
        ("ttc1", po::value<std::string>(&system_cmd1)->
         default_value("echo 'I could be test pulse.' >> /dev/null"), "Path to TTC command 1 (e.g. pulse1000)")
        ("ttc2", po::value<std::string>(&system_cmd2)->
         default_value("echo 'I could be reset/stop.' >> /dev/null"), "Path to TTC command 2 (e.g. reset)")
        ("startup", po::value<std::string>(&system_cmd3)->
         default_value("echo 'I could be ECR.' >> /dev/null"), "Path to startup command (e.g. sr 11 && ecr 11 && ./mmtp_loopback_test -n 0 --windowLeft 08 --windowRight 08  --windowCenter 22 -s NSW_TrigProc_MM.I2C_0.bus0 -o pcatlnswfelix03.cern.ch:48020)")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dry_run   = vm["dry_run"  ].as<bool>();
    mask_all  = vm["mask_all" ].as<bool>();
    reset_vmm = vm["reset_vmm"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // announce
    std::cout << "Dry run:          " << dry_run     << std::endl;
    std::cout << "VMM hard resets:  " << reset_vmm   << std::endl;
    std::cout << "System command 0: " << system_cmd0 << std::endl;
    std::cout << "System command 1: " << system_cmd1 << std::endl;
    std::cout << "System command 2: " << system_cmd2 << std::endl;
    std::cout << "System command 3: " << system_cmd3 << std::endl;

    // the febs
    auto febs = parse_feb_name(board_name_mmfe8, config_filename);
    std::cout << "Number of FEBs found: " << febs.size() << std::endl;

    // the addcs
    std::vector<nsw::ADDCConfig> addcs = {};
    if (addc_filename.length() > 0)
        addcs = parse_addc_name(board_name_addc, addc_filename);
    std::cout << "Number of ADDCs found: " << addcs.size() << std::endl;

    // the configuration threads
    std::cout << "Creating vector of threads..." << std::endl;
    auto threads = new std::vector< std::future<int> >();

    // limit 1 sender per board
    std::cout << "Creating map of ConfigSender*..." << std::endl;
    auto senders = new std::map< std::string, nsw::ConfigSender* >();
    for (auto & feb : febs)
        senders->insert( {feb.getAddress(), new nsw::ConfigSender()} );
    for (auto & addc : addcs)
        senders->insert( {addc.getAddress(), new nsw::ConfigSender()} );

    // mask everything
    if (mask_all) {
        for (auto & feb : febs) {
            for (int vmm_id = 0; vmm_id < (int)(feb.getVmms().size()); vmm_id++) {
                feb.getVmm(vmm_id).setTestPulseDAC(400);
                feb.getVmm(vmm_id).setChannelRegisterAllChannels("channel_st", 0);
                feb.getVmm(vmm_id).setChannelRegisterAllChannels("channel_sm", 1);
            }
            wait_until_fewer(threads, max_threads);
            if (!dry_run)
                threads->push_back(std::async(std::launch::async,
                                              configure_frontend,
                                              senders->at(feb.getAddress()), feb, 1, 1, 1) );
        }
        wait_until_done(threads);
    }

    // run startup command
    if (!dry_run) {
        std::cout << "Startup command running...";
        system(system_cmd3.c_str()); usleep(100000);
        std::cout << " finished. ";
        std::cout << std::endl;
    }

    // nuance: must configure 1 MMFE8 at a time,
    // so config-sending must be steered by MMFE8s, not patterns
    std::map< std::string, std::vector<int> > vmms4feb = {};
    for (auto & feb : febs)
        vmms4feb[feb.getAddress()] = {};

    // keep time
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    // loop over patterns
    std::string fename;
    int vmmid, chan;
    int ipatt = 0;
    auto patts = patterns();
    for (auto pattern : patts) {

        ipatt++;
        std::cout << "> " << ipatt << " / " << patts.size() << " :: ";

        // clear memory of which VMMs on FEB to configure
        for (auto & feb : febs)
            vmms4feb[feb.getAddress()].clear();

        // enable test pulse
        // dont limit threads - only 8 layers
        bool anything = 0;
        std::cout << "Enable";
        for (auto feb_vmm_chan : pattern) {
            std::tie(fename, vmmid, chan) = feb_vmm_chan;
            for (auto & feb : febs) {
                if (fename != feb.getAddress())
                    continue;
                anything = 1;
                std::cout << " " << feb.getAddress() << ", VMM" << vmmid << ", CH" << chan;
                feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_st", 1, chan);
                feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_sm", 0, chan);
                vmms4feb[feb.getAddress()].push_back(vmmid);
            }
        }
        for (auto & feb : febs)
            if (vmms4feb[feb.getAddress()].size() > 0 && !dry_run)
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              vmms4feb[feb.getAddress()], reset_vmm));
        std::cout << ". ";
        wait_until_done(threads);
        if (!anything) {
            std::cout << " Skipping.";
            std::cout << std::flush << std::endl;
            continue;
        }

        // run the TTC system commands
        if (addcs.size() == 0) {
            if (!dry_run) {
                std::cout << "System commands running...";
                system(system_cmd0.c_str()); usleep(100000);
                system(system_cmd1.c_str()); usleep(100000);
                system(system_cmd2.c_str()); usleep(100000);
                std::cout << " finished. ";
            }
        } else {
            // or, loop through ADDC input phases and run TTC
            uint nphase = 16;
            for (uint phase = 0; phase < nphase; phase++) {
                for (auto & addc : addcs)
                    if (!dry_run)
                        threads->push_back(std::async(std::launch::async,
                                                      configure_art_input_phase,
                                                      senders->at(addc.getAddress()), addc, phase));
                wait_until_done(threads);
                if (!dry_run) {
                    std::cout << "System commands running...";
                    system(system_cmd0.c_str()); usleep(100000);
                    system(system_cmd1.c_str()); usleep(100000);
                    system(system_cmd2.c_str()); usleep(100000);
                    std::cout << " finished. ";
                }
            }
        }

        // disable test pulse
        std::cout << "Disabled";
        for (auto feb_vmm_chan : pattern) {
            std::tie(fename, vmmid, chan) = feb_vmm_chan;
            for (auto & feb : febs) {
                if (fename != feb.getAddress())
                    continue;
                std::cout << " " << feb.getAddress() << ", VMM" << vmmid << ", CH" << chan;
                feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_st", 0, chan);
                feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_sm", 1, chan);
            }
        }
        for (auto & feb : febs)
            if (vmms4feb[feb.getAddress()].size() > 0 && !dry_run)
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              vmms4feb[feb.getAddress()], reset_vmm));
        std::cout << ". ";

        // wrap up
        elapsed_seconds = (std::chrono::system_clock::now() - time_start);
        std::cout << "~" << minutes_remaining(elapsed_seconds.count(), ipatt, patts.size()) << " min left.";
        std::cout << std::flush;
        std::cout << std::endl;
        wait_until_done(threads);
    }

    // wrap up
    std::cout << std::endl;
    elapsed_seconds = (std::chrono::system_clock::now() - time_start);
    std::cout << "Elapsed time: ~" << std::lround(elapsed_seconds.count()/60.0) << " minutes" << std::endl;
    return 0;
}

int active_threads(std::vector< std::future<int> >* threads) {
    int nfinished = 0;
    for (auto& thread : *threads)
        if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            nfinished++;
    return (int)(threads->size()) - nfinished;
}

int wait_until_done(std::vector< std::future<int> >* threads) {
    for (auto& thread : *threads)
        thread.get();
    threads->clear();
    return 0;
}

int wait_until_fewer(std::vector< std::future<int> >* threads, int max_threads) {
    if (max_threads > 0) {
        int n_active = active_threads(threads);
        while (n_active >= max_threads) {
            std::cout << "Too many active threads (" << n_active <<
                "), waiting for fewer than " << max_threads << std::endl;
            sleep(2);
            n_active = active_threads(threads);
        }
    }
    return 0;
}

std::string strf_time() {
    std::stringstream ss;
    std::string out;
    std::time_t result = std::time(nullptr);
    std::tm tm = *std::localtime(&result);
    ss << std::put_time(&tm, "%Y_%m_%d_%Hh%Mm%Ss");
    ss >> out;
    return out;
}

int minutes_remaining(double time_diff, int nprocessed, int ntotal) {
    double rate = (double)(nprocessed+1)/time_diff;
    return std::lround((double)(ntotal-nprocessed)/(rate*60));
}

std::vector< std::vector< std::tuple<std::string, int, int> > > patterns() {
    //
    // patterns = [ [ (MMFE8_L1P1_HOR, 0, 0), (MMFE8_L2P1_HOL, 0, 0), (MMFE8_L3P1_HOR, 0, 0), ... ], # pattern 0
    //              [ (MMFE8_L1P1_HOR, 0, 1), (MMFE8_L2P1_HOL, 0, 1), (MMFE8_L3P1_HOR, 0, 1), ... ], # pattern 1
    //              [ (MMFE8_L1P1_HOR, 0, 2), (MMFE8_L2P1_HOL, 0, 2), (MMFE8_L3P1_HOR, 0, 2), ... ], # pattern 2
    //              ...
    //            ]
    //
    std::vector< std::vector< std::tuple<std::string, int, int> > > patts = {};

    // connectivity example: channel 7
    // only 1 feb per pattern
    for (auto fename : {

    "MMFE8_L1P1_HOL",
    "MMFE8_L1P2_HOL",
    "MMFE8_L1P3_HOL",
    "MMFE8_L1P4_HOL",
    "MMFE8_L1P5_HOL",
    "MMFE8_L1P6_HOL",
    "MMFE8_L1P7_HOL",
    "MMFE8_L1P8_HOL",
    "MMFE8_L1P1_HOR",
    "MMFE8_L1P2_HOR",
    "MMFE8_L1P3_HOR",
    "MMFE8_L1P4_HOR",
    "MMFE8_L1P5_HOR",
    "MMFE8_L1P6_HOR",
    "MMFE8_L1P7_HOR",
    "MMFE8_L1P8_HOR",
    "MMFE8_L2P1_HOL",
    "MMFE8_L2P2_HOL",
    "MMFE8_L2P3_HOL",
    "MMFE8_L2P4_HOL",
    "MMFE8_L2P5_HOL",
    "MMFE8_L2P6_HOL",
    "MMFE8_L2P7_HOL",
    "MMFE8_L2P8_HOL",
    "MMFE8_L2P1_HOR",
    "MMFE8_L2P2_HOR",
    "MMFE8_L2P3_HOR",
    "MMFE8_L2P4_HOR",
    "MMFE8_L2P5_HOR",
    "MMFE8_L2P6_HOR",
    "MMFE8_L2P7_HOR",
    "MMFE8_L2P8_HOR",
    "MMFE8_L3P1_HOL",
    "MMFE8_L3P2_HOL",
    "MMFE8_L3P3_HOL",
    "MMFE8_L3P4_HOL",
    "MMFE8_L3P5_HOL",
    "MMFE8_L3P6_HOL",
    "MMFE8_L3P7_HOL",
    "MMFE8_L3P8_HOL",
    "MMFE8_L3P1_HOR",
    "MMFE8_L3P2_HOR",
    "MMFE8_L3P3_HOR",
    "MMFE8_L3P4_HOR",
    "MMFE8_L3P5_HOR",
    "MMFE8_L3P6_HOR",
    "MMFE8_L3P7_HOR",
    "MMFE8_L3P8_HOR",
    "MMFE8_L4P1_HOL",
    "MMFE8_L4P2_HOL",
    "MMFE8_L4P3_HOL",
    "MMFE8_L4P4_HOL",
    "MMFE8_L4P5_HOL",
    "MMFE8_L4P6_HOL",
    "MMFE8_L4P7_HOL",
    "MMFE8_L4P8_HOL",
    "MMFE8_L4P1_HOR",
    "MMFE8_L4P2_HOR",
    "MMFE8_L4P3_HOR",
    "MMFE8_L4P4_HOR",
    "MMFE8_L4P5_HOR",
    "MMFE8_L4P6_HOR",
    "MMFE8_L4P7_HOR",
    "MMFE8_L4P8_HOR",
    "MMFE8_L1P1_IPL",
    "MMFE8_L1P2_IPL",
    "MMFE8_L1P3_IPL",
    "MMFE8_L1P4_IPL",
    "MMFE8_L1P5_IPL",
    "MMFE8_L1P6_IPL",
    "MMFE8_L1P7_IPL",
    "MMFE8_L1P8_IPL",
    "MMFE8_L1P1_IPR",
    "MMFE8_L1P2_IPR",
    "MMFE8_L1P3_IPR",
    "MMFE8_L1P4_IPR",
    "MMFE8_L1P5_IPR",
    "MMFE8_L1P6_IPR",
    "MMFE8_L1P7_IPR",
    "MMFE8_L1P8_IPR",
    "MMFE8_L2P1_IPL",
    "MMFE8_L2P2_IPL",
    "MMFE8_L2P3_IPL",
    "MMFE8_L2P4_IPL",
    "MMFE8_L2P5_IPL",
    "MMFE8_L2P6_IPL",
    "MMFE8_L2P7_IPL",
    "MMFE8_L2P8_IPL",
    "MMFE8_L2P1_IPR",
    "MMFE8_L2P2_IPR",
    "MMFE8_L2P3_IPR",
    "MMFE8_L2P4_IPR",
    "MMFE8_L2P5_IPR",
    "MMFE8_L2P6_IPR",
    "MMFE8_L2P7_IPR",
    "MMFE8_L2P8_IPR",
    "MMFE8_L3P1_IPL",
    "MMFE8_L3P2_IPL",
    "MMFE8_L3P3_IPL",
    "MMFE8_L3P4_IPL",
    "MMFE8_L3P5_IPL",
    "MMFE8_L3P6_IPL",
    "MMFE8_L3P7_IPL",
    "MMFE8_L3P8_IPL",
    "MMFE8_L3P1_IPR",
    "MMFE8_L3P2_IPR",
    "MMFE8_L3P3_IPR",
    "MMFE8_L3P4_IPR",
    "MMFE8_L3P5_IPR",
    "MMFE8_L3P6_IPR",
    "MMFE8_L3P7_IPR",
    "MMFE8_L3P8_IPR",
    "MMFE8_L4P1_IPL",
    "MMFE8_L4P2_IPL",
    "MMFE8_L4P3_IPL",
    "MMFE8_L4P4_IPL",
    "MMFE8_L4P5_IPL",
    "MMFE8_L4P6_IPL",
    "MMFE8_L4P7_IPL",
    "MMFE8_L4P8_IPL",
    "MMFE8_L4P1_IPR",
    "MMFE8_L4P2_IPR",
    "MMFE8_L4P3_IPR",
    "MMFE8_L4P4_IPR",
    "MMFE8_L4P5_IPR",
    "MMFE8_L4P6_IPR",
    "MMFE8_L4P7_IPR",
    "MMFE8_L4P8_IPR"
    }) {
        continue;

        // every VMM
        for (auto ch : {10, 30, 50}) {
            std::vector< std::tuple<std::string, int, int> > patt = {};
            for (int vmmid = 0; vmmid < 8; vmmid++)
                patt.push_back(std::make_tuple(fename, vmmid, ch));
            patts.push_back(patt);
        }

        // one VMM per board
        // for (int vmmid = 0; vmmid < 8; vmmid++) {
        //     std::vector< std::tuple<std::string, int, int> > patt = {}; 
        //     patt.push_back(std::make_tuple(fename, vmmid, 7));
        //     patts.push_back(patt);
        // }

        // for (int vmmid = 0; vmmid < 8; vmmid++) {
        //     // if (vmmid != 3 && vmmid != 6)
        //     //   continue;
        //     for (int chch = 0; chch < 64; chch++) {
        //         std::vector< std::tuple<std::string, int, int> > patt = {};
        //         patt.push_back(std::make_tuple(fename, vmmid, chch));
        //         patts.push_back(patt);
        //     }
        // }
    }

    // // track-like example
    // for (int vmmid = 6; vmmid >= 0; vmmid--) {
    //     for (int chan = 63; chan >= 0; chan--) {
    //         std::vector< std::tuple<std::string, int, int> > patt = {};
    //         patt.push_back(std::make_tuple("MMFE8_L1P1_HOR", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L2P1_HOL", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L3P1_HOR", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L4P1_HOL", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L4P1_IPR", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L3P1_IPL", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L2P1_IPR", vmmid, chan));
    //         patt.push_back(std::make_tuple("MMFE8_L1P1_IPL", vmmid, chan));
    //         patts.push_back(patt);
    //         break; // only one channel for now!
    //     }
    //     break; // only one VMM for now!
    // }

    // connectivity max-parallel loop
    // 64 channels per test pulse command (8 layers, 8 VMM per MMFE8)
    // int nvmm = 8;
    // int nchan = 64;
    // for (int pcb = 1; pcb < 9; pcb++) {
    //     for (auto side : {"L", "R"}) {
    //         for (int chan = 0; chan < nchan; chan++) {
    //             if (chan != 21)
    //                 continue;
    //             std::vector< std::tuple<std::string, int, int> > patt = {};
    //             for (int vmmid = 0; vmmid < nvmm; vmmid++) {
    //                 auto pcbstr = std::to_string(pcb);
    //                 patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_HO" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_HO" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_HO" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_HO" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_IP" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_IP" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_IP" + side, vmmid, chan));
    //                 patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_IP" + side, vmmid, chan));
    //             }
    //             patts.push_back(patt);
    //         }
    //     }
    // }

    //
    // connectivity max-parallel loop
    // for each MMFE8 position on the chamber (0-15)
    //   configure the MMFE8s of all 8 layers in this position
    //   to test pulse one channel on all 8 VMMs => 64 channels total
    //
    bool even;
    int nvmm = 8;
    int nchan = 64;
    int pcb = 0;
    for (int pos = 0; pos < 16; pos++) {
        for (int chan = 0; chan < nchan; chan++) {
            if (chan % 5 != 0)
                continue;
            std::vector< std::tuple<std::string, int, int> > patt = {};
            for (int vmmid = 0; vmmid < nvmm; vmmid++) {
                even = pos % 2 == 0;
                pcb  = pos / 2 + 1;
                auto pcbstr = std::to_string(pcb);
                patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_HO" + (even ? "R" : "L"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_HO" + (even ? "L" : "R"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_HO" + (even ? "R" : "L"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_HO" + (even ? "L" : "R"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_IP" + (even ? "R" : "L"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_IP" + (even ? "L" : "R"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_IP" + (even ? "R" : "L"), vmmid, chan));
                patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_IP" + (even ? "L" : "R"), vmmid, chan));
            }
            patts.push_back(patt);
        }
    }

    // track-like loop
    // bool even;
    // int nvmm = 8;
    // int nchan = 64;
    // int pcb, this_vmm, this_chan;
    // for (int pos = 0; pos < 16; pos++) {
    //     for (int vmmid = 0; vmmid < nvmm; vmmid++) {
    //         for (int chan = 0; chan < nchan; chan++) {
    //             even = pos % 2 == 0;
    //             pcb  = pos / 2 + 1;
    //             auto pcbstr = std::to_string(pcb);
    //             this_vmm  = even ? nvmm-1-vmmid : vmmid;
    //             this_chan = 7; // even ? nchan-1-chan : chan;
    //             std::vector< std::tuple<std::string, int, int> > patt = {};
    //             patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_HO" + (even ? "R" : "L"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_HO" + (even ? "L" : "R"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_HO" + (even ? "R" : "L"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_HO" + (even ? "L" : "R"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L4P" + pcbstr + "_IP" + (even ? "R" : "L"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L3P" + pcbstr + "_IP" + (even ? "L" : "R"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L2P" + pcbstr + "_IP" + (even ? "R" : "L"), this_vmm, this_chan));
    //             patt.push_back(std::make_tuple("MMFE8_L1P" + pcbstr + "_IP" + (even ? "L" : "R"), this_vmm, this_chan));
    //             patts.push_back(patt);
    //             break; // only one channel for now!
    //         }
    //         // break; // only one VMM for now!
    //     }
    // }

    // extremely simple example:
    // std::vector< std::tuple<std::string, int, int> > patt0 = {};
    // patt0.push_back(std::make_tuple(std::string("MMFE8_L1P1_HOR"), 2, 10));
    // patt0.push_back(std::make_tuple(std::string("MMFE8_L2P1_HOL"), 2, 10));
    // patts.push_back(patt0);
    return patts;
}

std::vector<nsw::FEBConfig> parse_feb_name(std::string name, std::string cfg) {
    // create a json reader
    nsw::ConfigReader reader1("json://" + cfg);
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
    std::set<std::string> names;
    if (name != "") {
        if (std::count(name.begin(), name.end(), ',')) {
            std::istringstream ss(name);
            while (!ss.eof()) {
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    names.emplace(buf);
            }
        } else {
            names.emplace(name);
        }
    } else {
        names = reader1.getAllElementNames();
    }

    // make FEB objects
    std::vector<nsw::FEBConfig> feb_configs;
    for (auto & nm : names) {
        try {
            if (nsw::getElementType(nm) == "MMFE8") {
                feb_configs.emplace_back(reader1.readConfig(nm));
                std::cout << "Adding: " << nm << std::endl;
            } else {
                std::cout << std::endl << "Skipping: " << nm
                          << " because its a " << nsw::getElementType(nm)
                          << std::endl;
            }
        }
        catch (std::exception & e) {
            std::cout << nm << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }
    return feb_configs;
}

std::vector<nsw::ADDCConfig> parse_addc_name(std::string name, std::string cfg) {
    // create a json reader
    nsw::ConfigReader reader1("json://" + cfg);
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
    std::set<std::string> names;
    if (name != "") {
        if (std::count(name.begin(), name.end(), ',')) {
            std::istringstream ss(name);
            while (!ss.eof()) {
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    names.emplace(buf);
            }
        } else {
            names.emplace(name);
        }
    } else {
        names = reader1.getAllElementNames();
    }

    // make ADDC objects
    std::vector<nsw::ADDCConfig> addc_configs;
    for (auto & nm : names) {
        try {
            if (nsw::getElementType(nm) == "ADDC") {
                addc_configs.emplace_back(reader1.readConfig(nm));
                std::cout << "Adding: " << nm << std::endl;
            } else {
                std::cout << std::endl << "Skipping: " << nm
                          << " because its a " << nsw::getElementType(nm)
                          << std::endl;
            }
        }
        catch (std::exception & e) {
            std::cout << nm << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }
    return addc_configs;
}

int configure_frontend(nsw::ConfigSender* cs, nsw::FEBConfig feb, bool do_roc, bool do_reset, bool do_vmm) {
    std::cout << "New thread in configure_frontend for " << feb.getAddress() << std::endl;
    if (do_roc)
        cs->sendRocConfig(feb);
    // reset_vmm
    if (do_reset) {
        auto & vmms = feb.getVmms();
        std::vector <unsigned> reset_ori;
        // Set reset bits to 1
        for (auto & vmm : vmms) {
            reset_ori.push_back(vmm.getGlobalRegister("reset"));
            vmm.setGlobalRegister("reset", 3);
        }
        cs->sendVmmConfig(feb);
        // Set reset bits to original
        size_t i = 0;
        for (auto & vmm : vmms)
            vmm.setGlobalRegister("reset", reset_ori[i++]);
    }
    if (do_vmm)
        cs->sendVmmConfig(feb);
    return 0;
}

int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, std::vector<int> vmmids, bool reset) {
    if (reset) {
        for (auto vmmid : vmmids) {
            auto & vmm = feb.getVmm(vmmid);
            auto orig = vmm.getGlobalRegister("reset");
            vmm.setGlobalRegister("reset", 3);
            cs->sendVmmConfigSingle(feb, vmmid);
            vmm.setGlobalRegister("reset", orig);
        }
    }
    for (auto vmmid : vmmids)
        cs->sendVmmConfigSingle(feb, vmmid);
    return 0;
}

int configure_vmm(nsw::ConfigSender* cs, nsw::FEBConfig feb, int vmmid, bool reset) {
    if (reset) {
        auto & vmm = feb.getVmm(vmmid);
        auto orig = vmm.getGlobalRegister("reset");
        vmm.setGlobalRegister("reset", 3);
        cs->sendVmmConfigSingle(feb, vmmid);
        vmm.setGlobalRegister("reset", orig);
    }
    cs->sendVmmConfigSingle(feb, vmmid);
    return 0;
}

int configure_art_input_phase(nsw::ConfigSender* cs, nsw::ADDCConfig addc, uint phase) {
    if (phase > std::pow(2, 4))
        throw std::runtime_error("Gave bad phase to configure_art_input_phase: " + std::to_string(phase));
    size_t art_size = 2;
    uint8_t art_data[] = {0x0, 0x0};
    auto opc_ip   = addc.getOpcServerIp();
    auto sca_addr = addc.getAddress();
    uint8_t this_phase = phase + (phase << 4);
    std::cout << "Setting input phase of " << sca_addr << " to be 0x"
              << std::hex << (uint)(this_phase) << std::dec << std::endl;
    for (auto art : addc.getARTs()) {
        auto name = sca_addr + "." + art.getName() + "Ps" + "." + art.getName() + "Ps";
        for (auto reg : { 6,  7,  8,  9,
                         21, 22, 23, 24,
                         36, 37, 38, 39,
                         51, 52, 53, 54,
                    }) {
            art_data[0] = (uint8_t)(reg);
            art_data[1] = this_phase;
            cs->sendI2cRaw(opc_ip, name, art_data, art_size);
        }
    }
    return 0;
}
