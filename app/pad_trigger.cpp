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

double adc2temp(int adc);
std::vector<nsw::PadTriggerSCAConfig> parse_name(std::string name, std::string cfg);

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;
    std::string gpio_name;
    std::string adc_name;
    std::string i2c_reg;
    std::string i2c_val;
    bool do_config;
    int samples;
    int val;

    // options
    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file", po::value<std::string>(&config_filename)
         ->default_value(config_files+"padTrigger.json"), "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("gpio", po::value<std::string>(&gpio_name)
         ->default_value(""), "GPIO name to read/write (check the xml for valid names).")
        ("adc", po::value<std::string>(&adc_name)
         ->default_value(""), "ADC name to read (check the xml for valid names).")
        ("do_config", po::bool_switch()->
         default_value(false), "Option to send predefined configuration")
        ("val", po::value<int>(&val)
         ->default_value(-1), "Multi-purpose value for reading and writing. If no value given, will read-only.")
        ("i2c_reg", po::value<std::string>(&i2c_reg)
         ->default_value(""), "i2c register for SCA communication (0xHEX).")
        ("i2c_val", po::value<std::string>(&i2c_val)
         ->default_value(""), "i2c value to write (0xHEX)")
        ("samples", po::value<int>(&samples)
         ->default_value(100), "Number of samples when reading SCA ADC.")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    do_config = vm["do_config"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // make objects from json
    auto board_configs = parse_name(board_name, config_filename);
    for (auto & board: board_configs)
        std::cout << "Found " << board.getAddress() << " @ " << board.getOpcServerIp() << std::endl;

    // the sender
    nsw::ConfigSender cs;

    // GPIO read/write
    if (gpio_name != "") {
        for (auto & board: board_configs) {
            auto ip   = board.getOpcServerIp();
            auto addr = board.getAddress() + ".gpio." + gpio_name;
            auto name = ip + "::" + addr;
            if (val != -1) {
                std::cout << name << " -> write " << val << std::endl;
                cs.sendGPIO(ip, addr, val);
            }
            std::cout << name << " -> read  " << cs.readGPIO(ip, addr) << std::endl;
        }
    }

    // SCA ADC
    if (adc_name != "") {
        for (auto & board: board_configs) {
            auto ip   = board.getOpcServerIp();
            auto addr = board.getAddress() + ".ai." + adc_name;
            auto name = ip + "::" + addr;
            std::cout << name << " -> read ADC" << std::endl;
            auto datas = cs.readAnalogInputConsecutiveSamples(ip, addr, samples);
            for (auto data : datas)
                if (adc_name == "internalTemperature")
                    std::cout << " " << adc_name << " " << data << " -> " << adc2temp(data) << "C" << std::endl;
                else
                    std::cout << " " << adc_name << " " << data << std::endl;
        }
    }

    // pad i2c
    if (i2c_reg != "") {
        for (auto & board: board_configs) {
            auto ip   = board.getOpcServerIp();
            auto addr = board.getAddress() + ".fpga.fpga";
            uint8_t address[]    = {(uint8_t)(std::stol(i2c_reg, 0, 16) & 0xff)};
            size_t  address_size = 1;
            size_t  data_size    = 4;
            if (i2c_val != "") {
                uint32_t i2c_val_32 = (uint32_t)(std::stol(i2c_val, 0, 16));
                uint8_t data_data[]  = {address[0],
                                        (uint8_t)((i2c_val_32 >> 24) & 0xff),
                                        (uint8_t)((i2c_val_32 >> 16) & 0xff),
                                        (uint8_t)((i2c_val_32 >>  8) & 0xff),
                                        (uint8_t)((i2c_val_32 >>  0) & 0xff)};
                std::cout << " Writing  " << addr
                          << " reg "      << i2c_reg
                          << " val "      << i2c_val
                          << " -> msg = ";
                for (auto val : data_data)
                    std::cout << std::hex << unsigned(val) << " " << std::dec;
                std::cout << std::endl;
                cs.sendI2cRaw(ip, addr, data_data, address_size + data_size);
            }
            std::cout << " Readback " << addr << ": ";
            auto vals = cs.readI2cAtAddress(ip, addr, address, address_size, data_size);
            for (auto val : vals)
                std::cout << std::hex << unsigned(val) << " " << std::dec;
            if (address[0] == 1) {
                uint32_t temp = 0;
                temp += (uint32_t)(vals[0] << 24);
                temp += (uint32_t)(vals[1] << 16);
                temp += (uint32_t)(vals[2] <<  8);
                temp += (uint32_t)(vals[3] <<  0);
                std::cout << " -> " << (temp*503.975/4096)-273.15 << "C" << std::endl;
            }
            std::cout << std::endl;
        }
    }

    // config
    if (do_config)
        for (auto & board: board_configs)
            cs.sendPadTriggerSCAConfig(board);

    return 0;
}

double adc2temp(int adc) {
    return (0.79-adc/4095.0)*545.454545455 - 40;
}

std::vector<nsw::PadTriggerSCAConfig> parse_name(std::string name, std::string cfg) {

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

    // make board objects
    std::vector<nsw::PadTriggerSCAConfig> board_configs;
    for (auto & this_name : names) {
        try {
            if (nsw::getElementType(this_name) == "PadTriggerSCA") {
                board_configs.emplace_back(reader1.readConfig(this_name));
                std::cout << "Adding: " << this_name << std::endl;
            }
            else
                std::cout << "Skipping: " << this_name
                          << " because its a " << nsw::getElementType(this_name)
                          << std::endl;
        }
        catch (std::exception & e) {
            std::cout << this_name << " - ERROR: Skipping this!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }

    return board_configs;
}
