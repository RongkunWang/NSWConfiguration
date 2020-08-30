#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(const ptree& vmmconfig): m_config(vmmconfig) {
    m_bitstring = codec.buildConfig(m_config);
    ERS_DEBUG(5, "VMM Bitstream: " << m_bitstring);
    ERS_DEBUG(3, "VMM Bytestream(hex): " << nsw::bitstringToHexString(m_bitstring));
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() const {
    return nsw::stringToByteVector(m_bitstring);
}

unsigned nsw::VMMConfig::getGlobalRegister(const std::string& register_name) const {
    if (!codec.globalRegisterExists(register_name)) {
        std::string temp = register_name;
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return m_config.get<unsigned>(register_name);
}

unsigned nsw::VMMConfig::getChannelRegisterOneChannel(const std::string& register_name, size_t channel) const {
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

std::vector<unsigned> nsw::VMMConfig::getChannelRegisterAllChannels(std::string register_name) const {
    auto channelreg = codec.buildChannelRegisterMap(m_config);
    if (!codec.channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return channelreg[register_name];
}

void nsw::VMMConfig::setGlobalRegister(const std::string& register_name, unsigned value) {
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
        nsw::VmmConfigIssue issue(ERS_HERE, e.what());
        ers::error(issue);
        throw issue;
    }
}

void nsw::VMMConfig::setChannelRegisterAllChannels(const std::string& register_name, unsigned value) {
    m_config.erase(register_name);
    m_config.put(register_name, value);
    m_bitstring = codec.buildConfig(m_config);
}

void nsw::VMMConfig::setChannelRegisterOneChannel(const std::string& register_name, unsigned value, size_t channel) {
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

    auto channelreg = codec.buildChannelRegisterMap(m_config);
    channelreg[register_name][channel] = value;

    ptree temp = nsw::buildPtreeFromVector(channelreg[register_name]);

    // Delete the old node, and write the new one
    m_config.erase(register_name);
    m_config.add_child(register_name, temp);

    m_bitstring = codec.buildConfig(m_config);
}

void nsw::VMMConfig::setTestPulseDAC(size_t param) {
    //
    // param (tpdac): 0-1023
    //
    this->setGlobalRegister("sdp_dac", param);
}

void nsw::VMMConfig::setGlobalThreshold(size_t param) {
    //
    // param (thdac): 0-1023
    //
    this->setGlobalRegister("sdt_dac", param);
}

void nsw::VMMConfig::setMonitorOutput(size_t channel_id, size_t param) {
    //
    //
    // param = 0: common monitor mode
    // param = 1: channel monitor mode
    // In common monitor mode,
    //     channel_id = 1: test pulse DAC
    //     channel_id = 2: threshold DAC
    //     channel_id = 3: bandgap reference
    //     channel_id = 4: temperature
    //
    this->setGlobalRegister("scmx", param);
    this->setGlobalRegister("sm",   channel_id);
}

void nsw::VMMConfig::setChannelMOMode(size_t channel_id, size_t param) {
    //
    // param = 0: channel analog output
    // param = 1: channel threshold
    //
    this->setChannelRegisterOneChannel("channel_smx", param, channel_id);
}

void nsw::VMMConfig::setChannelTrimmer(size_t channel_id, size_t param) {
    //
    // param (trim): 0-31
    //
    this->setChannelRegisterOneChannel("channel_sd", param, channel_id);
}
