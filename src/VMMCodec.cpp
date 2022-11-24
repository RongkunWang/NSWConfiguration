#include "NSWConfiguration/VMMCodec.h"

#include <exception>
#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <ers/ers.h>

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

bool nsw::VMMCodec::globalRegisterExists(std::string_view register_name) {
  const auto func = [&register_name](const auto& el) {
    return register_name == el.first;
  };
  return std::any_of(std::begin(m_global_name_size0),
                     std::end(m_global_name_size0),
                     func) or
         std::any_of(std::begin(m_global_name_size1),
                     std::end(m_global_name_size1),
                     func);
}

bool nsw::VMMCodec::channelRegisterExists(std::string_view register_name) {
  const auto func = [&register_name](const auto& el) {
    return register_name == el.first;
  };
  return std::any_of(std::begin(m_channel_name_size),
                     std::end(m_channel_name_size),
                     func);
}

std::string nsw::VMMCodec::buildGlobalConfig0(const ptree& config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global0);
}

std::string nsw::VMMCodec::buildGlobalConfig1(const ptree& config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global1);
}

std::string nsw::VMMCodec::buildGlobalConfig(const ptree& config, nsw::GlobalRegisters type) {

    const auto buildBitStream = [&config] (const auto registers) {
        std::ostringstream ss;
        for (const auto [reg_name, size] : registers) {
            if (reg_name == nsw::NOT_USED) {
                const auto value = 0u;
                ss << bitString(value, size);
            } else {
                const auto reg_name_string = std::string{reg_name};
                try {
                    const auto value = config.get<unsigned>(reg_name_string);
                    nsw::checkOverflow(size, value, reg_name_string);

                    auto iter = std::find(m_bitreversed_registers.begin(), m_bitreversed_registers.end(), reg_name);
                    if (iter != m_bitreversed_registers.end()) {
                        ss << reversedBitString(value, size);
                    } else {
                        ss << bitString(value, size);
                    }
                } catch (const boost::property_tree::ptree_bad_path& e) {
                    nsw::MissingVmmRegister issue(ERS_HERE, reg_name.data());
                    ers::error(issue);
                    throw issue;
                }
            }
        }
        return ss.str();
    };

    if (type == nsw::GlobalRegisters::global0) {
        ERS_DEBUG(4, "Global 0 ");
        return buildBitStream(m_global_name_size0);
    }
    if (type == nsw::GlobalRegisters::global1) {
        ERS_DEBUG(4, "Global 1 ");
        return buildBitStream(m_global_name_size1);
    }
    throw std::logic_error(fmt::format("Received invalid type {}", type));
}

std::string nsw::VMMCodec::buildChannelConfig(const ptree& config) {
    const auto ch_reg_map = buildChannelRegisterMap(config);

    const auto getValue = [&ch_reg_map] (const auto reg_name, const std::size_t channel) {
        if (reg_name == nsw::NOT_USED) {
            return 0u;
        }
        return ch_reg_map.at(reg_name).at(channel);
    };

    std::string bitstr;
    // TODO(cyildiz): Verify if we should go from 0 to 64 or reversed
    for (std::size_t channel = nsw::vmm::NUM_CH_PER_VMM-1;; channel--) {
        std::string tempstr_ch;
        for (const auto& [reg_name, reg_size] : m_channel_name_size) {
            tempstr_ch += reversedBitString(getValue(reg_name, channel), reg_size);
        }
        // Reverse the bitstream of the channel
        std::reverse(tempstr_ch.begin(), tempstr_ch.end());
        bitstr += tempstr_ch;
        if (channel == 0) {
            break;
        }
    }
    ERS_DEBUG(6, "Channel config: " << bitstr);
    return bitstr;
}

std::string nsw::VMMCodec::buildConfig(const ptree& config) {
    return buildGlobalConfig1(config) + buildChannelConfig(config) + buildGlobalConfig0(config);
}

std::map<std::string_view, std::array<unsigned, nsw::vmm::NUM_CH_PER_VMM>> nsw::VMMCodec::buildChannelRegisterMap(ptree config) {
    std::map<std::string_view, std::array<unsigned, nsw::vmm::NUM_CH_PER_VMM>> result;

    for (const auto &[reg_name, reg_size] : m_channel_name_size) {
        std::array<unsigned, nsw::vmm::NUM_CH_PER_VMM> vtemp{};
        const std::string reg_name_string{reg_name};

        // Ignore not used bits
        if (reg_name == nsw::NOT_USED) {
            continue;
        }

        const ptree ptemp = config.get_child(reg_name_string);
        if (ptemp.empty()) {  // There is a single value for register, all channels have the same value
            const auto value =  config.get<unsigned>(reg_name_string);
            nsw::checkOverflow(reg_size, value, reg_name_string);
            for (size_t i = 0; i < nsw::vmm::NUM_CH_PER_VMM; i++) {
                vtemp.at(i) = value;
                ERS_DEBUG(5, reg_name << ": " << value);
            }
        } else {  // There is a array, each channel has different value
            size_t i = 0;
            for (auto iter = ptemp.begin(); iter != ptemp.end(); iter++) {
                const auto value = iter->second.get<unsigned>("");
                nsw::checkOverflow(reg_size, value, reg_name_string);
                vtemp.at(i) = value;
                i++;
            }
            if (i != nsw::vmm::NUM_CH_PER_VMM) {
                std::stringstream ss;
                ss << "Unexpected number of VMM channels: " << i;
                nsw::VmmCodecIssue issue(ERS_HERE, ss.str().c_str());
                ers::error(issue);
                throw issue;
            }
        }
        result[reg_name] = vtemp;
    }
    return result;
}
