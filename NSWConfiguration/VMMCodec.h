#include <bitset>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <utility>

#include <boost/property_tree/ptree.hpp>

#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

#ifndef NSWCONFIGURATION_VMMCODEC_H_
#define NSWCONFIGURATION_VMMCODEC_H_


namespace nsw {

enum class GlobalRegisters {global0, global1};

/// Class to encode/decode VMM configuration between ptrees and bytestreams
class VMMCodec {
 public:
    VMMCodec();
    ~VMMCodec() {}

    static constexpr size_t nchannels = 64;
    static constexpr size_t nbits = 1728;  /// total number of bits (2 * nbits_global + nbits_channel)
    static constexpr size_t nbits_global = 96;  /// Size of globals registers (32 * 3)
    static constexpr size_t nbits_channel = 1536;  /// Size of channel registers (24 * 64)

    std::bitset<nbits_global> build_global_config0(ptree config);
    std::bitset<nbits_global> build_global_config1(ptree config);
    std::bitset<nbits_channel> build_channel_config(ptree config);
    std::bitset<nbits> build_config(ptree config);
    // ptree bitset_to_ptree(std::bitset<nbits> bs) {}

 private:
    /// Creates a vector for each channel register, such that element ["sd"][4] is sd value for 4th channel
    std::map<std::string, std::vector<unsigned>> build_channel_register_map(ptree config);

    std::bitset<nbits_global> build_global_config(ptree config, GlobalRegisters type);

    void check_overflow(size_t register_size, unsigned value, std::string register_name);

    using name_size_t = std::pair<std::string, size_t>;

    /// In following vectors each element is pair such as {register name, register size in bits}
    std::vector<name_size_t> m_global_name_size0;
    std::vector<name_size_t> m_global_name_size1;
    std::vector<name_size_t> m_channel_name_size;
};
#endif  // NSWCONFIGURATION_VMMCODEC_H_
}  // namespace nsw
