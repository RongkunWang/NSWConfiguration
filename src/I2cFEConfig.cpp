#include <iostream>

#include "NSWConfiguration/I2cFEConfig.h"

void nsw::I2cFEConfig::dump() {
    std::cout << "Dumping Config for: " << m_address << std::endl;
    for (auto ab : m_address_bitstream) {
        auto address = ab.first;
        auto bitstream = ab.second;
        std::cout << address << " : " << bitstream << std::endl;
    }
}
