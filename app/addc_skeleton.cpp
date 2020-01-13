// Program to set ADDC configuration?

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/Utility.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int active_threads(std::vector< std::future<int> >* threads);
int configure_addc(nsw::ADDCConfig feb);
int change_phase(nsw::ADDCConfig addc, uint phase, uint fine, std::vector<bool> aligned);
std::string strf_time();

int main(int argc, const char *argv[])
{
    std::string config_filename;
    std::string board_name;
    bool dont_config;
    bool dont_align;
    bool dont_watch;
    bool map_phase;
    int manual_phase;
    int bcr_phase;

    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,C", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/addc_test_art_common_config.json"),
         "Configuration file path")
        ("dont_config", po::bool_switch()->
         default_value(false), "Option to NOT configure the ADDCs")
        ("dont_align", po::bool_switch()->
         default_value(false), "Option to NOT align the ADDCs to the TPs")
        ("dont_watch", po::bool_switch()->
         default_value(false), "Option to NOT monitor the ADDC-TP alignment vs time")
        ("map_phase", po::bool_switch()->
         default_value(false), "Option to turn on phase mapping mode")
        ("manual_phase", po::value<int>(&manual_phase)->
         default_value(-1), "Manual phase of ART alignment")
        ("bcr_phase", po::value<int>(&bcr_phase)->
         default_value(0), "Manual phase of ART BCRCLK")
        ("name,n", po::value<std::string>(&board_name)->
         default_value(""), "The name of frontend to configure (should start with ADDC_).");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dont_config = vm["dont_config"].as<bool>();
    dont_align  = vm["dont_align" ].as<bool>();
    dont_watch  = vm["dont_watch"].as<bool>();
    map_phase   = vm["map_phase"].as<bool>();

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // create a json reader
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
    std::set<std::string> board_names;
    if (board_name != ""){
        if (std::count(board_name.begin(), board_name.end(), ',')){
            std::istringstream ss(board_name);
            while(!ss.eof()){
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    board_names.emplace(buf);
            }
        }
        else
            board_names.emplace(board_name);
    }
    else
        board_names = reader1.getAllElementNames();

    // make ADDC objects
    std::vector<nsw::ADDCConfig> addc_configs;
    for (auto & name : board_names) {
        try {
            if (nsw::getElementType(name) == "ADDC") {
                addc_configs.emplace_back(reader1.readConfig(name));
                std::cout << "Adding: " << name << std::endl;
            }
            else
                std::cout << "Skipping: " << name
                          << " because its a " << nsw::getElementType(name)
                          << std::endl;
        }
        catch (std::exception & e) {
            std::cout << name << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }

    // the sender
    nsw::ConfigSender cs;

    // announce
    for (auto & addc: addc_configs){
        std::cout << "Found " << addc.getAddress() << " @ " << addc.getOpcServerIp() << std::endl;
        for (auto art: addc.getARTs()){
            std::cout << "Found " << art.getName()
                      << " with OpcServerIp_TP " << art.getOpcServerIp_TP()
                      << " and OpcNodeId_TP "    << art.getOpcNodeId_TP()
                      << " and TP_GBTxAlignmentBit " << art.TP_GBTxAlignmentBit()
                      << " and TP_GBTxAlignmentCommonPhases = ";
            for (auto ph: art.TP_GBTxAlignmentCommonPhases())
                std::cout << ph << " ";
            std::cout << std::endl;
        }
    }

    // configure
    auto threads = new std::vector< std::future<int> >();
    if (!dont_config) {
        for (auto & addc: addc_configs){
            std::cout << "Sending ADDC configuration... " << std::endl;
            threads->push_back( std::async(std::launch::async, configure_addc, addc) );
        }
        for (auto& thread: *threads)
            thread.get();
    }
    cs.alignAddcGbtxTp(addc_configs);

    // ART BCRCLK
    // uint phase_end = (uint)(bcr_phase);
    // size_t gbtx_size = 3;
    // uint8_t gbtx_data[] = {0x0,0x0,0x0};
    // uint8_t rbph_data[] = {0x0,0x0,0x0};
    // for (auto & addc: addc_configs) {
    //     for (auto art: addc.getARTs()) {
    //         auto opc_ip = addc.getOpcServerIp();
    //         auto name   = addc.getAddress() + "." + art.getNameGbtx();
    //         uint phase = 0;
    //         while (phase <= phase_end) {
    //             // coarse phase
    //             gbtx_data[0] = 11;
    //             gbtx_data[1] = 0;
    //             gbtx_data[2] = (uint8_t)(phase);
    //             cs.sendI2cRaw(opc_ip, name, gbtx_data, gbtx_size);
    //             // readback
    //             rbph_data[0] = 11; rbph_data[1] = 0; rbph_data[2] = (uint8_t)(phase);
    //             auto readback_phase = cs.readI2cAtAddress(opc_ip, name, rbph_data, 2, 1);
    //             if (readback_phase.size()==0)
    //                 throw std::runtime_error("Unable to readback phase in change_phase");
    //             // announce
    //             auto msg1 = opc_ip + "/" + name;
    //             auto msg2 = " set phase = " + std::to_string(phase) + " -> readback = " + std::to_string(readback_phase[0]);
    //             std::cout << msg1 << msg2 << std::endl;
    //             usleep(20000);
    //             phase = phase + 1;
    //         }
    //     }
    // }

    // smart alignment
    if (!dont_align) {

        // is anybody out there?
        bool go = 0;
        for (auto & addc: addc_configs)
            for (auto art: addc.getARTs())
                if (!art.TP_GBTxAlignmentSkip())
                    go = 1;
        if (!go)
            return 0;

        // variables for phase-setting
        size_t max_attempts = 5;
        auto regAddrVec = nsw::hexStringToByteVector("0x02", 4, true);

        // collect all TPs from the ARTs
        std::set< std::pair<std::string, std::string> > tps;
        for (auto & addc: addc_configs)
            for (auto art: addc.getARTs())
                tps.emplace(std::make_pair(art.getOpcServerIp_TP(), art.getOpcNodeId_TP()));
        for (auto tp: tps)
            std::cout << "Found TP for alignment: " << tp.first << "/" << tp.second << std::endl;

        // sleep time
        int sleeptime = 0;
        for (auto & addc: addc_configs)
            for (auto art: addc.getARTs())
                sleeptime = art.TP_GBTxAlignmentSleepTime();

        // loop over TPs
        for (auto tp: tps) {

            // keep track of alignment:
            // currently require 2 consecutive phases
            std::vector<bool> skip_me = {};
            std::vector<bool> prev_ok = {};
            std::vector<uint> cons_ok = {};
            for (int i = 0; i < 32; i++) {
                skip_me.push_back(1);
                prev_ok.push_back(0);
                cons_ok.push_back(0);
            }
            for (auto & addc: addc_configs)
                for (auto art: addc.getARTs())
                    if (art.IsMyTP(tp.first, tp.second) && !art.TP_GBTxAlignmentSkip())
                        skip_me[art.TP_GBTxAlignmentBit()] = 0;

            // first check: am I already aligned? if so, dont try to realign me.
            std::cout << tp.first << "/" << tp.second << " Reading register" << std::endl;
            auto outdata = cs.readI2cAtAddress(tp.first, tp.second, regAddrVec.data(), regAddrVec.size(), 4);
            std::cout << tp.first << "/" << tp.second << " Found " << nsw::vectorToBitString(outdata) << std::endl;
            for (auto & addc: addc_configs)
                for (auto art: addc.getARTs())
                    if (art.IsMyTP(tp.first, tp.second) && art.IsAlignedWithTP(outdata))
                        skip_me[art.TP_GBTxAlignmentBit()] = 1;


            std::ofstream myfileLive;
            if(map_phase){
                std::string fnameLive = "addc_alignment_" + strf_time() + "_phase_liveLoopingMonitor.txt";
                myfileLive.open(fnameLive);
                max_attempts = 1000;
            }

            // try N times
            for (size_t attempt = 0; attempt < max_attempts; attempt++) {

                bool done = 1;
                for (auto skip: skip_me)
                    if (!skip)
                        done = 0;
                if (done)
                    break;

                std::cout << "Alignment attempt " << attempt << std::endl;

                // loop over phases
                // for (uint phase = 0; phase < 32; phase++) {
                for (uint phase = 0; phase < 8; phase++) {

                    uint nFine = 1;
                    // uint nFine = 16;
                    // if (attempt>0) nFine = 16;
                    for (uint fine = 0; fine < nFine; fine++) {

                        // check if we need to align
                        bool at_least_one = 0;
                        for (auto & addc: addc_configs)
                            for (auto art: addc.getARTs())
                                if (art.IsMyTP(tp.first, tp.second) && !art.TP_GBTxAlignmentSkip() && !skip_me[art.TP_GBTxAlignmentBit()])
                                    at_least_one = 1;

                        // increment and check
                        if (at_least_one) {

                            // set each phase, updated
                            std::cout << tp.first << "/" << tp.second << " Changing phases of ARTs" << std::endl;
                            auto thrs = new std::vector< std::future<int> >();
                            for (auto & addc: addc_configs)
                                thrs->push_back( std::async(std::launch::async, change_phase, addc, phase, fine, skip_me) );
                            for (auto& thread: *thrs)
                                thread.get();
                            thrs->clear();

                            // query TP
                            std::cout << tp.first << "/" << tp.second << " Reading register" << std::endl;
                            usleep(sleeptime);
                            auto outdata = cs.readI2cAtAddress(tp.first, tp.second, regAddrVec.data(), regAddrVec.size(), 4);
                            std::cout << tp.first << "/" << tp.second << " Found " << nsw::vectorToBitString(outdata) << std::endl;

                            if(!map_phase){
                                // record output
                                for (auto & addc: addc_configs) {
                                    for (auto art: addc.getARTs()) {
                                        auto bit = art.TP_GBTxAlignmentBit();
                                        if (art.TP_GBTxAlignmentSkip() || !art.IsMyTP(tp.first, tp.second))
                                            continue;
                                        if (skip_me[bit])
                                            continue;
                                        auto ok = art.IsAlignedWithTP(outdata);
                                        skip_me[bit] = ok && prev_ok[bit];
                                        prev_ok[bit] = ok;
                                        cons_ok[bit] = ok ? cons_ok[bit]+1 : 0;
                                    }
                                }
                            } else {
                                myfileLive << "Time " << strf_time() << std::endl;
                                for (auto & addc: addc_configs){
                                    for (auto art: addc.getARTs()){
                                        auto aligned = art.IsAlignedWithTP(outdata);
                                        std::stringstream result;
                                        result << addc.getAddress() << " "
                                               << art.getName()     << " "
                                               << phase << " "
                                               << fine << " "
                                               << aligned << std::endl;
                                        myfileLive << result.str();
                                        // std::cout << result.str();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // alignment
    for (auto & addc: addc_configs){
        if (!dont_align) {
            // std::cout << "Aligning ADDC to TP... " << std::endl;
            // cs.alignAddcGbtxTp(addc);
        }
        if (manual_phase >= 0){
            for (uint phase = 0; phase <= (uint)(manual_phase); phase++) {
                // auto phase = (uint)(manual_phase);
                std::cout << "Sending manual phase " << phase << " to each ART" << std::endl;
                size_t gbtx_size = 3;
                uint8_t gbtx_data[] = {0x0,0x0,0x0};
                auto regAddrVec = nsw::hexStringToByteVector("0x02", 4, true);
                for (auto art: addc.getARTs()){
                    auto opc_ip = addc.getOpcServerIp();
                    auto name   = addc.getAddress() + "." + art.getNameGbtx();
                    auto opc_ip_tp = art.getOpcServerIp_TP();
                    auto name_tp   = art.getOpcNodeId_TP();
                    std::cout << opc_ip << "/" << name << " Setting phase = " << phase;
                    gbtx_data[1] = 0;
                    gbtx_data[0] = 8;
                    gbtx_data[2] = (uint8_t)(phase);
                    cs.sendI2cRaw(opc_ip, name, gbtx_data, gbtx_size);
                    usleep(50000);
                    // readback
                    auto gbtx_reg_read = cs.readI2cAtAddress(opc_ip, name, gbtx_data, 2, 1);
                    std::cout << " -> Readback = ";
                    for (uint i = 0; i < gbtx_reg_read.size(); i++)
                        std::cout << (unsigned)(gbtx_reg_read[i]) << " ";
                    std::cout << std::endl;
                    // TP
                    usleep(art.TP_GBTxAlignmentSleepTime());
                    auto outdata = cs.readI2cAtAddress(opc_ip_tp, name_tp, regAddrVec.data(), regAddrVec.size(), 4);
                    auto aligned = art.IsAlignedWithTP(outdata);
                    std::cout << "Result: "
                              << addc.getAddress() << " "
                              << art.getName()     << " "
                              << "-> alignment = " << aligned << std::endl;
                }
            }
        }
    }

    // watch alignment?
    if (!dont_align && !dont_watch) {
        size_t i  = 0;
        size_t slp = 5;
        std::string phase_position = "";
        for (auto & addc: addc_configs) {
            for (auto art: addc.getARTs()) {
                phase_position = art.TP_GBTxAlignmentPhase();
                break;
            }
            break;
        }
        std::string fname = "addc_alignment_" + strf_time() + "_" + phase_position + "_phase.txt";
        std::ofstream myfile;
        myfile.open(fname);
        std::cout << "Watching ADDC-TP alignment."              << std::endl;
        std::cout << "Sleep time: " + std::to_string(slp) + "s" << std::endl;
        std::cout << "Output file: " << fname                   << std::endl;
        std::cout << "Press Ctrl+C to exit"                     << std::endl;
        auto regAddrVec = nsw::hexStringToByteVector("0x02", 4, true);
        while (true) {
            myfile << "Time " << strf_time() << std::endl;
            for (auto & addc: addc_configs){
                for (auto art: addc.getARTs()){
                    auto outdata = cs.readI2cAtAddress(art.getOpcServerIp_TP(), art.getOpcNodeId_TP(), regAddrVec.data(), regAddrVec.size(), 4);
                    usleep(10000);
                    auto aligned = art.IsAlignedWithTP(outdata);
                    std::stringstream result;
                    result << addc.getAddress() << " "
                           << art.getName()     << " "
                           << aligned << std::endl;
                    myfile << result.str();
                    std::cout << result.str();
                }
            }
            if (i % 100 == 0)
                std::cout << std::endl << strf_time() << " " << std::endl << std::flush;
            std::cout << "." << std::endl << std::flush;
            i++;
            sleep(slp);
        }
        myfile.close();
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

std::string strf_time() {
    std::stringstream ss;
    std::string out;
    std::time_t result = std::time(nullptr);
    std::tm tm = *std::localtime(&result);
    ss << std::put_time(&tm, "%Y_%m_%d_%Hh%Mm%Ss");
    ss >> out;
    return out;
}

int configure_addc(nsw::ADDCConfig addc) {
    std::cout << "New thread in configure_frontend for " << addc.getAddress() << std::endl;
    nsw::ConfigSender cs;
    cs.sendAddcConfig(addc);
    return 0;
}

int change_phase(nsw::ADDCConfig addc, uint phase, uint fine, std::vector<bool> aligned) {
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0};
    uint8_t rbph_data[] = {0x0,0x0,0x0};
    uint8_t rbfi_data[] = {0x0,0x0,0x0};
    for (auto art: addc.getARTs()) {
        auto bit = (size_t)(art.TP_GBTxAlignmentBit());
        if (aligned.at(bit))
            continue;
        auto opc_ip = addc.getOpcServerIp();
        auto name   = addc.getAddress() + "." + art.getNameGbtx();
        // coarse phase
        gbtx_data[1] = 0;
        gbtx_data[0] = 8;
        gbtx_data[2] = (uint8_t)(phase);
        local_sender->sendI2cRaw(opc_ip, name, gbtx_data, gbtx_size);
        // fine phase
        gbtx_data[1] = 0;
        gbtx_data[0] = 4;
        gbtx_data[2] = (uint8_t)(fine);
        local_sender->sendI2cRaw(opc_ip, name, gbtx_data, gbtx_size);
        // readback
        rbph_data[0] = 8; rbph_data[1] = 0; rbph_data[2] = (uint8_t)(phase);
        rbfi_data[0] = 4; rbfi_data[1] = 0; rbfi_data[2] = (uint8_t)(fine);
        auto readback_phase = local_sender->readI2cAtAddress(opc_ip, name, rbph_data, 2, 1);
        auto readback_fine  = local_sender->readI2cAtAddress(opc_ip, name, rbfi_data, 2, 1);
        if (readback_phase.size()==0)
            throw std::runtime_error("Unable to readback phase in change_phase");
        if (readback_fine.size()==0)
            throw std::runtime_error("Unable to readback fine in change_phase");
        // announce
        auto msg1 = opc_ip + "/" + name;
        auto msg2 = " set phase = " + std::to_string(phase) + " -> readback = " + std::to_string(readback_phase[0]);
        auto msg3 = " set fine  = " + std::to_string(fine)  + " -> readback = " + std::to_string(readback_fine[0]);
        std::cout << msg1 << msg2 << msg3 << std::endl;
    }
    return 0;
}

