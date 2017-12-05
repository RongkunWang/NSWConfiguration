#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(ptree vmmconfig): m_config(vmmconfig) {
    m_opcserver_ip = m_config.get<std::string>("OpcServerIp");
    m_vmm_address = m_config.get<std::string>("OpcNodeId");
    m_bitset = codec.buildConfig(m_config);
}

std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8> nsw::VMMConfig::getByteArray() {
    // TODO(cyildiz): implement
    return std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8>();
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() {
    // TODO(cyildiz): implement
    return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
