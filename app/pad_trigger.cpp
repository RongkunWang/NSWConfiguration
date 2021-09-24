// Program to set Pad Trigger configuration

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/PadTriggerSCAConfig.h"
#include "NSWConfiguration/Utility.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

double adc2temp(int adc);
bool file_exists(std::string fname);

int main(int argc, const char *argv[]) 
{
    std::string config_files = "/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/";
    std::string config_filename;
    std::string board_name;
    std::string bitstream;
    std::string gpio_name;
    std::string adc_name;
    std::string i2c_reg;
    std::string i2c_val;
    bool do_config;
    bool do_control;
    int samples;
    int val;

    // options
    po::options_description desc(std::string("Pad trigger configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->default_value(config_files+"config_json/191/A12/padTrigger.json"), "Configuration file path")
        ("name,n", po::value<std::string>(&board_name)
         ->default_value("PadTriggerSCA_00"), "Name of desired PT (should contain PadTriggerSCA).")
        ("bitstream,b", po::value<std::string>(&bitstream)
         ->default_value(""), "Bitstream name to write to the FPGA. WARNING: EXPERIMENTAL.")
        ("gpio", po::value<std::string>(&gpio_name)
         ->default_value(""), "GPIO name to read/write (check the xml for valid names).")
        ("adc", po::value<std::string>(&adc_name)
         ->default_value(""), "ADC name to read (check the xml for valid names).")
        ("do_config", po::bool_switch()->
         default_value(false), "Option to send predefined configuration")
        ("do_control", po::bool_switch()->
         default_value(false), "Option to send Pad Trigger control register (built from json)")
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
    do_config  = vm["do_config"] .as<bool>();
    do_control = vm["do_control"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // make objects from json
    auto board_configs = nsw::ConfigReader::makeObjects<nsw::PadTriggerSCAConfig>
        ("json://" + config_filename, "PadTriggerSCA", board_name);
    for (auto & board: board_configs)
        std::cout << "Found "
                  << board.getAddress()
                  << " @ "
                  << board.getOpcServerIp()
                  << std::endl
                  << " with "
                  << board.firmware()
                  << " -> T = "
                  << board.firmware_dateword()
                  << std::endl;

    // the sender
    nsw::ConfigSender cs;

    // fpga bitstream
    if (bitstream != "") {
        if (!file_exists(bitstream))
            throw std::runtime_error("File doesnt exist: " + bitstream);
        for (auto & board: board_configs)
            cs.sendFPGA(board.getOpcServerIp(), board.getAddress(), bitstream);
    }

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

    // control
    if (do_control)
        for (auto & board: board_configs)
            cs.sendPadTriggerSCAControlRegister(board);

    return 0;
}

double adc2temp(int adc) {
    return (0.79-adc/4095.0)*545.454545455 - 40;
}

bool file_exists(std::string fname) {
    std::ifstream fi(fname.c_str());
    return fi.good();
}

