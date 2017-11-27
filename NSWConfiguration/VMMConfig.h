/// VMM Configuration class

#ifndef NSWCONFIGURATION_VMMCONFIG_H_
#define NSWCONFIGURATION_VMMCONFIG_H_

#include <bitset>
#include <string>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

namespace nsw {

class VMMConfig {
 private:
    VMMCodec& codec = VMMCodec::Instance();
    std::string m_opcserver_ip;
    std::string m_vmm_address;
    std::bitset<nsw::VMMCodec::NBITS_TOTAL> m_bitset;  // Config information in terms of bitset
    ptree m_config;

 public:
    explicit VMMConfig(ptree vmmconfig);
    ~VMMConfig() {}

    std::array<uint8_t, nsw::VMMCodec::NBITS_TOTAL/8> getByteArray();  /// Create btye array from the bitset
    std::string getOpcServerIp() {return m_opcserver_ip;}
    std::string getVmmAddress() {return m_vmm_address;}

    void setRegister(std::string register_name, unsigned value);
    void setAllChannelRegisters(std::string register_name, unsigned value);
    void setChannelRegister(size_t channel, std::string register_name, unsigned value);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_VMMCONFIG_H_
