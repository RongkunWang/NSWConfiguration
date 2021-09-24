#include "NSWConfiguration/SCAConfig.h"

#include <iostream>
#include <sstream>

#include "boost/property_tree/json_parser.hpp"

#include <ers/ers.h>

using boost::property_tree::ptree;

nsw::SCAConfig::SCAConfig(const ptree& config): m_config(config) {
    try {
        m_opcserver_ip = m_config.get<std::string>("OpcServerIp");
        m_address = m_config.get<std::string>("OpcNodeId");
    } catch (const boost::property_tree::ptree_bad_path& e) {
        std::stringstream ss;
        ss << "OpcServerIp or OpcNodeId missing in configuration! " << e.what();
        nsw::SCAConfigIssue issue(ERS_HERE, ss.str().c_str());
        ers::error(issue);
        ss << " - Problematic FE ptree: \n";
        boost::property_tree::json_parser::write_json(ss, m_config);
        std::cerr << ss.str() << "\n";
        throw issue;
    }
}
