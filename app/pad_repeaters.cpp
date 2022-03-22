//
// Program to write values to the Pad Trigger repeater chips,
//    via SCA I2C.
//
#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/Utility.h"

#include <fmt/core.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, const char *argv[]) {
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
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(config_files+"config_json/191/A14/STGC_191_A14_HOIP_PAD_TEST.json"), "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("rep,r", po::value<std::string>(&rep)
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
        std::cerr << "Error: please give a repeater number (-r) which is 1, 2, 3, 4, 5, or 6." << std::endl;
        return 1;
    }
    if (i2c_addr == "") {
        std::cerr << "Error: please give an I2C address (--address) to write to." << std::endl;
        return 1;
    }
    if (i2c_val == "") {
        std::cerr << "Error: please give a value (--value) to write." << std::endl;
        return 1;
    }

    // Repeater I2C
    uint8_t address_repeater[]   = {(uint8_t)(std::stol(i2c_addr, 0, 16) & 0xff)};
    uint8_t data_data_repeater[] = {(uint8_t)(std::stol(i2c_addr, 0, 16) & 0xff),
                                    (uint8_t)(std::stol(i2c_val,  0, 16) & 0xff)};

    // make objects from json
    auto board_configs = nsw::ConfigReader::makeObjects<boost::property_tree::ptree>
      ("json://" + config_filename, "PadTrigger");
    if (board_configs.empty()) {
      std::cerr << "You provided a JSON with no pad triggers! Exiting." << std::endl;
      return 1;
    }

    // the hw objects
    nsw::OpcManager opcManager{};
    std::vector<nsw::hw::PadTrigger> hws;
    hws.reserve(board_configs.size());
    for (const auto& cfg: board_configs) {
      hws.emplace_back(nsw::hw::PadTrigger(opcManager, cfg));
    }
    for (auto & board : hws) {
        std::cout << fmt::format("Found {}. Configuring Repeater {}", board.getName(), rep) << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Configuring Repeater " << rep << std::endl;
    std::cout << std::endl;

    // loop over pad triggers
    for (auto & board : hws) {

        const auto gpio = fmt::format("gpio-repeaterChip{}", rep);
        const auto rep8 = static_cast<std::uint8_t>(std::stoul(rep));

        // GPIO enable
        std::cout << "Repeater GPIO. Writing 1" << std::endl;
        if (!dry_run) {
            board.writeGPIO(gpio, true);
        }
        usleep(10000);
        std::cout << " Readback " << rep << ": "
                  << (dry_run ? -1 : board.readGPIO(gpio))
                  << std::endl;
        std::cout << std::endl;

        // Repeater I2C: write
        std::cout << "Repeater I2C. Writing 0x" << std::hex << unsigned(data_data_repeater[1])
                  << " to address 0x" << unsigned(data_data_repeater[0]) << std::dec << std::endl;
        if (!dry_run) {
            board.writeRepeaterRegister(rep8, data_data_repeater[0], data_data_repeater[1]);
        }
        usleep(10000);

        // Repeater I2C: readback
        std::uint8_t val{0};
        if (!dry_run) {
            val = board.readRepeaterRegister(rep8, address_repeater[0]);
        } else {
            val = 0;
        }
        std::cout << " Readback " << rep << ": 0x" << std::hex << unsigned(val) << std::dec << std::endl;
        usleep(10000);
        std::cout << std::endl;

        // GPIO disable
        std::cout << "Repeater GPIO. Writing 0" << std::endl;
        if (!dry_run) {
            board.writeGPIO(gpio, false);
        }
        usleep(10000);
        std::cout << " Readback " << rep << ": "
                  << (dry_run ? -1 : board.readGPIO(gpio))
                  << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
