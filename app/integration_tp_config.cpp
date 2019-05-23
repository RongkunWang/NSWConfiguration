// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/TPConfig.h"
// #include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/OpcClient.h"

int main(int argc, const char *argv[]) 
{

    // not sure if/what is needed for ADDC
    // std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    // nsw::ConfigReader reader1("json://" + base_folder + "integration_config.json"); //here is OPC address and SCA address that can be useful
    // auto config1 = reader1.readConfig();
    // write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    // now we start with (preliminary) ADDC part

    nsw::ConfigSender cs; // in principle the config sender is all that is needed for now

    // auto addcconfig0 = reader1.readConfig("A01.ROC_L01_M01"); // THIS NEEDS CHANGE!!! fine for now
    // nsw::TPConfig tp(addcconfig0);
    // tp.dump();

    // Seqence of actions to send ADDC config
    // auto opc_ip = tp.getOpcServerIp();
    // auto sca_tp_address = tp.getAddress();

    std::string opc_ip = "pcatlnswfelix01.cern.ch:48020";
    std::string sca_address = "NSW_TrigProc_STGC";
    // std::string sca_address = sca_tp_address;
    // this might need to be: simple-netio://direct/pcatlnswfelix01.cern.ch/12340/12350/88

    std::cout << "Hello World!" << std::endl;
    std::vector<uint8_t> data;
    data = {0x00, 0x00, 0x03, 0x00}; //4
    // data = {0x00, 0x00, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xff}; //8

    // Example
    // data = {0x00, 0x00, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xff}; //8
    //             [0] first byte of address
    //             [1] second byte of address
    //             [4,5,6,7] last..first bytes of message


    data = {0x00, 0x03, 0x00, 0x00, 0x55, 0xa5, 0xad, 0xba}; //8
    // std::cout << "Trying to write zeroes to slave 0 on master 0" << std::endl;
    cs.sendI2cRaw(opc_ip, sca_address + ".I2C_0.bus0", data.data(), data.size() );

    data = {0xad, 0x02, 0x00, 0x00, 0xed, 0xfe, 0xce, 0xfa}; //8
    cs.sendI2cRaw(opc_ip, sca_address + ".I2C_0.bus0", data.data(), data.size() );

    data = {0x00, 0x03, 0x00, 0x00}; //4

    // // implement some reading of a register via I2C just to test
    std::vector<uint8_t> outdata = cs.readI2cAtAddress(opc_ip, sca_address + ".I2C_0.bus0", data.data(), data.size(), 4);
    std::cout << "Testing the readout of a register via I2c..." << std::endl;
    for (uint i=0; i<outdata.size(); i++) {
        std::cout << std::hex << unsigned(outdata[i]) << std::hex  << std::endl;
    }

    data = {0xad, 0x02, 0x00, 0x00}; //4

    // // implement some reading of a register via I2C just to test
    outdata = cs.readI2cAtAddress(opc_ip, sca_address + ".I2C_0.bus0", data.data(), data.size(), 4);
    std::cout << "Testing the readout of a register via I2c..." << std::endl;
    for (uint i=0; i<outdata.size(); i++) {
        std::cout << std::hex << unsigned(outdata[i]) << std::hex  << std::endl;
    }


    return 0;

}
