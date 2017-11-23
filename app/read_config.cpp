// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/VMMCodec.h"

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

    ConfigReader reader3("oksconfig:" + base_folder + "dummy_config.xml");
    auto config3 = reader3.readConfig();
    // write_json(std::cout, config);

    ConfigReader reader4("oracle:hostname");
    auto config = reader4.readConfig();
    // write_json(std::cout, config);

    auto vmm2 = reader2.readConfig("A01.VMM_L01_M01_01");

    auto vmm0 = reader1.readConfig("A01.VMM_L01_M01_01");
    write_json(std::cout, vmm0);

    nsw::VMMCodec vmmcodec;
    auto bs = vmmcodec.buildConfig(vmm0);
    std::cout << bs << std::endl;

    return 0;
}
