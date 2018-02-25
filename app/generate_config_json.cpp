
// Sample program to create configuration json

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
        bool first1 = true;
        for (auto address : i2cmaster) {
            if (first1) {
                first1 = false;
            } else {
                std::cout << "},\n";
            }
            auto name = address.first;
            std::cout << "\"" << name << "\": {" << std::endl;
            auto registers = address.second;
            bool first2 = true;
            for (auto reg : registers) {
                if (first2) {
                    first2 = false;
                } else {
                    std::cout << ",\n";
                }
                auto regname = reg.first;
                std::cout << "    \"" << regname <<  "\": 0";
            }
            std::cout << "\n";
        }
        std::cout << "}" << std::endl;
        std::cout << "-------" << std::endl;
    }
}
