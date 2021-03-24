// VMM Encoder/Decoder class to convert ptree/bitstreams to each other
// This is a singleton function that should be created as follows:
// nsw::VMMCodec& vmmcodec = nsw::VMMCodec::Instance();

#ifndef NSWCONFIGURATION_VMMCODEC_H_
#define NSWCONFIGURATION_VMMCODEC_H_

#include <vector>
#include <map>
#include <string>
#include <utility>

#include "boost/property_tree/ptree_fwd.hpp"

#include "ers/Issue.h"

ERS_DECLARE_ISSUE(nsw,
                  MissingVmmRegister,
                  "VMM register missing from configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  VmmCodecIssue,
                  message,
                  ((const char *)message)
                  )

namespace nsw {

enum class GlobalRegisters {global0, global1};

/// Class to encode/decode VMM configuration between ptrees and bytestreams
class VMMCodec {
 public:
    static VMMCodec& Instance();

    static constexpr size_t NCHANNELS = 64;
    static constexpr size_t NBITS_GLOBAL = 32 * 3;  /// Size of globals registers
    static constexpr size_t NBITS_CHANNEL = 24 * NCHANNELS;  /// Size of channel registers
    static constexpr size_t NBITS_TOTAL = NBITS_CHANNEL + 2*NBITS_GLOBAL;  /// total number of bits

    std::string buildConfig(const boost::property_tree::ptree& config) const;

    bool globalRegisterExists(const std::string& register_name) const;
    bool channelRegisterExists(const std::string& register_name) const;

    /// Creates a vector for each channel register, such that element ["channel_sd"][4] is sd value for 4th channel
    std::map<std::string, std::vector<unsigned>> buildChannelRegisterMap(boost::property_tree::ptree config) const;

 private:
    /// Private VMMCodec for singleton class
    VMMCodec();
   ~VMMCodec();

    std::string buildGlobalConfig(const boost::property_tree::ptree& config, GlobalRegisters type) const;

    std::string buildGlobalConfig0(const boost::property_tree::ptree& config) const;
    std::string buildGlobalConfig1(const boost::property_tree::ptree& config) const;
    std::string buildChannelConfig(const boost::property_tree::ptree& config) const;

    // void checkOverflow(size_t register_size, unsigned value, const std::string& register_name);

    /// Type to be used for pairs that contain: {register name, register size in bits}
    using NameSizeType = std::pair<std::string, size_t>;

    /// In following vectors each element is pair such as {register name, register size in bits}
    std::vector<NameSizeType> m_global_name_size0;
    std::vector<NameSizeType> m_global_name_size1;
    std::vector<NameSizeType> m_channel_name_size;

    /// List of register names, whose bits are reversed in the bitstream
    /// For instance, if value of a bitreversed register with size 5 is 3,
    /// it will be put in bitstream as 11000 (instead of 00011)
    std::vector<std::string> m_bitreversed_registers;
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCODEC_H_
