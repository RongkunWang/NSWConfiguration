/// VMM Configuration class

#ifndef NSWCONFIGURATION_VMMCONFIG_H_
#define NSWCONFIGURATION_VMMCONFIG_H_

#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/VMMCodec.h"
#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class VMMConfig: public FEConfig {
 private:
    VMMCodec& codec = VMMCodec::Instance();
    std::string m_bitstring;  // Config information as a string of 0/1s

 public:
    explicit VMMConfig(ptree vmmconfig);
    ~VMMConfig() {}

    std::vector<uint8_t> getByteVector() const;  /// Create a vector of bytes

    void setRegister(std::string register_name, unsigned value);
    void setAllChannelRegisters(std::string register_name, unsigned value);
    void setChannelRegister(size_t channel, std::string register_name, unsigned value);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCONFIG_H_
