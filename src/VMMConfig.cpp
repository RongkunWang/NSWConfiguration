#include <string>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(ptree vmmconfig): m_config(vmmconfig) {
    // m_opcserver_ip = ...
    // m_vmm_address = m_config.get<std::string>("OpcAddress"):
    m_bitset = codec.buildConfig(m_config);
}

std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8> nsw::VMMConfig::getByteArray() {
    return std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8>();
}

void nsw::VMMConfig::setRegister(std::string register_name, unsigned value) {
}

void nsw::VMMConfig::setAllChannelRegisters(std::string register_name, unsigned value) {
}

void nsw::VMMConfig::setChannelRegister(size_t channel, std::string register_name, unsigned value) {
}
