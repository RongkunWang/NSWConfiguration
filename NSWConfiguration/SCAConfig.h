/// FE Base Configuration class

#ifndef NSWCONFIGURATION_SCACONFIG_H_
#define NSWCONFIGURATION_SCACONFIG_H_

#include <string>
#include <set>

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
    template <typename T>
    std::set<T> SkipRegisters() const {
        static_assert(std::is_integral_v<T>, "type must be an integral number");
      std::set<T> r;
      if ( m_config.count("SkipRegisters") != 0) {
        for (const auto& item : m_config.get_child("SkipRegisters")) {
          r.insert(item.second.get_value<T>());
        }
      }
      return r;
    }

    boost::property_tree::ptree getConfig() const {return m_config;}
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_SCACONFIG_H_
