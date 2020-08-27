#include <map>
#include <string>
#include <exception>
#include <vector>
#include <algorithm>

#include "ers/ers.h"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/Utility.h"

constexpr size_t nsw::VMMCodec::NBITS_TOTAL;
constexpr size_t nsw::VMMCodec::NBITS_GLOBAL;
constexpr size_t nsw::VMMCodec::NBITS_CHANNEL;
constexpr size_t nsw::VMMCodec::NCHANNELS;

nsw::VMMCodec::VMMCodec() {
    ERS_DEBUG(1, "Constructing VMM Codec, building lookup vectors");
    // We could read these mappings from a configuration file, but these are not supposed to change in time,
    // It may be better to keep them hardcoded


    m_global_name_size0.push_back({"sp", 1});
    m_global_name_size0.push_back({"sdp", 1});
    m_global_name_size0.push_back({"sbmx", 1});
    m_global_name_size0.push_back({"sbft", 1});
    m_global_name_size0.push_back({"sbfp", 1});
    m_global_name_size0.push_back({"sbfm", 1});
    m_global_name_size0.push_back({"slg", 1});
    m_global_name_size0.push_back({"sm", 6});
    m_global_name_size0.push_back({"scmx", 1});  // TODO(cyildiz): scmx and sm are connected. Handle it in the code
    m_global_name_size0.push_back({"sfa", 1});
    m_global_name_size0.push_back({"sfam", 1});
    m_global_name_size0.push_back({"st", 2});
    m_global_name_size0.push_back({"sfm", 1});
    m_global_name_size0.push_back({"sg", 3});
    m_global_name_size0.push_back({"sng", 1});
    m_global_name_size0.push_back({"stot", 1});
    m_global_name_size0.push_back({"sttt", 1});
    m_global_name_size0.push_back({"ssh", 1});
    m_global_name_size0.push_back({"stc", 2});
    m_global_name_size0.push_back({"sdt_dac", 10});
    m_global_name_size0.push_back({"sdp_dac", 10});
    m_global_name_size0.push_back({"sc10b", 2});
    m_global_name_size0.push_back({"sc8b", 2});
    m_global_name_size0.push_back({"sc6b", 3});
    m_global_name_size0.push_back({"s8b", 1});
    m_global_name_size0.push_back({"s6b", 1});
    m_global_name_size0.push_back({"s10b", 1});
    m_global_name_size0.push_back({"sdcks", 1});
    m_global_name_size0.push_back({"sdcka", 1});
    m_global_name_size0.push_back({"sdck6b", 1});
    m_global_name_size0.push_back({"sdrv", 1});
    m_global_name_size0.push_back({"stpp", 1});
    m_global_name_size0.push_back({"res00", 1});

    m_global_name_size0.push_back({"res", 4});
    m_global_name_size0.push_back({"slvs", 1});
    m_global_name_size0.push_back({"s32", 1});
    m_global_name_size0.push_back({"stcr", 1});
    m_global_name_size0.push_back({"ssart", 1});
    m_global_name_size0.push_back({"srec", 1});
    m_global_name_size0.push_back({"stlc", 1});
    m_global_name_size0.push_back({"sbip", 1});
    m_global_name_size0.push_back({"srat", 1});
    m_global_name_size0.push_back({"sfrst", 1});
    m_global_name_size0.push_back({"slvsbc", 1});
    m_global_name_size0.push_back({"slvstp", 1});
    m_global_name_size0.push_back({"slvstk", 1});
    m_global_name_size0.push_back({"slvsdt", 1});
    m_global_name_size0.push_back({"slvsart", 1});
    m_global_name_size0.push_back({"slvstki", 1});
    m_global_name_size0.push_back({"slvsena", 1});
    m_global_name_size0.push_back({"slvs6b", 1});
    m_global_name_size0.push_back({"sL0enaV", 1});
    m_global_name_size0.push_back({"slh", 1});
    m_global_name_size0.push_back({"slxh", 1});
    m_global_name_size0.push_back({"stgc", 1});
    m_global_name_size0.push_back({"NOT_USED", 5});
    m_global_name_size0.push_back({"reset", 2});

    m_global_name_size1.push_back({"NOT_USED", 31});
    m_global_name_size1.push_back({"nskipm", 1});
    m_global_name_size1.push_back({"sL0cktest", 1});
    m_global_name_size1.push_back({"sL0dckinv", 1});
    m_global_name_size1.push_back({"sL0ckinv", 1});
    m_global_name_size1.push_back({"sL0ena", 1});
    m_global_name_size1.push_back({"truncate", 6});
    m_global_name_size1.push_back({"nskip", 7});
    m_global_name_size1.push_back({"window", 3});
    m_global_name_size1.push_back({"rollover", 12});
    m_global_name_size1.push_back({"l0offset", 12});
    m_global_name_size1.push_back({"offset", 12});
    m_global_name_size1.push_back({"NOT_USED", 8});

    m_channel_name_size.push_back({"channel_sc", 1});
    m_channel_name_size.push_back({"channel_sl", 1});
    m_channel_name_size.push_back({"channel_st", 1});
    m_channel_name_size.push_back({"channel_sth", 1});
    m_channel_name_size.push_back({"channel_sm", 1});
    m_channel_name_size.push_back({"channel_smx", 1});
    m_channel_name_size.push_back({"channel_sd", 5});
    m_channel_name_size.push_back({"channel_sz10b", 5});
    m_channel_name_size.push_back({"channel_sz8b", 4});
    m_channel_name_size.push_back({"channel_sz6b", 3});
    m_channel_name_size.push_back({"NOT_USED", 1});

    // TODO(cyildiz): Verify this is a complete list of reversed registers
    m_bitreversed_registers.push_back("sm");
    m_bitreversed_registers.push_back("st");
    m_bitreversed_registers.push_back("sg");
    m_bitreversed_registers.push_back("sdt_dac");
    m_bitreversed_registers.push_back("sdp_dac");
    // m_bitreversed_registers.push_back("truncate");
    // m_bitreversed_registers.push_back("l0offset");
     ERS_DEBUG(1, "Reversed regs:");
    for (auto m : m_bitreversed_registers) {
        ERS_DEBUG(1, m);
    }
}

nsw::VMMCodec& nsw::VMMCodec::Instance() {
    static nsw::VMMCodec c;
    return c;
}

bool nsw::VMMCodec::globalRegisterExists(const std::string& register_name) {
    for (auto name_size : m_global_name_size0) {
        if (register_name == name_size.first) {
            return true;
        }
    }
    for (auto name_size : m_global_name_size1) {
        if (register_name == name_size.first) {
            return true;
        }
    }
    return false;
}

bool nsw::VMMCodec::channelRegisterExists(const std::string& register_name) {
    for (auto name_size : m_channel_name_size) {
        if (register_name == name_size.first) {
            return true;
        }
    }
    return false;
}

std::string nsw::VMMCodec::buildGlobalConfig0(const ptree& config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global0);
}

std::string nsw::VMMCodec::buildGlobalConfig1(const ptree& config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global1);
}

std::string nsw::VMMCodec::buildGlobalConfig(const ptree& config, nsw::GlobalRegisters type) {
    std::vector<NameSizeType> vname_size;
    if (type == nsw::GlobalRegisters::global0) {
        vname_size = m_global_name_size0;
        ERS_DEBUG(4, "Global 0 ");
    } else if (type == nsw::GlobalRegisters::global1) {
        vname_size = m_global_name_size1;
        ERS_DEBUG(4, "Global 1 ");
    }

    std::string bitstr;

    for (auto ns : vname_size) {
        auto name = ns.first;
        auto size = ns.second;

        unsigned value;
        std::string str;
        // Fill not used bits with 0
        if (name == "NOT_USED") {
            value = 0;
            str = bitString(value, size);
        } else {
            try {
                value = config.get<unsigned>(name);
            } catch (const boost::property_tree::ptree_bad_path& e) {
                nsw::MissingVmmRegister issue(ERS_HERE, name.c_str());
                ers::error(issue);
                throw issue;
            }
            nsw::checkOverflow(size, value, name);

            auto iter = std::find(m_bitreversed_registers.begin(), m_bitreversed_registers.end(), name);
            if (iter != m_bitreversed_registers.end()) {
                str = reversedBitString(value, size);
                ERS_DEBUG(5, name << " -- " << value << " - reversed: " << str);
            } else {
                str = bitString(value, size);
                ERS_DEBUG(5, name << " -- " << value << " - regular: " << str);
            }
        }

        bitstr = str + bitstr;
    }

    ERS_DEBUG(6, "global regs: " << bitstr);
    // std::reverse(bitstr.begin(), bitstr.end());
    return bitstr;
}

std::string nsw::VMMCodec::buildChannelConfig(const ptree& config) {
    auto constexpr Nch = nsw::VMMCodec::NCHANNELS;

    auto ch_reg_map = buildChannelRegisterMap(config);

    unsigned value;

    std::string bitstr;
    std::string tempstr_ch;
    // TODO(cyildiz): Verify if we should go from 0 to 64 or reversed
    for (int channel = Nch-1; channel >= 0; channel--) {
        tempstr_ch = "";
        for (auto name_size : m_channel_name_size) {
            std::string register_name = name_size.first;
            size_t register_size = name_size.second;

            if (register_name == "NOT_USED") {
                value = 0;
            } else {
                value = ch_reg_map[register_name][channel];
            }
            tempstr_ch += reversedBitString(value, register_size);
        }
        // Reverse the bitstream of the channel
        std::reverse(tempstr_ch.begin(), tempstr_ch.end());
        bitstr += tempstr_ch;
    }
    ERS_DEBUG(6, "Channel config: " << bitstr);
    return bitstr;
}

std::string nsw::VMMCodec::buildConfig(const ptree& config) {
    return buildGlobalConfig1(config) + buildChannelConfig(config) + buildGlobalConfig0(config);
}

std::map<std::string, std::vector<unsigned>> nsw::VMMCodec::buildChannelRegisterMap(ptree config) {
    auto constexpr Nch = nsw::VMMCodec::NCHANNELS;
    std::map<std::string, std::vector<unsigned>> result;

    for (auto name_size : m_channel_name_size) {
        std::vector<unsigned> vtemp;
        std::string register_name = name_size.first;
        size_t register_size = name_size.second;

        // Ignore not used bits
        if (register_name == "NOT_USED") {
            continue;
        }

        ptree ptemp = config.get_child(register_name);
        if (ptemp.empty()) {  // There is a single value for register, all channels have the same value
            unsigned value =  config.get<unsigned>(register_name);
            nsw::checkOverflow(register_size, value, register_name);
            for (size_t i = 0; i < Nch; i++) {
                vtemp.push_back(value);
                ERS_DEBUG(5, register_name << ": " << value);
            }
        } else {  // There is a array, each channel has different value
            size_t i = 0;
            std::string tmpstr;
            for (ptree::iterator iter = ptemp.begin(); iter != ptemp.end(); iter++) {
                unsigned value = iter->second.get<unsigned>("");
                nsw::checkOverflow(register_size, value, register_name);
                vtemp.push_back(value);
                tmpstr = tmpstr + std::to_string(value) + ", ";
                i++;
            }
            if (vtemp.size() != Nch) {
                std::stringstream ss;
                ss << "Unexpected number of VMM channels: " << vtemp.size();
                nsw::VmmCodecIssue issue(ERS_HERE, ss.str().c_str());
                ers::error(issue);
                throw issue;
            }
            ERS_DEBUG(5, register_name << ": " << tmpstr);
        }
        result[register_name] = vtemp;
    }
    return result;
}
