/// VMM Configuration class

#ifndef NSWCONFIGURATION_VMMCONFIG_H_
#define NSWCONFIGURATION_VMMCONFIG_H_

#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

ERS_DECLARE_ISSUE(nsw,
                  NoSuchVmmRegister,
                  "No VMM register with this name: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  VmmChannelOutOfRange,
                  "Channel number out of range: " << channel,
                  ((size_t)channel)
                  )

namespace nsw {

class VMMConfig: public FEConfig {
 private:
    VMMCodec& codec = VMMCodec::Instance();
    std::string m_bitstring;  // Config information as a string of 0/1s

 public:
    explicit VMMConfig(ptree vmmconfig);
    ~VMMConfig() {}

    std::vector<uint8_t> getByteVector() const;  /// Create a vector of bytes

    std::string getBitString() const {return m_bitstring;}  /// return the string of bits

    unsigned getGlobalRegister(std::string register_name);
    unsigned getChannelRegisterOneChannel(std::string register_name, size_t channel);
    std::vector<unsigned> getChannelRegisterAllChannels(std::string register_name);

    void setGlobalRegister(std::string register_name, unsigned value);
    void setChannelRegisterAllChannels(std::string register_name, unsigned value);
    void setChannelRegisterOneChannel(std::string register_name, unsigned value, size_t channel);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCONFIG_H_
