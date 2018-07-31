// Sample program to read configuration from db/json

#include <iostream>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/FEBConfig.h"

int main(int argc, const char *argv[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";
    nsw::ConfigReader reader1("json://" + base_folder + "integration_config_20180701.json");
    auto config1 = reader1.readConfig();
    // write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    // nsw::ConfigReader reader2("xml://" + base_folder + "dummy_config.xml");
    // auto config2 = reader2.readConfig();
    // write_xml(std::cout, config2, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
    // write_json(std::cout, config2); // Doesn't work!

    // nsw::ConfigReader reader3("oksconfig:" + base_folder + "dummy_config.xml");
    // auto config3 = reader3.readConfig();
    // write_json(std::cout, config);

    // nsw::ConfigReader reader4("oracle:hostname");
    // auto config = reader4.readConfig();
    // write_json(std::cout, config);

    std::string fe_name = "PFEB-0001";
    auto feb_config_tree = reader1.readConfig(fe_name);
    write_json(std::cout, feb_config_tree);
    nsw::FEBConfig feb(feb_config_tree);

    

    return 0;
}
