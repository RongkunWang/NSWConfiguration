
// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

int main(int argc, const char *argv[]) {
    auto roc_digital = ROC_DIGITAL_REGISTERS;
    auto roc_analog = ROC_ANALOG_REGISTERS;

    auto all = {roc_digital, roc_analog};
    for (auto i2cmaster : all) {
        for (auto address : i2cmaster) {
            auto name = address.first;
            std::cout << "\"" << name << "\": {" << std::endl;
            auto registers = address.second;
            for (auto reg : registers) {
                auto regname = reg.first;
                std::cout << "    \"" << regname <<  "\": 0," << std::endl;
            }
            std::cout << "}," << std::endl;
        }
        std::cout << "-------" << std::endl;
    }
}
