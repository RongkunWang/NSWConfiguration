/// FE Base Configuration class

#ifndef NSWCONFIGURATION_FECONFIG_H_
#define NSWCONFIGURATION_FECONFIG_H_

#include <string>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

namespace nsw {

//! Base class for front ends that can be communicated via Opc path

class FEConfig {
 protected:
    std::string m_opcserver_ip;  /// address and port of Opc Server
    std::string m_address;   /// SCA address of FE item in Opc address space
    ptree m_config;  /// Configuration ptree that contains at least OpcServerIp and OpcNodeId:w

 public:
    explicit FEConfig(ptree config);
    virtual ~FEConfig() {}

    std::string getOpcServerIp() const {return m_opcserver_ip;}
    std::string getAddress() const {return m_address;}

    ptree getConfig() const {return m_config;}
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_FECONFIG_H_
