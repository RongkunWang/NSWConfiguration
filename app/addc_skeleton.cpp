// Program to set ADDC configuration?

#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/Utility.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

std::string strf_time();

int main(int argc, const char *argv[]) 
{
    std::string config_filename;
    std::string board_name;
    bool dont_config;
    bool dont_align;
    bool dont_watch;

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
        ("name,n", po::value<std::string>(&board_name)->
         default_value(""), "The name of frontend to configure (should start with ADDC_).");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dont_config = vm["dont_config"].as<bool>();
    dont_align  = vm["dont_align" ].as<bool>();
    dont_watch  = vm["dont_watch"].as<bool>();

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

    // announce and go
    for (auto & addc: addc_configs){
        std::cout << "Found " << addc.getAddress() << " @ " << addc.getOpcServerIp() << std::endl;
        for (auto art: addc.getARTs()){
            std::cout << "Found " << art.getName()
                      << " with OpcServerIp_TP " << art.getOpcServerIp_TP()
                      << " and OpcNodeId_TP "    << art.getOpcNodeId_TP()
                      << " and TP_GBTxAlignmentBit " << art.TP_GBTxAlignmentBit()
                      << std::endl;
        }
        if (!dont_config) {
            std::cout << "Sending ADDC configuration... " << std::endl;
            cs.sendAddcConfig(addc);
        }
        if (!dont_align) {
            std::cout << "Aligning ADDC to TP... " << std::endl;
            cs.alignAddcGbtxTp(addc);
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
        while (false) {
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
                }
            }
            if (i % 100 == 0)
                std::cout << std::endl << strf_time() << " " << std::flush;
            std::cout << "." << std::flush;
            i++;
            sleep(slp);
        }
        myfile.close();
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

