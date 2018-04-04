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

unsigned nsw::VMMConfig::getGlobalRegister(std::string register_name) {
    if (!codec.globalRegisterExists(register_name)) {
        std::string temp = register_name;
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return m_config.get<unsigned>(register_name);
}

unsigned nsw::VMMConfig::getChannelRegisterOneChannel(std::string register_name, size_t channel) {
    // TODO(cyildiz): implement
    auto channelreg = codec.buildChannelRegisterMap(m_config);
    if (!codec.channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    } else if (channel >= nsw::VMMCodec::NCHANNELS) {
        nsw::VmmChannelOutOfRange issue(ERS_HERE, channel);
        ers::error(issue);
        throw issue;
    }
    return channelreg[register_name][channel];
}

std::vector<unsigned> nsw::VMMConfig::getChannelRegisterAllChannels(std::string register_name) {
    auto channelreg = codec.buildChannelRegisterMap(m_config);
    if (!codec.channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return channelreg[register_name];
}

void nsw::VMMConfig::setGlobalRegister(std::string register_name, unsigned value) {
    if (!codec.globalRegisterExists(register_name)) {
        std::string temp = register_name;
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    m_config.put(register_name, value);
    try {
        m_bitstring = codec.buildConfig(m_config);
    } catch(std::exception & e) {
        throw e;
    }
}

void nsw::VMMConfig::setChannelRegisterAllChannels(std::string register_name, unsigned value) {
    // TODO(cyildiz): implement
    m_bitstring = codec.buildConfig(m_config);
}

void nsw::VMMConfig::setChannelRegisterOneChannel(std::string register_name, unsigned value, size_t channel) {
    // TODO(cyildiz): implement
    m_bitstring = codec.buildConfig(m_config);
}
