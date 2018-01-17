#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <numeric>

#include "NSWConfiguration/I2cFECodec.h"
#include "NSWConfiguration/Utility.h"

void nsw::I2cFECodec::calculateTotalSizes() {
    for (auto e : m_addr_reg) {
        auto address = e.first;
        auto register_sizes = e.second;

        // Total size of registers, by summing sizes of individual registers
        size_t total_size = std::accumulate(register_sizes.begin(), register_sizes.end(), 0,
                                            [](size_t sum, i2c::RegisterAndSize & p) {
                                            return sum + p.second;  });
        std::cout << address << " -> total size: " << total_size << std::endl;
        for (auto i : register_sizes) {
            std::cout << "register: " << i.first << " : " << i.second << "\n";
            /* code */
        }
        std::cout << "\n";
        m_addr_size[address] = total_size;
    }
}

nsw::i2c::AddressBitstreamMap nsw::I2cFECodec::buildConfig(ptree config) {
    i2c::AddressBitstreamMap bitstreams;
    for (auto e : m_addr_reg) {
        auto address = e.first;
        auto register_sizes = e.second;

        auto child = config.get_child(address);

        std::string tempstr;
        for (auto rs : register_sizes) {
            auto register_name = rs.first;
            auto size = rs.second;

            auto u = child.get<unsigned>(register_name);
            // checkOverflow
            std::cout << register_name << " -> " << u << std::endl;

            // TODO(cyildiz): Large enough to take any register
            std::bitset<32> bs(u);
            auto stringbs = bs.to_string();
            stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
            // std::cout << stringbs << std::endl;
            tempstr += stringbs;
        }
        std::cout << tempstr << std::endl;
        bitstreams[address] = tempstr;
    }
    return bitstreams;
}
