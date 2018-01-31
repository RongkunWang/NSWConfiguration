#include <utility>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/I2cFEConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/ConfigReader.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

int main() {
    using RegisterAndSize = std::pair<std::string, size_t>;

    // Ordered map of register names and sizes in bits
    using RegisterSizeVector = std::vector<RegisterAndSize>;
    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;

    const AddressRegisterMap & a = ROC_REGISTER_SIZE;
    const AddressRegisterMap & b = ROC_REGISTER_SIZE;

    std::cout << "Location: " << &a << std::endl;
    std::cout << "Location: " << &b << std::endl;

    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/";
    nsw::ConfigReader reader1("json://" + base_folder + "integration_config.json");
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    // ROC Config
    auto rocconfig = reader1.readConfig("A01.ROC_L01_M01");
    write_json(std::cout, rocconfig);

    // auto codec = nsw::I2cFECodec(ROC_REGISTER_SIZE);
    // auto roc01 = nsw::I2cFEConfig(rocconfig, ROC_REGISTER_SIZE);

    auto roc02 = nsw::ROCConfig(rocconfig);
    roc02.dump();

    return 0;
}
