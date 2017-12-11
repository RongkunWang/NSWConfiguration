/// ROC Configuration class

#ifndef NSWCONFIGURATION_ROCCONFIG_H_
#define NSWCONFIGURATION_ROCCONFIG_H_

#include <bitset>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCCodec.h"

using boost::property_tree::ptree;

namespace nsw {

class ROCConfig {
 private:
    ROCCodec& codec = ROCCodec::Instance();
    std::string m_opcserver_ip;
    std::string m_vmm_address;
    ptree m_config;

 public:
    explicit ROCConfig(ptree vmmconfig);
    ~ROCConfig() {}

    std::string getOpcServerIp() {return m_opcserver_ip;}
    std::string getVmmAddress() {return m_vmm_address;}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCONFIG_H_
