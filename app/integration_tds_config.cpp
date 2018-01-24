// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

int main(int argc, const char *argv[]) {
    nsw::ConfigSender cs;

    // Seqence of actions to send ROC config
    std::string opc_ip = "pcatlnswfelix01.cern.ch:4841";
    std::string sca_base_address = "SCA on Felix (elink 0x80)";
    std::string sca_tds_address =  sca_base_address + ".TDS";
    std::vector<uint8_t> data;


    // DO SAME ACTIONS for configuring ROC!
    /*
    // GPIOs
    cs.sendGPIO(opc_ip, sca_address + ".gpio.1", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.2", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.3", 0);

    // I2c
    cs.sendI2cRaw(opc_ip, sca_tds_address + ".smtg", data, size);
    cs.sendI2cRaw(opc_ip, sca_tds_address + ".smtg", data, size);
    cs.sendI2cRaw(opc_ip, sca_tds_address + ".smtg", data, size);
    cs.sendI2cRaw(opc_ip, sca_tds_address + ".smtg",  data, size);

    data = {0x0,0x0,0x0};
    cs.sendI2cRaw(opc_ip, sca_tds_address + ".smtg",  data.data(), data.size() );
    */

    // auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    // std::cout << "vmm_config for A01.VMM_L01_M01_00\n";
    // write_json(std::cout, vmmconfig0);
    // std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;

    // nsw::VMMConfig vmm0(vmmconfig0);

    // cs.sendVmmConfig(vmm0);


    return 0;
}
