// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"

int main(int argc, const char *argv[]) {
    ConfigReader reader1("json:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.json");
    auto config1 = reader1.read_config();
    write_json(std::cout, config1);

    ConfigReader reader2("xml:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.xml");
    auto config2 = reader2.read_config();
    write_xml(std::cout, config2, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));  // output ptree content

    ConfigReader reader3("oksconfig:/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.xml");
    auto config3 = reader3.read_config();
    // write_json(std::cout, config);

    return 0;
}
