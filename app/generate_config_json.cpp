
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
    auto tds = TDS_REGISTERS;

    auto all = {roc_digital, roc_analog, tds};
    for (auto i2cmaster : all) {
        bool first1 = true;
        for (auto address : i2cmaster) {
            auto name = address.first;
            if (name.find("READONLY") != std::string::npos) {
                continue;
            }

            if (first1) {
                first1 = false;
            } else {
                std::cout << "},\n";
            }
            std::cout << "\"" << name << "\": {" << std::endl;
            auto registers = address.second;
            bool first2 = true;
            for (auto reg : registers) {
                auto regname = reg.first;
                if (regname == "NOT_USED") {
                    continue;
                }
                if (first2) {
                    first2 = false;
                } else {
                    std::cout << ",\n";
                }
                std::cout << "    \"" << regname <<  "\": 0";
            }
            std::cout << "\n";
        }
        std::cout << "}" << std::endl;
        std::cout << "-------" << std::endl;
    }
}
