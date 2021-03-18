/// FE Base Configuration class

#ifndef NSWCONFIGURATION_SCACONFIG_H_
#define NSWCONFIGURATION_SCACONFIG_H_

#include <string>

#include "ers/Issue.h"

ERS_DECLARE_ISSUE(nsw,
                  SCAConfigIssue,
                  message,
                  ((const char*)message)
                  )

#include "boost/property_tree/ptree.hpp"

namespace nsw {

//! Base class for front ends that can be communicated via Opc path

class SCAConfig {
 protected:
    std::string m_opcserver_ip;  /// address and port of Opc Server
    std::string m_address;   /// SCA address of FE item in Opc address space
    boost::property_tree::ptree m_config;  /// Configuration ptree that contains at least OpcServerIp and OpcNodeId

 public:
    explicit SCAConfig(const boost::property_tree::ptree& config);
    virtual ~SCAConfig() = default;

    std::string getOpcServerIp() const {return m_opcserver_ip;}
    std::string getAddress() const {return m_address;}

    boost::property_tree::ptree getConfig() const {return m_config;}
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_SCACONFIG_H_
