// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

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

    auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    nsw::VMMConfig vmm0(vmmconfig0);
    auto vmmconfig7 = reader1.readConfig("A01.VMM_L01_M01_07");
    nsw::VMMConfig vmm7(vmmconfig7);

    auto vec = vmm0.getByteVector();  /// Create a vector of bytes
    for (auto el : vec) {
        std::cout << "0x" << std::hex << unsigned(el) << ", ";
    }
    std::cout << std::dec << std::endl;;

    nsw::ConfigSender cs;

    // Send all ROC config
    // cs.sendRocConfig(roc0);

    // Seqence of actions to send ROC config
    auto opc_ip = roc0.getOpcServerIp();
    auto sca_roc_address = roc0.getAddress();
    size_t size = 1;
    uint8_t data[] = {0x19};

    std::string sca_address = "SCA on Felix (elink 0x80)";
    std::string sca_roc_address_digital = sca_address + "mmfe8RocPllCoreDigital";

    // 1. Reset all logics
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 0);

    // 2. set rocSResetN to 1
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocSResetN", 1);

    // sleep(2);

    // 3. Initialize registers:
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllVMM0reg70", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllVMM1reg86", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".ePllTDCreg102", data, size);
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".register112",  data, size);

    // sleep(2);

    // 4.
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocPllResetN", 1);
    // 5.
    cs.sendGPIO(opc_ip, sca_address + ".gpio.rocCoreResetN", 1);

    // sleep(1);

    cs.sendI2cRaw(opc_ip, sca_roc_address + ".register112",  data, size);

    // sleep(1);

    // std::cout << "vmm_config for A01.VMM_L01_M01_00\n";
    // write_json(std::cout, vmmconfig0);
    // std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;

    // Inverse VMM enable to get VMM into config mode
    data[0] = {static_cast<uint8_t>(0xff)};
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".vmmEnaInvReg122",  data, size);

    std::vector<uint8_t> vmmConfigurationData {  // is SOP 0x00
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x34, 0x84, 0xc0, 0x10, 0x22, 0x20
            // last bit is sp (polarity) and it is the last bit to be written to VMM
    };
    // sleep(4);


    cs.sendSpi(opc_ip, "SCA on Felix (elink 0x80).spi.vmm0", vmmConfigurationData);
    cs.sendSpi(opc_ip, "SCA on Felix (elink 0x80).spi.vmm1", vmmConfigurationData);

    // cs.sendVmmConfig(vmm0);
    // cs.sendVmmConfig(vmm7);

    // sleep(4);

    // Set back the register
    data[0] = {static_cast<uint8_t>(0x0)};
    cs.sendI2cRaw(opc_ip, sca_roc_address + ".vmmEnaInvReg122",  data, size);

    // Change ROC delay and sROC enable parameters (one of: 0x0f, 0x1f, 0x2f, 0x3f)
    // data[0] = 0xf;
    // cs.sendI2cRaw(opc_ip, sca_roc_address_digital + ".sRocEnable", data, size);

    /* 
    // Toggle TpInv to create pulses
    for (int i = 0; i < 10000000; i++) {
        data[0] = 0xff;
        cs.sendI2cRaw(opc_ip, sca_roc_address + ".vmmEnaTpReg124", data, size);
        // sleep(1);

        data[0] = 0x0;
        cs.sendI2cRaw(opc_ip, sca_roc_address + ".vmmEnaTpReg124", data, size);
        // sleep(1);
    }
    */

    // Read adcs:
    // nsw::OpcClient client(opc_ip);
    // std::string adc_address = sca_address + ".ai.vmmPdo1";
    // client.readAnalogOutput(adc_address);

    return 0;
}
