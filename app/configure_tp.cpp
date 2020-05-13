// Sample program to read configuration from db/json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPConfig.h"

#include "boost/program_options.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace po = boost::program_options;
namespace pt = boost::property_tree;
using boost::property_tree::ptree;

std::string strf_time();

int main(int ac, const char *av[]) {
    std::string description = "This program is for sending/receiving messages from the SCX on the TP.";
    std::string confdir = "/afs/cern.ch/user/n/nswdaq/public/sw1/config-ttc/config-files/";
    std::string config_filename;
    std::string tp_name;
    bool dry_run;
    bool do_bcr;
    bool do_phase;
    bool phase_daq;
    bool no_conf;
    int user_phase;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(confdir + "full_small_sector_14_internalPulser_ADDC.json"),
        "Configuration file path")
        ("no_conf",   po::bool_switch()->default_value(false), "Option to not send config")
        ("dry_run",   po::bool_switch()->default_value(false), "Option to NOT send configurations")
        ("bcr",       po::bool_switch()->default_value(false), "Option to send BCR while doing phase stuff")
        ("do_phase",  po::bool_switch()->default_value(false), "Option to do phase stuff")
        ("phase_daq", po::bool_switch()->default_value(false), "Option to do phase stuff")
        ("phase", po::value<int>(&user_phase)->default_value(-1), "Option to set the phase")
        ("tp,t", po::value<std::string>(&tp_name)->
        default_value("MMTP_A14"),
        "Name of trigger processor");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    dry_run   = vm["dry_run"  ].as<bool>();
    do_bcr    = vm["bcr"      ].as<bool>();
    do_phase  = vm["do_phase" ].as<bool>();
    phase_daq = vm["phase_daq"].as<bool>();
    no_conf   = vm["no_conf"  ].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    // Testing json parsing
    nsw::ConfigReader reader_tp("json://" + config_filename);
    reader_tp.readConfig();
    auto tp_config_tree = pt::ptree();
    try {
      tp_config_tree = reader_tp.readConfig(tp_name);
    }
    catch (std::exception &e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::cout << "Parsed ptree, about to build TPConfig" << std::endl;

    nsw::TPConfig tp(tp_config_tree);
    tp.dump();

    // setRegisterValue(std::string master, std::string slave, uint32_t value);
    // getRegisterValue(std::string master, std::string slave);

    nsw::ConfigSender cs;  // in principle the config sender is all that is needed for now

    std::cout << "Created a ConfigSender" << std::endl;
    if (!dry_run && !no_conf)
        cs.sendTpConfig(tp);

    std::cout << "... Done with configure_tp" << std::endl;

    // phase stuff
    if (!do_phase)
        return 0;
    auto ip                        = tp.getOpcServerIp();
    auto addr                      = tp.getAddress();
    uint8_t nphase                 = 8;
    uint8_t phreg                  = 0x0B;
    auto align                     = nsw::hexStringToByteVector("0x02", 4, true);
    std::vector<std::string> bxlsb = {"0x04", "0x05", "0x06", "0x07"};
    std::vector<uint8_t> data      = {0x55, 0x55, 0x55, 0x55};
    std::string bcr                = do_bcr ? "/afs/cern.ch/user/n/nswdaq/public/alti/ALTI_bcr.expect 11" : "echo";
    std::string now                = strf_time();
    int nreads                     = 100;

    system(bcr.c_str());

    for (uint8_t ph = 0; ph < nphase; ph++) {
        if (user_phase != -1 && static_cast<int>(ph) != user_phase)
            continue;

        // set phase
        std::cout << "Set phase: " << int(ph) << std::endl;
        if (!dry_run) {
            cs.sendI2cAtAddress(ip, addr, {0x00, 0x00, 0x00, phreg}, 
                                nsw::intToByteVector(static_cast<int>(ph), 4, true));
            usleep(500e3);
            system(bcr.c_str());
            usleep(999e3);
        }

        // read alignment
        std::cout << "Read alignment: ";
        if (!dry_run)
            data = cs.readI2cAtAddress(ip, addr, align.data(), align.size(), 4);
        for (auto byte : data)
            std::cout << std::bitset<8>(byte);
        std::cout << std::endl;

        // read BC LSBs
        std::cout << "Read BC LSB: " << std::endl;;
        for (auto reg : bxlsb) {
            auto bxdata = nsw::hexStringToByteVector(reg, 4, true);
            if (!dry_run)
                data = cs.readI2cAtAddress(ip, addr, bxdata.data(), bxdata.size(), 4);
            std::cout << reg << " ";
            for (auto byte : data)
                std::cout << std::hex << unsigned(byte) << std::dec;
            std::cout << std::endl;
        }

        // "DAQ"
        if (phase_daq) {
            std::cout << "Read alignment and BC LSBs " << nreads << " times" << std::endl;
            std::ofstream myfile;
            myfile.open("tpscax_" + now + "_phase" + std::to_string(static_cast<int>(ph)) + ".txt");
            for (int i = 0; i < nreads; i++) {
                // BCR
                if (!dry_run) {
                    system(bcr.c_str());
                    usleep(999e3);
                }
                myfile << strf_time() << " ";

                // alignment
                if (!dry_run)
                    data = cs.readI2cAtAddress(ip, addr, align.data(), align.size(), 4);
                for (auto byte : data)
                    myfile << std::bitset<8>(byte);
                myfile << " ";

                // BC LSBs
                for (auto reg : bxlsb) {
                    auto bxdata = nsw::hexStringToByteVector(reg, 4, true);
                    if (!dry_run)
                        data = cs.readI2cAtAddress(ip, addr, bxdata.data(), bxdata.size(), 4);
                    for (auto byte : data)
                        myfile << std::hex << std::setfill('0') << std::setw(2) << unsigned(byte) << std::dec;
                }
                myfile << std::endl;
            }
            myfile.close();
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
