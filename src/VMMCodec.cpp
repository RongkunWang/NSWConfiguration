#include <map>
#include <cmath>
#include <string>
#include <exception>
#include <vector>

#include "NSWConfiguration/VMMCodec.h"

constexpr size_t nsw::VMMCodec::NBITS_TOTAL;
constexpr size_t nsw::VMMCodec::NBITS_GLOBAL;
constexpr size_t nsw::VMMCodec::NBITS_CHANNEL;
constexpr size_t nsw::VMMCodec::NCHANNELS;

void nsw::VMMCodec::checkOverflow(size_t register_size, unsigned value, std::string register_name) {
    if (std::pow(2, register_size) <= value) {
        std::string err = "Overflow, register: " + register_name + ", max value: "
                           + std::to_string(std::pow(2, register_size)-1)
                           + ", actual value: " + std::to_string(value);
        throw std::runtime_error(err);  // TODO(cyildiz): convert to ers
    }
}

nsw::VMMCodec::VMMCodec() {
    std::cout << "Constructing VMM Codec, building lookup vectors" << std::endl;
    // We could read these mappings from a configuration file, but these are not supposed to change in time,
    // It may be better to keep them hardcoded

    // TODO(cyildiz): Implement a mechanism to ignore the non used bits (nu...)

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
    m_global_name_size0.push_back({"nu1", 8});
    m_global_name_size0.push_back({"reset", 2});

    m_global_name_size1.push_back({"nu2", 31});
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
    m_global_name_size1.push_back({"nu3", 8});

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
    m_channel_name_size.push_back({"channel_nu4", 1});
}

nsw::VMMCodec& nsw::VMMCodec::Instance() {
    static nsw::VMMCodec c;
    return c;
}

std::bitset<nsw::VMMCodec::NBITS_GLOBAL> nsw::VMMCodec::buildGlobalConfig0(ptree config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global0);
}

std::bitset<nsw::VMMCodec::NBITS_GLOBAL> nsw::VMMCodec::buildGlobalConfig1(ptree config) {
    return buildGlobalConfig(config, nsw::GlobalRegisters::global1);
}

std::bitset<nsw::VMMCodec::NBITS_GLOBAL> nsw::VMMCodec::buildGlobalConfig(ptree config, nsw::GlobalRegisters type) {
    auto constexpr N = nsw::VMMCodec::NBITS_GLOBAL;

    std::vector<NameSizeType> vname_size;
    if (type == nsw::GlobalRegisters::global0) {
        vname_size = m_global_name_size0;
        std::cout << "Global 0 " << std::endl;
    } else if (type == nsw::GlobalRegisters::global1) {
        vname_size = m_global_name_size1;
        std::cout << "Global 1 " << std::endl;
    }

    std::string bitstr = "";

    for (auto name_size : vname_size) {
        std::string register_name = name_size.first;
        size_t register_size = name_size.second;
        unsigned value = config.get<unsigned>(register_name);

        checkOverflow(register_size, value, register_name);

        // TODO(cyildiz): Verify which of the following should be used for each register
        auto str = reversedBits(value, register_size);
        //auto str = bits(value, register_size);
        bitstr = str + bitstr;

        std::cout << register_name << " : " << register_size << " -> ";
        std::cout << value << " - reversed: " << str << std::endl;
    }

    std::bitset<N> global(bitstr);
    std::cout << "global regs: " << global << std::endl;
    return global;
}

std::bitset<nsw::VMMCodec::NBITS_CHANNEL> nsw::VMMCodec::buildChannelConfig(ptree config) {
    auto constexpr Nch = nsw::VMMCodec::NCHANNELS;
    auto constexpr N = nsw::VMMCodec::NBITS_CHANNEL;

    std::bitset<N> result;
    std::bitset<N> temp;

    auto ch_reg_map = buildChannelRegisterMap(config);

    // TODO(cyildiz): Verify bit orderings are correct
    size_t position = 0;
    for (size_t channel = 0; channel < Nch; channel++) {
        for (auto name_size : m_channel_name_size) {
            std::string register_name = name_size.first;
            size_t register_size = name_size.second;

            temp = ch_reg_map[register_name][channel];
            result = result | (temp << position);
            position = position + register_size;
        }
    }
    std::cout << result << std::endl;

    return result;
}

std::bitset<nsw::VMMCodec::NBITS_TOTAL> nsw::VMMCodec::buildConfig(ptree config) {
    return nsw::concatenate(buildGlobalConfig1(config), buildChannelConfig(config), buildGlobalConfig0(config));
}

std::map<std::string, std::vector<unsigned>> nsw::VMMCodec::buildChannelRegisterMap(ptree config) {
    auto constexpr Nch = nsw::VMMCodec::NCHANNELS;
    std::map<std::string, std::vector<unsigned>> result;

    for (auto name_size : m_channel_name_size) {
        std::vector<unsigned> vtemp;
        std::string register_name = name_size.first;
        size_t register_size = name_size.second;

        ptree ptemp = config.get_child(register_name);
        std::cout << register_name << " : ";
        if (ptemp.empty()) {
            unsigned value =  config.get<unsigned>(register_name);
            checkOverflow(register_size, value, register_name);
            for (size_t i = 0; i < Nch; i++) {
                vtemp.push_back(value);
                std::cout << value << ", ";
            }
        } else {
            size_t i = 0;
            for (ptree::iterator iter = ptemp.begin(); iter != ptemp.end(); iter++) {
                unsigned value = iter->second.get<unsigned>("");
                checkOverflow(register_size, value, register_name);
                vtemp.push_back(value);
                std::cout << value << ", ";
                i++;
            }
            if (vtemp.size() != Nch) {
                throw std::runtime_error("Unexpected number of channels!");
            }
        }
        std::cout << "\n";
        result[register_name] = vtemp;
    }
    return result;
}
