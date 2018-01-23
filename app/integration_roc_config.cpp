// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/OpcClient.h"

int main(int argc, const char *argv[]) {
    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    nsw::ConfigReader reader1("json://" + base_folder + "integration_config.json");
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    // ROC Config
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    std::cout << "ROC Config:"  << std::endl;
    // write_json(std::cout, rocconfig0);
    nsw::ROCConfig roc0(rocconfig0);
    // roc0.dump();

    nsw::ConfigSender cs;

    // Send all ROC config
    // cs.sendRocConfig(roc0);

    // Seqence of actions to send ROC config
    auto opc_ip = roc0.getOpcServerIp();
    auto sca_roc_address = roc0.getAddress();
    size_t size = 1;
    uint8_t data[] = {0x19};

    std::string sca_address = "SCA on Felix (elink 0x80)";

    // 1. Reset all logics
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 0);

    // 2. set rocSResetN to 1
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 1);

    // 3. Initialize registers:
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllVMM0reg70", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllVMM1reg86", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllTDCreg102", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".register112",  data, size);

    // 4.
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 1);
    // 5.
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 1);


    data[0] = {static_cast<uint8_t>(0xff)};
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".register112",  data, size);

    auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    // std::cout << "vmm_config for A01.VMM_L01_M01_00\n";
    // write_json(std::cout, vmmconfig0);
    // std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;

    nsw::VMMConfig vmm0(vmmconfig0);

    cs.sendVmmConfig(vmm0);

    // Read adcs:
    nsw::OpcClient client(opc_ip);
    std::string adc_address = sca_address + ".";
    client.readAnalogOutput(adc_address);

    return 0;
}
