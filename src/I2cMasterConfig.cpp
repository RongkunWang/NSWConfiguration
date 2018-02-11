#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <numeric>

#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/Types.h"

nsw::I2cMasterCodec::I2cMasterCodec(const i2c::AddressRegisterMap & ar_map): m_addr_reg(ar_map) {
    calculateSizesAndPositions();
}

void nsw::I2cMasterCodec::calculateSizesAndPositions() {
    for (auto e : m_addr_reg) {
        auto address = e.first;
        auto register_sizes = e.second;

        i2c::AddressSizeMap register_position;
        i2c::AddressSizeMap register_size;

        // Total size of registers, by summing sizes of individual registers
        size_t total_size = std::accumulate(register_sizes.begin(), register_sizes.end(), 0,
                                            [](size_t sum, i2c::RegisterSizePair & p) {
                                            return sum + p.second;  });
        std::cout << address << " -> total size: " << total_size << std::endl;
        size_t pos = 0;
        for (auto i : register_sizes) {
            register_position[i.first] = pos;
            register_size[i.first] = i.second;
            pos = pos + i.second;
            std::cout << "register: " << i.first << ", size : " << i.second << ", pos: " << pos << "\n";
            /* code */
        }
        std::cout << "\n";
        m_addr_size[address] = total_size;
        m_addr_reg_pos[address] = register_position;
        m_addr_reg_size[address] = register_size;
    }
}

i2c::AddressBitstreamMap nsw::I2cMasterCodec::buildConfig(ptree config) {
    i2c::AddressBitstreamMap bitstreams;
    for (auto e : m_addr_reg) {
        auto address = e.first;
        auto register_sizes = e.second;

        auto child = config.get_child(address);

        std::string tempstr;
        for (auto rs : register_sizes) {
            auto register_name = rs.first;
            auto size = rs.second;

            auto value = child.get<unsigned>(register_name);
            nsw::checkOverflow(size, value, register_name);
            std::cout << register_name << " -> " << value << std::endl;

            // TODO(cyildiz): Large enough to take any register
            std::bitset<32> bs(value);
            auto stringbs = bs.to_string();
            stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
            // std::cout << "substr:" << stringbs << std::endl;
            tempstr += stringbs;
        }
        // std::cout << tempstr << std::endl;
        bitstreams[address] = tempstr;
    }
    return bitstreams;
}

void nsw::I2cMasterConfig::dump() {
    std::cout << "Dumping Config for: " << m_name << std::endl;
    for (auto ab : m_address_bitstream) {
        auto address = ab.first;
        auto bitstream = ab.second;
        std::cout << address << " : " << bitstream << std::endl;
    }
}

uint32_t nsw::I2cMasterConfig::getRegisterValue(std::string address, std::string register_name) {
    // TODO(cyildiz): Exception handling if the elements doesn't exist
    auto reg_pos = m_codec.m_addr_reg_pos[address][register_name];
    auto reg_size = m_codec.m_addr_reg_size[address][register_name];

    // Find corresponding string of bits from the bitstream and convert it to long
    auto tmp = m_address_bitstream[address].substr(reg_pos, reg_size);
    return std::stoul(tmp, nullptr, 2);
}

void nsw::I2cMasterConfig::setRegisterValue(std::string address, std::string register_name, uint32_t value) {
    // TODO(cyildiz): Exception handling if the elements doesn't exist
    auto reg_pos = m_codec.m_addr_reg_pos[address][register_name];
    auto reg_size = m_codec.m_addr_reg_size[address][register_name];

    nsw::checkOverflow(reg_size, value, register_name);

    auto bitstring = nsw::bitString(value, reg_size);

    // Replace corresponding substring of bitstream with new bitstring
    m_address_bitstream[address].replace(reg_pos, reg_size, bitstring);
}
