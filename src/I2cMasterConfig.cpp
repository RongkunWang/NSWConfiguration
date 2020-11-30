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
        std::string register_name;
        for (auto i : register_sizes) {
            // NOT_USED is a special register name. It means there are some bits not used in
            // the configuration but they affect the position of other configuration bits.
            // By default they will be all set to 0
            if (i.first == "NOT_USED") {
                register_name = "NOT_USED";
                ERS_DEBUG(3, "Not used bits : " << register_name << ", size: " << i.second << ", pos: " << pos);
            } else {
                register_name = i.first;
                ERS_DEBUG(3, "register: " << register_name << ", size : " << i.second << ", pos: " << pos);
                register_position[register_name] = pos;
                register_size[register_name] = i.second;
            }
            pos = pos + i.second;
        }
        ERS_DEBUG(3, "");
        m_addr_size[address] = total_size;
        m_addr_reg_pos[address] = register_position;
        m_addr_reg_size[address] = register_size;
    }
}

i2c::AddressBitstreamMap nsw::I2cMasterCodec::buildConfig(const ptree& config) const {
    i2c::AddressBitstreamMap bitstreams;
    for (auto e : m_addr_reg) {
        auto address = e.first;
        if (address.find("READONLY") != std::string::npos) {
            continue;  // Ignore readonly registers, as they don't have a value in configuration
        }
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
            // Fill not used bits with 0
            if (register_name == "NOT_USED") {
                value = 0;
            } else {
                try {
                    value = child.get<unsigned>(register_name);
                } catch (const boost::property_tree::ptree_bad_path& e) {
                    std::string temp = address + ": " + e.what();
                    nsw::MissingI2cRegister issue(ERS_HERE, temp.c_str());
                    ers::error(issue);
                    throw issue;
                }
                nsw::checkOverflow(size, value, register_name);
            }
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

i2c::AddressBitstreamMap nsw::I2cMasterCodec::buildPartialConfig(const ptree& config) const {
    i2c::AddressBitstreamMap bitstreams;
    std::transform(std::begin(config), std::end(config), std::inserter(bitstreams, std::end(bitstreams)), [&config] (const auto& pair) -> std::pair<std::string, std::string> {
        const std::string address = pair.first;
        const unsigned int value = config.get<unsigned int>(address);
        // FIXME: Size?
        std::bitset<8> bs(value);
        return {address, bs.to_string()};
    });
    return bitstreams;
}

void nsw::I2cMasterConfig::dump() const {
    ERS_LOG("Dumping Config for: " << m_name);
    for (auto ab : m_address_bitstream) {
        auto address = ab.first;
        auto bitstream = ab.second;
        std::cout << address
                  << "\n  bits  : " << bitstream
                  << "\n  bytes : " << bitstringToHexString(bitstream)
                  << std::endl;
    }
}

uint32_t nsw::I2cMasterConfig::getRegisterValue(const std::string& address, const std::string& register_name) const {
    if (m_codec.m_addr_reg_pos.find(address) == m_codec.m_addr_reg_pos.end()) {
        std::string temp = address;
        nsw::NoSuchI2cAddress issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    if (m_codec.m_addr_reg_pos.at(address).find(register_name) == m_codec.m_addr_reg_pos.at(address).end()) {
        std::string temp = address;
        nsw::NoSuchI2cRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    auto reg_pos = m_codec.m_addr_reg_pos.at(address).at(register_name);
    auto reg_size = m_codec.m_addr_reg_size.at(address).at(register_name);

    // Find corresponding string of bits from the bitstream and convert it to long
    auto tmp = m_address_bitstream.at(address).substr(reg_pos, reg_size);
    return std::stoul(tmp, nullptr, 2);
}

void nsw::I2cMasterConfig::setRegisterValue(const std::string& address, const std::string& register_name,
    uint32_t value) {
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

void nsw::I2cMasterConfig::decodeVector(const std::string& address, const std::vector<uint8_t>& vec) const {
    if (m_codec.m_addr_reg_pos.find(address) == m_codec.m_addr_reg_pos.end()) {
        nsw::NoSuchI2cAddress issue(ERS_HERE, address.c_str());
        ers::error(issue);
        return;
    }

    if (vec.size() * 8 != getTotalSize(address)) {
        auto vecsize = vec.size()*8;
        auto i2csize = getTotalSize(address);
        nsw::I2cSizeMismatch issue(ERS_HERE, address, vecsize, i2csize);
        ers::error(issue);
        return;
    }

    std::cout << address << std::endl;
    auto bitstring = vectorToBitString(vec);
    for (auto reg : m_codec.m_addr_reg_size.at(address)) {
        auto register_name = reg.first;
        auto reg_size = reg.second;
        auto reg_pos = m_codec.m_addr_reg_pos.at(address).at(register_name);
        auto tmp = bitstring.substr(reg_pos, reg_size);
        std::stringstream name_stream;
        name_stream << std::setw(30) << std::left << register_name;
        std::cout << " - " << name_stream.str()  << " : " <<  std::stoul(tmp, nullptr, 2) << std::endl;
    }
}

size_t nsw::I2cMasterCodec::getTotalSize(const std::string& address) const {
    size_t size{ 0 };
    try {
       size = m_addr_size.at(address);
    } catch (const std::exception& ex) {
        nsw::NoSuchI2cAddress issue(ERS_HERE, address.c_str());
        ers::error(issue);
        throw issue;
    }
    return size;
}

std::vector<std::string> nsw::I2cMasterCodec::getAddresses() const {
    std::vector<std::string> addresses;
    for (auto e : m_addr_reg) {
        addresses.push_back(e.first);
    }
    return addresses;
}
