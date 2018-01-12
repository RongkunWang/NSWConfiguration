// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

int main(int argc, const char *argv[]) {
    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    nsw::ConfigReader reader1("json://" + base_folder + "dummy_config.json");
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    nsw::ConfigReader reader2("xml://" + base_folder + "dummy_config.xml");
    auto config2 = reader2.readConfig();
    // write_xml(std::cout, config2, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
    // write_json(std::cout, config2); // Doesn't work!

    // nsw::ConfigReader reader3("oksconfig:" + base_folder + "dummy_config.xml");
    // auto config3 = reader3.readConfig();
    // write_json(std::cout, config);

    // nsw::ConfigReader reader4("oracle:hostname");
    // auto config = reader4.readConfig();
    // write_json(std::cout, config);

    auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    std::cout << "vmm_config for A01.VMM_L01_M01_00\n";
    write_json(std::cout, vmmconfig0);
    // nsw::VMMCodec& vmmcodec0 = nsw::VMMCodec::Instance();
    // auto bs = vmmcodec0.buildConfig(vmmconfig0);
    // td::cout << bs << std::endl;

    auto vmmconfig1 = reader1.readConfig("A01.VMM_L01_M01_01");
    std::cout << "vmm_config for A01.VMM_L01_M01_01\n";
    write_json(std::cout, vmmconfig1);
    // nsw::VMMCodec& vmmcodec = nsw::VMMCodec::Instance();
    // bs = vmmcodec.buildConfig(vmmconfig1);
    // std::cout << bs << std::endl;

    std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;
    std::cout << "vmm1 sca address: " << vmmconfig1.get<std::string>("OpcServerIp")  << std::endl;


    nsw::VMMConfig vmm0(vmmconfig0);
    nsw::VMMConfig vmm1(vmmconfig1);
    // vmm0.set_register("sbmx",1);
    // vmm0.set_all_channel_register("sd",4);  // Set all sd to 4
    // vmm0.set_channel_register("sd",15,4);  // Set sd of 15th channel to 4

    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    write_json(std::cout, rocconfig0);
    nsw::ROCConfig roc0(rocconfig0);

    nsw::ConfigSender cs;

    cs.sendVmmConfig(vmm0);
    cs.sendRocConfig(roc0);

    // cs.sendVmmConfig(vmm1);

    return 0;
}
