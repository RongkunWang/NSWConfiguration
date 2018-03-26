#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(ptree vmmconfig): FEConfig(vmmconfig) {
    m_bitstring = codec.buildConfig(m_config);
    ERS_DEBUG(5, "VMM Bitstream: " << m_bitstring);
    ERS_DEBUG(3, "VMM Bytestream(hex): " << nsw::bitstringToHexString(m_bitstring));
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() const {
    return nsw::stringToByteVector(m_bitstring);
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
