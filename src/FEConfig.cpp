#include <string>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

nsw::FEConfig::FEConfig(ptree config): m_config(config) {
    m_opcserver_ip = m_config.get<std::string>("OpcServerIp");
    m_address = m_config.get<std::string>("OpcNodeId");
}
