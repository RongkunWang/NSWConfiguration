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

    // ROC Config
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);
    // roc0.dump();

    nsw::ConfigSender cs;

    auto opc_ip = roc0.getOpcServerIp();
    auto sca_roc_address = roc0.getAddress();
    size_t size = 1;
    uint8_t data[] = {0x0};

    std::string sca_address = "SCA on Felix (elink 0x80)";
    std::string sca_roc_address_digital = sca_address + ".mmfe8RocCoreDigital";

    // Go over the TTC Start Bits in sRocEnable register and set them to 4 possible values (0,1,2,3)
    for (auto bits : {0, 1, 2, 3}) {
        data[0] = (bits << 4) | 0xf;
        cs.sendI2cRaw(opc_ip, sca_roc_address_digital + ".sRocEnable", data, size);
        sleep(2);
    }

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


    return 0;
}
