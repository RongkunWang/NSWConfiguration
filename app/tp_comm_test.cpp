// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
// #include "NSWConfiguration/TPConfig.h"
#include "NSWConfiguration/OpcClient.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;


int main(int argc, const char *argv[]) 
{

    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program is for sending/receiving messages from the SCX on the TP.";

    std::string opc_ip;
    bool readMode;
    bool writeMode;
    std::string slaveAddr;
    std::string regAddr;
    std::string message;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("read,R", po::bool_switch(&readMode)->default_value(false),
            "read value at address(Default: False)")
        ("write,W", po::bool_switch(&writeMode)->default_value(false),
            "write value at address(Default: False)")
        ("message,m", po::value<std::string>(&message)->default_value("0XC0FFEEEE"),
            "message to write in hex (Default: 0xC0FFEEEE)")
        ("slaveAddr,s", po::value<std::string>(&slaveAddr)->default_value("NSW_TrigProc_STGC.I2C_0.bus0"),
            "slave bus to write to(Default: NSW_TrigProc_STGC.I2C_0.bus0)")
        ("regAddr,r", po::value<std::string>(&regAddr)->default_value("0x000"),
            "register to read from/write to(Default: 0x2AD)")
        ("opc_ip,o", po::value<std::string>(&opc_ip)->default_value("pcatlnswfelix01.cern.ch:48020"),
            "hostname for OPC server");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigSender cs; // in principle the config sender is all that is needed for now

    // auto addcconfig0 = reader1.readConfig("A01.ROC_L01_M01"); // THIS NEEDS CHANGE!!! fine for now
    // nsw::TPConfig tp(addcconfig0);
    // tp.dump();

    // Seqence of actions to send ADDC config
    // auto opc_ip = tp.getOpcServerIp();
    // auto sca_tp_address = tp.getAddress();

    int test = 0;

    std::vector<uint8_t> data;
    std::vector<uint8_t> outdata;

    std::vector<uint8_t> regAddrVec;

    regAddrVec = nsw::hexStringToByteVector(regAddr,4,true);
    std::cout << "... Register address array: ";
    for (uint i=0; i<regAddrVec.size(); i++) {
        std::cout << std::hex << unsigned(regAddrVec[i]) << " ";
    }
    std::cout << std::endl;

    if (readMode) {
        if(test==0){
            std::cout << "... Testing the readout of a register via I2c..." << std::endl;
            outdata = cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);
            // outdata = cs.readI2cAtAddress("pcatlnswfelix01.cern.ch:48020", "NSW_TrigProc_STGC.I2C_0.bus0", regAddrVec.data(), regAddrVec.size(), 4);
            for (uint i=0; i<outdata.size(); i++) {
                std::cout << std::hex << unsigned(outdata[i]) << std::endl;
            }
        }
    }

    if (writeMode){
        // Example
        // data = {0x00, 0x00, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xff}; //8
        //             [0] first byte of address
        //             [1] second byte of address
        //             [4,5,6,7] last..first bytes of message
        // Already have the first four bytes of vector stored as regAddrVec.
        // Just need to encode the message.
        data = nsw::hexStringToByteVector(message,4,true);
        std::cout << "... Message to write: " << message << std::endl;
        std::cout << "... ... in a byte vector: ";
        for (uint i=0; i<data.size(); i++) {
            std::cout << std::hex << unsigned(data[i]) << " ";
        }
        std::cout << std::endl;

        std::vector<uint8_t> entirePayload(regAddrVec);
        entirePayload.insert(entirePayload.end(), data.begin(), data.end() );

        if(test==0){
            cs.sendI2cRaw(opc_ip, slaveAddr, entirePayload.data(), entirePayload.size() );
            if (readMode){
                std::cout << "... Reading data back from register: " << std::endl;
                outdata = cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);
                for (uint i=0; i<outdata.size(); i++) {
                    std::cout << std::hex << unsigned(outdata[i]) << std::endl;
                }
            }
        }

    }


    // tp = nsw::TPConfig();

    std::cout << "... Done with TP Comm test" << std::endl;


    return 0;

}
