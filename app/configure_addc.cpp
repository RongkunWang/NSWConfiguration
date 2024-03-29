// Program to set ADDC configuration?

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <future>

#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ADDC.h"
#include "NSWConfiguration/hw/MMTP.h"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/Utility.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

int active_threads(std::vector< std::future<int> >* threads);
int configure_addc(const nsw::hw::ADDC& addc, int iart);
int change_phase(nsw::ADDCConfig addc, uint phase, uint fine, std::vector<bool> aligned);
std::string strf_time();

int main(int argc, const char *argv[])
{
    std::string config_filename;
    std::string board_name;
    bool dont_config;
    bool dont_watch;
    int manual_phase;
    int bcr_phase;
    int iart;

    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/A10/full_small_sector_a10_bb5_ADDC_TP.json"),
         "Configuration file path")
        ("dont_config", po::bool_switch()->
         default_value(false), "Option to NOT configure the ADDCs")
        ("dont_watch", po::bool_switch()->
         default_value(false), "Option to NOT monitor the ADDC-TP alignment vs time")
        ("manual_phase", po::value<int>(&manual_phase)->
         default_value(-1), "Manual phase of ART alignment")
        ("bcr_phase", po::value<int>(&bcr_phase)->
         default_value(0), "Manual phase of ART BCRCLK")
        ("iart,i", po::value<int>(&iart)->
         default_value(-1), "Configure a specific ART (0 or 1)")
        ("name,n", po::value<std::string>(&board_name)->
         default_value(""), "The name of frontend to configure (should start with ADDC_).");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dont_config = vm["dont_config"].as<bool>();
    dont_watch  = vm["dont_watch"].as<bool>();

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // addc objects
    auto cfg = "json://" + config_filename;
    auto addc_configs = nsw::ConfigReader::makeObjects<nsw::ADDCConfig>(cfg, "ADDC", board_name);

    auto addcs = std::vector<nsw::hw::ADDC>{};
    auto opcManager = nsw::OpcManager{};

    // announce
    for (auto & addc : addc_configs) {
        std::cout << "Found " << addc.getAddress() << " @ " << addc.getOpcServerIp() << std::endl;
        for (auto art : addc.getARTs()) {
            std::cout << "Found "                    << art.getName()
                      << " with OpcServerIp_TP "     << art.getOpcServerIp_TP()
                      << " and OpcNodeId_TP "        << art.getOpcNodeId_TP()
                      << " and TP_GBTxAlignmentBit " << art.TP_GBTxAlignmentBit()
                      << std::endl;
        }
        addcs.emplace_back(opcManager, addc);
    }


    // configure
    auto threads = new std::vector< std::future<int> >();
    if (!dont_config) {
        for (auto & addc : addcs) {
            std::cout << "Sending ADDC configuration... " << std::endl;
            threads->push_back(std::async(std::launch::async, configure_addc, addc, iart));
        }
        for (auto& thread : *threads)
            thread.get();
    }

    // to check: reading GBTx phase registers
    // for (auto & addc: addc_configs) {
    //     for (auto art: addc.getARTs()) {
    //         auto opc_ip = addc.getOpcServerIp();
    //         auto name   = addc.getAddress() + "." + art.getNameGbtx();
    //         for (uint8_t reg = 66; reg < 222; reg++) {
    //             if ((reg <  66 || reg >  77) &&
    //                 (reg <  90 || reg > 101) &&
    //                 (reg < 114 || reg > 125) &&
    //                 (reg < 138 || reg > 149) &&
    //                 (reg < 162 || reg > 173) &&
    //                 (reg < 186 || reg > 197) &&
    //                 (reg < 210 || reg > 221))
    //                 continue;
    //             uint8_t rbph_data[] = {0x0, 0x0, 0x0};
    //             rbph_data[0] = reg; rbph_data[1] = 0; rbph_data[2] = 0;
    //             auto readback_phase = cs.readI2cAtAddress(opc_ip, name, rbph_data, 2, 1);
    //             if (readback_phase.size()==0)
    //                 throw std::runtime_error("Unable to readback phase in change_phase");
    //             auto msg1 = opc_ip + "/" + name;
    //             auto msg2 = " reg = " + std::to_string(unsigned(reg)) + " -> readback = " + std::to_string(readback_phase[0]);
    //             std::cout << msg1 + msg2 << std::endl;
    //         }
    //     }
    // }
    // </check>

    // watch alignment?
    if (!dont_watch) {

        // sleep time
        size_t slp = 1;

        // collect all TPs from the ARTs
        std::set< std::pair<std::string, std::string> > mmtp_configs;
        for (const auto & addc : addc_configs)
            for (const auto &art : addc.getARTs())
                mmtp_configs.emplace(std::make_pair(art.getOpcServerIp_TP(), art.getOpcNodeId_TP()));

        auto mmtps = std::vector<nsw::hw::MMTP>{};
        for (const auto& config : mmtp_configs) {
          pt::ptree ptree;
          ptree.put("OpcServerIp", config.first);
          ptree.put("OpcNodeId", config.second);

          mmtps.emplace_back(opcManager, ptree);
        }

        for (const auto& tp : mmtp_configs)
            std::cout << "Found TP for alignment: " << tp.first << "/" << tp.second << std::endl;

        // output file
        std::string fname = "addc_alignment_" + strf_time() + ".txt";
        std::ofstream myfile;
        myfile.open(fname);
        std::cout << "Watching ADDC-TP alignment."              << std::endl;
        std::cout << "Sleep time: " + std::to_string(slp) + "s" << std::endl;
        std::cout << "Output file: " << fname                   << std::endl;
        std::cout << "Press Ctrl+C to exit"                     << std::endl;
        auto regAddrVec = nsw::hexStringToByteVector("0x02", 4, true);
        while (true) {
            myfile << "Time " << strf_time() << std::endl;
            for (const auto& tp : mmtps) {
                // auto outdata = cs.readI2cAtAddress(tp.first, tp.second,
                                                   // regAddrVec.data(), regAddrVec.size(), 4);
                auto outdata = tp.readRegister(0x02);
                for (auto & addc : addc_configs) {
                    for (auto art : addc.getARTs()) {
                        auto aligned = art.IsAlignedWithTP(outdata);
                          std::stringstream result;
                          result << addc.getAddress()         << " "
                                 << art.getName()             << " "
                                 << art.TP_GBTxAlignmentBit() << " "
                                 << aligned << std::endl;
                          myfile << result.str();
                          std::cout << result.str();
                    }
                }
            }
            std::cout << "." << std::endl << std::flush;
            sleep(slp);
        }
        myfile.close();
    }

    return 0;
}

int active_threads(std::vector< std::future<int> >* threads) {
    int nfinished = 0;
    for (auto& thread : *threads)
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

int configure_addc(const nsw::hw::ADDC& addc, int iart) {
    std::cout << "New thread in configure_frontend for " << addc.getScaAddress() << " art index(indices) "<< iart << std::endl;
    
    addc.writeConfiguration(iart);
    return 0;
}

// TODO obsolete?
int change_phase(nsw::ADDCConfig addc, uint phase, uint fine, std::vector<bool> aligned) {
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0, 0x0, 0x0};
    uint8_t rbph_data[] = {0x0, 0x0, 0x0};
    uint8_t rbfi_data[] = {0x0, 0x0, 0x0};
    for (auto art : addc.getARTs()) {
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

