/// FE Base Configuration class

#ifndef NSWCONFIGURATION_FECONFIG_H_
#define NSWCONFIGURATION_FECONFIG_H_

#include <string>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

namespace nsw {

class FEConfig {
 protected:
    std::string m_opcserver_ip;  /// address and port of Opc Server
    std::string m_address;   /// address of FE item in Opc address space
    ptree m_config;

 public:
    explicit FEConfig(ptree config);
    virtual ~FEConfig() {}

    std::string getOpcServerIp() const {return m_opcserver_ip;}
    std::string getAddress() const {return m_address;}
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_FECONFIG_H_
