/// VMM Configuration class

#ifndef NSWCONFIGURATION_VMMCONFIG_H_
#define NSWCONFIGURATION_VMMCONFIG_H_

#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"

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

class VMMConfig {
 private:
    VMMCodec& codec = VMMCodec::Instance();
    std::string m_bitstring;  // Config information as a string of 0/1s
    std::string name;         // Name of the element (vmm0,vmm1,vmm2 ...)
    ptree m_config;

 public:
    explicit VMMConfig(ptree vmmconfig);
    ~VMMConfig() {}

    std::vector<uint8_t> getByteVector() const;  /// Create a vector of bytes

    std::string getBitString() const {return m_bitstring;}  /// return the string of bits

    void setName(std::string str) {name = str;}
    std::string getName() {return name;}

    unsigned getGlobalRegister(std::string register_name);
    unsigned getChannelRegisterOneChannel(std::string register_name, size_t channel);
    std::vector<unsigned> getChannelRegisterAllChannels(std::string register_name);

    void setGlobalRegister(std::string register_name, unsigned value);
    void setChannelRegisterAllChannels(std::string register_name, unsigned value);
    void setChannelRegisterOneChannel(std::string register_name, unsigned value, size_t channel);

    void setTestPulseDAC    (size_t param);
    void setGlobalThreshold (size_t param);
    void setMonitorOutput   (size_t channel_id, size_t param);
    void setChannelTrimmer  (size_t channel_id, size_t param);
    void setChannelMOMode   (size_t channel_id, size_t param);
};

namespace vmm {

    /// vmm register values

    /// scmx
    const size_t CommonMonitor  = 0;
    const size_t ChannelMonitor = 1;

    /// sm (when scmx = 0)
    const size_t TestPulseDAC = 1;
    const size_t ThresholdDAC = 2;
    const size_t BandGapRef   = 3;
    const size_t Temperature  = 4;
    
    /// channel_smx
    const size_t ChannelAnalogOutput     = 0;
    const size_t ChannelTrimmedThreshold = 1;

}

}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCONFIG_H_
