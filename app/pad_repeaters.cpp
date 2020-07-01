// Program to set Pad Trigger configuration

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/PadTriggerSCAConfig.h"
#include "NSWConfiguration/Utility.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;
    std::string gpio_name;
    std::string adc_name;
    std::string i2c_addr;
    std::string i2c_val;
    std::string rep;
    bool dry_run;

    // options
    po::options_description desc(std::string("Pad trigger repeaters configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file", po::value<std::string>(&config_filename)
         ->default_value(config_files+"config_json/191/A14/STGC_191_A14_HOIP_PAD_TEST.json"), "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("rep", po::value<std::string>(&rep)
         ->default_value(""), "Repeater number (1, 2, 3, 4, 5, 6)")
        ("address", po::value<std::string>(&i2c_addr)
         ->default_value(""), "i2c register for SCA communication (0xHEX).")
        ("value", po::value<std::string>(&i2c_val)
         ->default_value(""), "i2c value to write (0xHEX)")
        ("dry_run", po::bool_switch()->
         default_value(false), "Option to NOT send any communication")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dry_run = vm["dry_run"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // checks
    if (rep != "1" &&
        rep != "2" &&
        rep != "3" &&
        rep != "4" &&
        rep != "5" &&
        rep != "6") {
        throw std::runtime_error("Repeater number must be 1, 2, 3, 4, 5, 6");
    }

    // Repeater I2C
    size_t address_size_repeater = 1;
    size_t data_size_repeater    = 2;
    uint8_t address_repeater[]   = {(uint8_t)(std::stol(i2c_addr, 0, 16) & 0xff)};
    uint8_t data_data_repeater[] = {(uint8_t)(std::stol(i2c_addr, 0, 16) & 0xff), (uint8_t)(std::stol(i2c_val, 0, 16) & 0xff)};

    // make objects from json
    auto board_configs = nsw::ConfigReader::makeObjects<nsw::PadTriggerSCAConfig>("json://" + config_filename, "PadTriggerSCA");
    for (auto & board: board_configs)
        std::cout << "Found " << board.getAddress() << " @ " << board.getOpcServerIp() << ". Configuring Repeater " << rep << std::endl;
    std::cout << std::endl;
    std::cout << "Configuring Repeater " << rep << std::endl;
    std::cout << std::endl;

    // the sender
    nsw::ConfigSender cs;

    // loop over pad triggers
    for (auto & board: board_configs) {

        auto opc_ip   = board.getOpcServerIp();
        auto sca_addr = board.getAddress();
        auto node     = sca_addr + ".repeaterChip" + rep + ".repeaterChip" + rep;

        // GPIO enable
        std::cout << "Repeater GPIO. Writing 1" << std::endl;
        if (!dry_run)
            cs.sendGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep, 1);
        usleep(10000);
        std::cout << " Readback " << rep << ": " << (dry_run ? -1 : cs.readGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep)) << std::endl;
        std::cout << std::endl;

        // Repeater I2C: write
        std::cout << "Repeater I2C. Writing 0x" << std::hex << unsigned(data_data_repeater[1])
                  << " to address 0x" << unsigned(data_data_repeater[0]) << std::dec << std::endl;
        if (!dry_run)
            cs.sendI2cRaw(opc_ip, node, data_data_repeater, data_size_repeater);
        usleep(10000);

        // Repeater I2C: readback
        std::vector<uint8_t> val;
        if (!dry_run)
            val = cs.readI2cAtAddress(opc_ip, node, address_repeater, address_size_repeater);
        else
            val.push_back(0);
        std::cout << " Readback " << rep << ": 0x" << std::hex << unsigned(val[0]) << std::dec << std::endl;
        usleep(10000);
        std::cout << std::endl;
    
        // GPIO disable
        std::cout << "Repeater GPIO. Writing 0" << std::endl;
        if (!dry_run)
            cs.sendGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep, 0);
        usleep(10000);
        std::cout << " Readback " << rep << ": " << (dry_run ? -1 : cs.readGPIO(opc_ip, sca_addr + ".gpio.gpio-repeaterChip" + rep)) << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
