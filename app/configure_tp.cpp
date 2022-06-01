// Sample program to read configuration from db/json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPConfig.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/MMTP.h"

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
using boost::property_tree::ptree;

int main(int ac, const char *av[]) {
    std::string description = "This program is for sending/receiving messages from the SCAX on the TP.";
    std::string confdir = "/afs/cern.ch/user/n/nswdaq/public/sw1/config-ttc/config-files/";
    std::string config_filename;
    std::string tp_name;
    bool dry_run{false};
    bool hw{false};
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(confdir + "full_small_sector_14_internalPulser_ADDC.json"),
        "Configuration file path")
        ("dry_run",   po::bool_switch()->default_value(false), "Option to NOT send configurations")
        ("hw",        po::bool_switch()->default_value(false), "Option to use hw interface")
        ("tp,t", po::value<std::string>(&tp_name)->
        default_value("MMTP_A14"),
        "Name of trigger processor");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    dry_run = vm["dry_run"].as<bool>();
    hw      = vm["hw"]     .as<bool>();
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    const auto json_filename = fmt::format("json://{}", config_filename);

    auto opcManager = nsw::OpcManager{};

    // STGC TP
    const auto stgc_tp_configs = nsw::ConfigReader::makeObjects<boost::property_tree::ptree>
      (json_filename, "STGCTP", tp_name);
    auto stgc_tps = std::vector<nsw::hw::STGCTP>{};
    for (const auto& config : stgc_tp_configs) {
      stgc_tps.emplace_back(opcManager, config);
    }
    for (const auto& tp: stgc_tps) {
      std::cout << fmt::format("Found STGC TP {}", tp.getName()) << std::endl;
      tp.writeConfiguration();
      for (const auto& [reg, val]: tp.readConfiguration()) {
        std::cout << fmt::format("Reg {:#04x}: val = {:#010x}", reg, val) << std::endl;
      }
    }

    // MM TP
    const auto configs = nsw::ConfigReader::makeObjects<nsw::TPConfig>(json_filename, "TP", tp_name);

    if (hw) {
      // hw interface
      auto mmtps = std::vector<nsw::hw::MMTP>{};
      for (const auto& config : configs) {
        mmtps.emplace_back(opcManager, config);
      }
      for (const auto& tp: mmtps) {
        for (const auto& [addr, val]: tp.readConfiguration()) {
          fmt::print("{:#010x}: {:#010x}\n", addr, val);
        }
        tp.writeConfiguration();
      }
    } else {
      // config sender
      for (const auto& tp: configs) {
        std::cout << fmt::format("Found MM TP {}/{}", tp.getOpcServerIp(), tp.getAddress()) << std::endl;
      }
      nsw::ConfigSender cs;
      for (const auto& tp: configs) {
        tp.dump();
        if (!dry_run) {
          cs.sendTPConfig(tp);
        }
      }
    }

    return 0;
}

