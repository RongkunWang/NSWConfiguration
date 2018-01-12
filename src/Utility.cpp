#include <string>
#include <exception>
#include <cmath>

#include "NSWConfiguration/Utility.h"

// template<size_t N1, size_t N2>
// std::bitset<N1 + N2> concatenate(std::bitset<N1> b1, std::bitset<N2> b2);

std::string nsw::reversedBitString(unsigned value, size_t nbits) {
    std::bitset<32> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    std::reverse(str.begin(), str.end());
    return str;
}

std::string nsw::bitString(unsigned value, size_t nbits) {
    std::bitset<32> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    std::reverse(str.begin(), str.end());
    return str;
}

std::string nsw::getElementType(std::string element_name) {
    if (element_name.find("VMM") != std::string::npos) {
        return "VMM";
    } else if (element_name.find("ROC") != std::string::npos) {
        return "ROC";
    } else {
        auto err = "Type not VMM or ROC!. Unknown front end element: " + element_name;
        throw std::runtime_error(err);
    }
}

void nsw::checkOverflow(size_t register_size, unsigned value, std::string register_name) {
    if (std::pow(2, register_size) <= value) {
        std::string err = "Overflow, register: " + register_name + ", max value: "
                           + std::to_string(std::pow(2, register_size)-1)
                           + ", actual value: " + std::to_string(value);
        throw std::runtime_error(err);  // TODO(cyildiz): convert to ers
    }
}

