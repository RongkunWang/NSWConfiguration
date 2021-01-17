#include "NSWConfiguration/MemfileReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/TPConfig.h"

#include <boost/program_options.hpp>

#include <utility>
#include <string>

namespace po = boost::program_options;

std::map<uint16_t, std::vector<uint32_t>> parse_memfile(const std::string& filename);
void write_bram(const std::map<uint16_t, std::vector<uint32_t>>& contents, const std::string& opc_server,
    const std::string& opc_node);
nsw::TPConfig get_tp_config(const std::string& config_file, const std::string& tp_name);

int main(int argc, char** argv) {
    std::string description = "A program for writing to bRAM on the trigger processor, using the Xilinx MEM format";

    std::string config_filename;
    std::string mem_filename;
    std::string tp_name;
    bool dry_run;

    po::options_description po_desc(description);
    po_desc.add_options()
        ("help,h", "Display help message")
        ("configfile,c", po::value<std::string>(&config_filename)->required(), "Configuration file path")
        ("memfile,m", po::value<std::string>(&mem_filename)->required(), "MEM file path")
        ("dry-run", po::bool_switch(&dry_run)->default_value(false), "DON'T send bRAM contents to trigger processor")
        ("tp,t", po::value<std::string>(&tp_name)->required(), "Name of trigger processor");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, po_desc), vm);
    
    if (vm.count("help")) {
        std::cout << po_desc << std::endl;
        return 0;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Parsing memory file..." << std::endl;
    const auto memory = parse_memfile(config_filename);
    
    std::cout << "Parsing configuration file..." << std::endl;
    auto tp_config = get_tp_config(config_filename, tp_name);

    if (!dry_run) {
        std::cout << "Writing bRAM contents to trigger processor..." << std::endl;
        write_bram(memory, tp_config.getOpcServerIp(), tp_config.getAddress());
    }
}

std::map<uint16_t, std::vector<uint32_t>> parse_memfile(const std::string& filename) {
    MemfileReader mr{ filename, '\n' };
    return mr.read();
}

void write_bram(const std::map<uint16_t, std::vector<uint32_t>>& contents, const std::string& opc_server,
    const std::string& opc_node) {
    nsw::ConfigSender cs;
    for (const auto& [base_addr, data] : contents) {
        // send write address to SCAx Memory Controller
        // TODO: fill the I2C addresses here!
        cs.sendI2cAtAddress(opc_server, opc_node, {0x0, 0x0, 0x0, 0x0}, nsw::intToByteVector(base_addr, 4));
        for (const auto val : data) {
            // Send the actual data
            cs.sendI2cAtAddress(opc_server, opc_node, {0x0, 0x0, 0x0, 0x0}, nsw::intToByteVector(val, 4));
        }
    }

}

nsw::TPConfig get_tp_config(const std::string& config_file, const std::string& tp_name) {
    nsw::ConfigReader config_reader{ config_file };
    boost::property_tree::ptree tp_config_tree{};
    try {
        config_reader.readConfig();
        tp_config_tree = config_reader.readConfig(tp_name);
    }
    catch (const std::exception& ex) {
        std::cout << "Can't read config file due to : " << ex.what() << std::endl;
        std::cout << "Exiting..." << std::endl;
        std::exit(1);
    }
    return nsw::TPConfig{ tp_config_tree };
}