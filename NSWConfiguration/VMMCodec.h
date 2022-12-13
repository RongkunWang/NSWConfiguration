// VMM Encoder/Decoder class to convert ptree/bitstreams to each other
// This is a singleton function that should be created as follows:
// nsw::VMMCodec& vmmcodec = nsw::VMMCodec::Instance();

#ifndef NSWCONFIGURATION_VMMCODEC_H_
#define NSWCONFIGURATION_VMMCODEC_H_

#include <vector>
#include <map>
#include <string>
#include <utility>

#include "NSWConfiguration/Constants.h"

#include <boost/property_tree/ptree_fwd.hpp>

#include <ers/Issue.h>

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
    static constexpr size_t NCHANNELS = 64;
    static constexpr size_t NBITS_GLOBAL = 32 * 3;  /// Size of globals registers
    static constexpr size_t NBITS_CHANNEL = 24 * NCHANNELS;  /// Size of channel registers
    static constexpr size_t NBITS_TOTAL = NBITS_CHANNEL + 2*NBITS_GLOBAL;  /// total number of bits

    static std::string buildConfig(const boost::property_tree::ptree& config);

    static bool globalRegisterExists(std::string_view register_name);
    static bool channelRegisterExists(std::string_view register_name);

    /// Creates a vector for each channel register, such that element ["channel_sd"][4] is sd value for 4th channel
    static std::map<std::string_view, std::array<unsigned, nsw::vmm::NUM_CH_PER_VMM>> buildChannelRegisterMap(boost::property_tree::ptree config);

 private:
    static std::string buildGlobalConfig(const boost::property_tree::ptree& config, GlobalRegisters type);

    static std::string buildGlobalConfig0(const boost::property_tree::ptree& config);
    static std::string buildGlobalConfig1(const boost::property_tree::ptree& config);
    static std::string buildChannelConfig(const boost::property_tree::ptree& config);

    // void checkOverflow(size_t register_size, unsigned value, const std::string& register_name);

    // Sizes of arrays
    constexpr static std::size_t NUM_GLOBAL_REGS_SIZE0 = 57;
    constexpr static std::size_t NUM_GLOBAL_REGS_SIZE1 = 13;
    constexpr static std::size_t NUM_CHANNEL_REGS = 11;
    constexpr static std::size_t NUM_REVERSED_REGS = 5;

    /// Type to be used for pairs that contain: {register name, register size in bits}
    using NameSizeType = std::pair<std::string_view, size_t>;

    /// In following vectors each element is pair such as {register name, register size in bits}
    constexpr static std::array<NameSizeType, NUM_GLOBAL_REGS_SIZE0> m_global_name_size0{
      std::pair{"reset", 2},    std::pair{nsw::NOT_USED, 5},
      std::pair{"stgc", 1},     std::pair{"slxh", 1},
      std::pair{"slh", 1},      std::pair{"sL0enaV", 1},
      std::pair{"slvs6b", 1},   std::pair{"slvsena", 1},
      std::pair{"slvstki", 1},  std::pair{"slvsart", 1},
      std::pair{"slvsdt", 1},   std::pair{"slvstk", 1},
      std::pair{"slvstp", 1},   std::pair{"slvsbc", 1},
      std::pair{"sfrst", 1},    std::pair{"srat", 1},
      std::pair{"sbip", 1},     std::pair{"stlc", 1},
      std::pair{"srec", 1},     std::pair{"ssart", 1},
      std::pair{"stcr", 1},     std::pair{"s32", 1},
      std::pair{"slvs", 1},     std::pair{"res", 4},
      std::pair{"res00", 1},    std::pair{"stpp", 1},
      std::pair{"sdrv", 1},     std::pair{"sdck6b", 1},
      std::pair{"sdcka", 1},    std::pair{"sdcks", 1},
      std::pair{"s10b", 1},     std::pair{"s6b", 1},
      std::pair{"s8b", 1},      std::pair{"sc6b", 3},
      std::pair{"sc8b", 2},     std::pair{"sc10b", 2},
      std::pair{"sdp_dac", 10}, std::pair{"sdt_dac", 10},
      std::pair{"stc", 2},      std::pair{"ssh", 1},
      std::pair{"sttt", 1},     std::pair{"stot", 1},
      std::pair{"sng", 1},      std::pair{"sg", 3},
      std::pair{"sfm", 1},      std::pair{"st", 2},
      std::pair{"sfam", 1},     std::pair{"sfa", 1},
      std::pair{"scmx", 1},     std::pair{"sm", 6},
      std::pair{"slg", 1},      std::pair{"sbfm", 1},
      std::pair{"sbfp", 1},     std::pair{"sbft", 1},
      std::pair{"sbmx", 1},     std::pair{"sdp", 1},
      std::pair{"sp", 1}};

    constexpr static std::array<NameSizeType, NUM_GLOBAL_REGS_SIZE1> m_global_name_size1{
      std::pair{nsw::NOT_USED, 8},
      std::pair{"offset", 12},
      std::pair{"l0offset", 12},
      std::pair{"rollover", 12},
      std::pair{"window", 3},
      std::pair{"nskip", 7},
      std::pair{"truncate", 6},
      std::pair{"sL0ena", 1},
      std::pair{"sL0ckinv", 1},
      std::pair{"sL0dckinv", 1},
      std::pair{"sL0cktest", 1},
      std::pair{"nskipm", 1},
      std::pair{nsw::NOT_USED, 31}};

    constexpr static std::array<NameSizeType, NUM_CHANNEL_REGS>
      m_channel_name_size{std::pair{"channel_sc", 1},
                          std::pair{"channel_sl", 1},
                          std::pair{"channel_st", 1},
                          std::pair{"channel_sth", 1},
                          std::pair{"channel_sm", 1},
                          std::pair{"channel_smx", 1},
                          std::pair{"channel_sd", 5},
                          std::pair{"channel_sz10b", 5},
                          std::pair{"channel_sz8b", 4},
                          std::pair{"channel_sz6b", 3},
                          std::pair{nsw::NOT_USED, 1}};

    /// List of register names, whose bits are reversed in the bitstream
    /// For instance, if value of a bitreversed register with size 5 is 3,
    /// it will be put in bitstream as 11000 (instead of 00011)
    constexpr static std::array<std::string_view, NUM_REVERSED_REGS>
      m_bitreversed_registers{"sm", "st", "sg", "sdt_dac", "sdp_dac"};
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCODEC_H_
