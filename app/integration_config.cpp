// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

int main(int argc, const char *argv[]) {
    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    nsw::ConfigReader reader1("json://" + base_folder + "integration_config.json");
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);


    // ROC Config
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    std::cout << "ROC Config:"  << std::endl;
    write_json(std::cout, rocconfig0);
    nsw::ROCConfig roc0(rocconfig0);
    roc0.dump();

    nsw::ConfigSender cs;

    cs.sendRocConfig(roc0);

    /*
    auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    std::cout << "vmm_config for A01.VMM_L01_M01_00\n";
    write_json(std::cout, vmmconfig0);

    std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;

    nsw::VMMConfig vmm0(vmmconfig0);

    cs.sendVmmConfig(vmm0);
    */

    return 0;
}
