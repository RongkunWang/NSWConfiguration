// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/VMMCodec.h"

int main(int argc, const char *argv[]) {
    ConfigReader reader1("json:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.json");
    auto config1 = reader1.read_config();
    write_json(std::cout, config1);
    //write_xml(std::cout, config1);

    ConfigReader reader2("xml:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.xml");
    auto config2 = reader2.read_config();
    //write_xml(std::cout, config2, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));  // output ptree content
    //write_json(std::cout, config2); // Doesn't work!

    ConfigReader reader3("oksconfig:/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.xml");
    auto config3 = reader3.read_config();
    // write_json(std::cout, config);

    ConfigReader reader4("oracle:hostname");
    auto config = reader4.read_config();
    // write_json(std::cout, config);

    auto vmm2 = reader2.read_config("A01.VMM_L01_M01_01");

    auto vmm0 = reader1.read_config("A01.VMM_L01_M01_01");
    write_json(std::cout, vmm0);

    nsw::VMMCodec vmmcodec;
    vmmcodec.build_global_config0(vmm0);
    vmmcodec.build_global_config1(vmm0);
    vmmcodec.build_channel_config(vmm0);

    return 0;
}
