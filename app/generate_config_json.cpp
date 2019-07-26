
// Sample program to create configuration json for all i2c devices

#include <iostream>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

int main(int argc, const char *argv[]) {
    auto roc_digital = ROC_DIGITAL_REGISTERS;
    auto roc_analog  = ROC_ANALOG_REGISTERS;
    auto tds         = TDS_REGISTERS;
    auto art_core    = ART_CORE_REGISTERS;
    auto art_pll     = ART_PLL_REGISTERS;
    // If there is a new object in I2cRegisterMappings, add it to the following list

    auto all = {roc_digital, roc_analog, tds, art_core, art_pll};
    for (auto i2cmaster : all) {
        bool first1 = true;
        for (auto address : i2cmaster) {
            auto name = address.first;

            // Don't put READONLY registers in the configuration
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

                // NOT_USED is a special word, that marks unused bits. Ignore it
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
