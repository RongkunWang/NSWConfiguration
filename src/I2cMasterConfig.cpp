#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <numeric>

#include "ers/ers.h"

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
       ERS_DEBUG(3, address << " -> total size: " << total_size);
        size_t pos = 0;
        for (auto i : register_sizes) {
            register_position[i.first] = pos;
            register_size[i.first] = i.second;
            ERS_DEBUG(3, "register: " << i.first << ", size : " << i.second << ", pos: " << pos);
            pos = pos + i.second;
            /* code */
        }
        ERS_DEBUG(3, "");
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

        ptree child;
        try {
            child = config.get_child(address);
        } catch (const boost::property_tree::ptree_bad_path& e) {
            std::string temp = e.what();
            nsw::MissingI2cAddress issue(ERS_HERE, temp.c_str());
            ers::error(issue);
            throw issue;
        }

        std::string tempstr;
        ERS_DEBUG(4, address);
        for (auto rs : register_sizes) {
            auto register_name = rs.first;
            auto size = rs.second;

            unsigned value;
            try {
                value = child.get<unsigned>(register_name);
            } catch (const boost::property_tree::ptree_bad_path& e) {
                std::string temp = address + ": " + e.what();
                nsw::MissingI2cRegister issue(ERS_HERE, temp.c_str());
                ers::error(issue);
                throw issue;
            }
            nsw::checkOverflow(size, value, register_name);
            ERS_DEBUG(5, " -- " << register_name << " -> " << value);

            // TODO(cyildiz): Large enough to take any register
            std::bitset<32> bs(value);
            auto stringbs = bs.to_string();
            stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
            ERS_DEBUG(6, " --- substr:" << stringbs);
            tempstr += stringbs;
        }
        ERS_DEBUG(4, " - bitstream : " << tempstr);
        bitstreams[address] = tempstr;
    }
    return bitstreams;
}

void nsw::I2cMasterConfig::dump() {
    ERS_LOG("Dumping Config for: " << m_name);
    for (auto ab : m_address_bitstream) {
        auto address = ab.first;
        auto bitstream = ab.second;
        std::cout << address << " : " << bitstream << std::endl;
    }
}

uint32_t nsw::I2cMasterConfig::getRegisterValue(std::string address, std::string register_name) {
    if (m_codec.m_addr_reg_pos.find(address) == m_codec.m_addr_reg_pos.end()) {
        std::string temp = address;
        nsw::NoSuchI2cAddress issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    if (m_codec.m_addr_reg_pos[address].find(register_name) == m_codec.m_addr_reg_pos[address].end()) {
        std::string temp = address;
        nsw::NoSuchI2cRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    auto reg_pos = m_codec.m_addr_reg_pos[address][register_name];
    auto reg_size = m_codec.m_addr_reg_size[address][register_name];

    // Find corresponding string of bits from the bitstream and convert it to long
    auto tmp = m_address_bitstream[address].substr(reg_pos, reg_size);
    return std::stoul(tmp, nullptr, 2);
}

void nsw::I2cMasterConfig::setRegisterValue(std::string address, std::string register_name, uint32_t value) {
    if (m_codec.m_addr_reg_pos.find(address) == m_codec.m_addr_reg_pos.end()) {
        std::string temp = address;
        nsw::NoSuchI2cAddress issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    if (m_codec.m_addr_reg_pos[address].find(register_name) == m_codec.m_addr_reg_pos[address].end()) {
        std::string temp = address;
        nsw::NoSuchI2cRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    auto reg_pos = m_codec.m_addr_reg_pos[address][register_name];
    auto reg_size = m_codec.m_addr_reg_size[address][register_name];

    nsw::checkOverflow(reg_size, value, register_name);

    auto bitstring = nsw::bitString(value, reg_size);

    // Replace corresponding substring of bitstream with new bitstring
    m_address_bitstream[address].replace(reg_pos, reg_size, bitstring);
}
