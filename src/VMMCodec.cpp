#include <cmath>
#include <string>
#include <exception>

#include "NSWConfiguration/VMMCodec.h"

constexpr size_t nsw::VMMCodec::nbits;
constexpr size_t nsw::VMMCodec::nbits_global;
constexpr size_t nsw::VMMCodec::nbits_channel;
constexpr size_t nsw::VMMCodec::nchannels;

void nsw::VMMCodec::check_overflow(size_t register_size, unsigned value, std::string register_name) {
    if (std::pow(2, register_size) <= value) {
        std::string err = "Overflow, register: " + register_name + ", max value: "
                           + std::to_string(std::pow(2, register_size)-1)
                           + ", actual value: " + std::to_string(value);
        throw std::runtime_error(err);
    }
}

nsw::VMMCodec::VMMCodec() {
    std::cout << "Constructor, building lookup vectors" << std::endl;
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
    m_global_name_size0.push_back({"scmx", 1});
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

    m_global_name_size0.push_back({"reset0", 5});
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
    m_global_name_size0.push_back({"reset1", 2});

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

    m_channel_name_size.push_back({"sc", 1});
    m_channel_name_size.push_back({"sl", 1});
    m_channel_name_size.push_back({"st", 1});
    m_channel_name_size.push_back({"sth", 1});
    m_channel_name_size.push_back({"sm", 1});
    m_channel_name_size.push_back({"smx", 1});
    m_channel_name_size.push_back({"sd", 5});
    m_channel_name_size.push_back({"sz10b", 5});
    m_channel_name_size.push_back({"sz8b", 4});
    m_channel_name_size.push_back({"sz6b", 3});
    m_channel_name_size.push_back({"nu4", 3});
}

std::bitset<nsw::VMMCodec::nbits_global> nsw::VMMCodec::build_global_config0(ptree config) {
    return build_global_config(config, GlobalRegisters::global0);
}

std::bitset<nsw::VMMCodec::nbits_global> nsw::VMMCodec::build_global_config1(ptree config) {
    return build_global_config(config, GlobalRegisters::global1);
}

std::bitset<nsw::VMMCodec::nbits_global> nsw::VMMCodec::build_global_config(ptree config, GlobalRegisters type) {
    auto constexpr N = nsw::VMMCodec::nbits_global;

    std::vector<name_size_t> vname_size;
    if (type == GlobalRegisters::global0) {
        vname_size = m_global_name_size0;
    } else if (type == GlobalRegisters::global1) {
        vname_size = m_global_name_size1;
    }

    std::bitset<N> global;
    std::bitset<N> temp;

    std::cout << "Ptree" << std::endl;

    size_t position = 0;
    for (auto name_size : vname_size) {
        std::string register_name = name_size.first;
        size_t register_size = name_size.second;
        unsigned value = config.get<unsigned>(register_name);
        std::cout << register_name << " : " << register_size << " -> " << value << std::endl;
        temp = value;
        global = global | (temp << position);
        position = position + register_size;

        check_overflow(register_size, value, register_name);
    }
    std::cout << global << std::endl;

    return global;
}

std::bitset<nsw::VMMCodec::nbits_channel> nsw::VMMCodec::build_channel_config(ptree config) {
    auto constexpr Nch = nsw::VMMCodec::nchannels;
    auto constexpr N = nsw::VMMCodec::nbits_channel;

    std::bitset<N> result;
    std::bitset<N> temp;

    auto ch_reg_map = build_channel_register_map(config);

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

std::map<std::string, std::vector<unsigned>> nsw::VMMCodec::build_channel_register_map(ptree config) {
    auto constexpr Nch = nsw::VMMCodec::nchannels;
    std::map<std::string, std::vector<unsigned>> result;

    for (auto name_size : m_channel_name_size) {
        std::vector<unsigned> vtemp;
        std::string register_name = name_size.first;
        size_t register_size = name_size.second;

        ptree ptemp = config.get_child(register_name);
        std::cout << register_name;
        if (ptemp.empty()) {
            unsigned value =  config.get<unsigned>(register_name);
            check_overflow(register_size, value, register_name);
            for (size_t i = 0; i < Nch; i++) {
                vtemp.push_back(value);
                std::cout << value << ", ";
            }
        } else {
            size_t i = 0;
            for (ptree::iterator iter = ptemp.begin(); iter != ptemp.end(); iter++) {
                unsigned value = iter->second.get<unsigned>("");
                check_overflow(register_size, value, register_name);
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
