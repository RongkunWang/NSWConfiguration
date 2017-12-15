#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(ptree vmmconfig): FEConfig(vmmconfig) {
    m_bitset = codec.buildConfig(m_config);
    std::cout << m_bitset << std::endl;
    std::cout << nsw::bitsetToHexString(m_bitset) << std::endl;
}

std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8> nsw::VMMConfig::getByteArray() {
    // TODO(cyildiz): implement? or ditch...
    return std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8>();
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() {
    auto bitstr = m_bitset.to_string();
    std::vector<uint8_t> vec;
    std::string substr;
    uint8_t byte;
    // Go 8 bit at a time and convert it to hex
    for (size_t pos; pos < nsw::VMMCodec::NBITS_TOTAL; pos=pos+8) {
        substr = bitstr.substr(pos, 8);
        // std::cout << "substr: " << substr << std::endl;
        byte = static_cast<uint8_t> (std::stoi(substr, nullptr, 2));
        vec.push_back(byte);
        // std::cout << std::hex << "0x" << unsigned(byte) << std::endl;
    }
    // std::cout << "Vector size: " << std::dec << vec.size() << std::endl;
    return vec;
}

void nsw::VMMConfig::setRegister(std::string register_name, unsigned value) {
    // TODO(cyildiz): implement
}

void nsw::VMMConfig::setAllChannelRegisters(std::string register_name, unsigned value) {
    // TODO(cyildiz): implement
}

void nsw::VMMConfig::setChannelRegister(size_t channel, std::string register_name, unsigned value) {
    // TODO(cyildiz): implement
}
