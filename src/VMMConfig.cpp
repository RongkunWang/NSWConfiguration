#include "NSWConfiguration/VMMConfig.h"

#include "NSWConfiguration/Utility.h"

#include <utility>

#include <ers/ers.h>

using boost::property_tree::ptree;

nsw::VMMConfig::VMMConfig(const ptree& vmmconfig): m_config(vmmconfig) {
    m_bitstring = VMMCodec::buildConfig(m_config);
    ERS_DEBUG(5, "VMM Bitstream: " << m_bitstring);
    ERS_DEBUG(3, "VMM Bytestream(hex): " << nsw::bitstringToHexString(m_bitstring));
}

std::vector<uint8_t> nsw::VMMConfig::getByteVector() const {
    return nsw::stringToByteVector(m_bitstring);
}

std::uint32_t nsw::VMMConfig::getGlobalRegister(const std::string& register_name) const {
    if (!VMMCodec::globalRegisterExists(register_name)) {
        std::string temp = register_name;
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return m_config.get<std::uint32_t>(register_name);
}

std::uint32_t nsw::VMMConfig::getChannelRegisterOneChannel(const std::string& register_name, const std::uint32_t channel) const {
    auto channelreg = VMMCodec::buildChannelRegisterMap(m_config);
    if (!VMMCodec::channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    } else if (channel >= nsw::VMMCodec::NCHANNELS) {
        nsw::VmmChannelOutOfRange issue(ERS_HERE, channel);
        ers::error(issue);
        throw issue;
    }
    return channelreg.at(register_name).at(channel);
}

std::array<std::uint32_t, nsw::vmm::NUM_CH_PER_VMM> nsw::VMMConfig::getChannelRegisterAllChannels(const std::string& register_name) const {
    auto channelreg = VMMCodec::buildChannelRegisterMap(m_config);
    if (!VMMCodec::channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    return channelreg.at(register_name);
}

std::uint32_t nsw::VMMConfig::getGlobalThreshold() const {
    return getGlobalRegister("sdt_dac");
}

void nsw::VMMConfig::setGlobalRegister(const std::string& register_name, const std::uint32_t value) {
    if (!VMMCodec::globalRegisterExists(register_name)) {
        std::string temp = register_name;
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    }
    m_config.put(register_name, value);
    try {
        m_bitstring = VMMCodec::buildConfig(m_config);
    } catch(std::exception & e) {
        nsw::VmmConfigIssue issue(ERS_HERE, e.what());
        ers::error(issue);
        throw issue;
    }
}

void nsw::VMMConfig::setChannelRegisterAllChannels(const std::string& register_name, const std::uint32_t value) {
    m_config.erase(register_name);
    m_config.put(register_name, value);
    m_bitstring = VMMCodec::buildConfig(m_config);
}

void nsw::VMMConfig::setChannelRegisterOneChannel(const std::string& register_name, const std::uint32_t value, const std::uint32_t channel) {
    if (!VMMCodec::channelRegisterExists(register_name)) {
        std::string temp = register_name + "(Channel register)";
        nsw::NoSuchVmmRegister issue(ERS_HERE, temp.c_str());
        ers::error(issue);
        throw issue;
    } else if (channel >= nsw::VMMCodec::NCHANNELS) {
        nsw::VmmChannelOutOfRange issue(ERS_HERE, channel);
        ers::error(issue);
        throw issue;
    }

    auto channelreg = VMMCodec::buildChannelRegisterMap(m_config);
    channelreg[register_name][channel] = value;

    ptree temp = nsw::buildPtreeFromVector(channelreg[register_name]);

    // Delete the old node, and write the new one
    m_config.erase(register_name);
    m_config.add_child(register_name, temp);

    m_bitstring = VMMCodec::buildConfig(m_config);
}

void nsw::VMMConfig::setTestPulseDAC(const std::uint32_t param) {
    //
    // param (tpdac): 0-1023
    //
    setGlobalRegister("sdp_dac", param);
}

void nsw::VMMConfig::setGlobalThreshold(const std::uint32_t param) {
    //
    // param (thdac): 0-1023
    //
    setGlobalRegister("sdt_dac", param);
}

void nsw::VMMConfig::setMonitorOutput(const std::uint32_t channel_id, const std::uint32_t param) {
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
    setGlobalRegister("scmx", param);
    setGlobalRegister("sm",   channel_id);
}

void nsw::VMMConfig::setChannelMOMode(const std::uint32_t channel_id, const std::uint32_t param) {
    //
    // param = 0: channel analog output
    // param = 1: channel threshold
    //
    setChannelRegisterOneChannel("channel_smx", param, channel_id);
}

void nsw::VMMConfig::setChannelTrimmer(const std::uint32_t channel_id, const std::uint32_t param) {
    //
    // param (trim): 0-31
    //
    setChannelRegisterOneChannel("channel_sd", param, channel_id);
}
