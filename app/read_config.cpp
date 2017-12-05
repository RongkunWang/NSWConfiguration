// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"

int main(int argc, const char *argv[]) {
    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    ConfigReader reader1("json://" + base_folder + "dummy_config.json");
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    ConfigReader reader2("xml://" + base_folder + "dummy_config.xml");
    auto config2 = reader2.readConfig();
    // write_xml(std::cout, config2, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
    // write_json(std::cout, config2); // Doesn't work!

    // ConfigReader reader3("oksconfig:" + base_folder + "dummy_config.xml");
    // auto config3 = reader3.readConfig();
    // write_json(std::cout, config);

    // ConfigReader reader4("oracle:hostname");
    // auto config = reader4.readConfig();
    // write_json(std::cout, config);

    auto vmmconfig0 = reader1.readConfig("A01.VMM_L01_M01_00");
    nsw::VMMCodec& vmmcodec0 = nsw::VMMCodec::Instance();
    // auto bs = vmmcodec0.buildConfig(vmmconfig0);
    // td::cout << bs << std::endl;

    auto vmmconfig1 = reader1.readConfig("A01.VMM_L01_M01_01");
    // write_json(std::cout, vmmconfig1);
    nsw::VMMCodec& vmmcodec = nsw::VMMCodec::Instance();
    // bs = vmmcodec.buildConfig(vmmconfig1);
    // std::cout << bs << std::endl;

    std::cout << "vmm0 sca address: " << vmmconfig0.get<std::string>("OpcServerIp")  << std::endl;
    std::cout << "vmm1 sca address: " << vmmconfig1.get<std::string>("OpcServerIp")  << std::endl;

    nsw::VMMConfig vmm0(vmmconfig0);
    // vmm0.set_register("sbmx",1);
    // vmm0.set_all_channel_register("sd",4);  // Set all sd to 4
    // vmm0.set_channel_register("sd",15,4);  // Set sd of 15th channel to 4
    nsw::ConfigSender cs;
    cs.sendVmmConfig(vmm0); // Get opc address and send config.



    return 0;
}
