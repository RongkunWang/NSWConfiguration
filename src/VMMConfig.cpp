#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(ptree vmmconfig): FEConfig(vmmconfig) {
    m_bitset = codec.buildConfig(m_config);
    ERS_DEBUG(5, "VMM Bitset: " << m_bitset);
    ERS_DEBUG(3, "VMM Bitset(hex): " << nsw::bitsetToHexString(m_bitset));
}

std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8> nsw::VMMConfig::getByteArray() {
    // TODO(cyildiz): implement? or ditch...
    return std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8>();
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() const {
    auto bitstr = m_bitset.to_string();
    return nsw::stringToByteVector(bitstr);
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
