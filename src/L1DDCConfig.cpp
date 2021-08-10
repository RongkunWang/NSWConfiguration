#include "NSWConfiguration/L1DDCConfig.h"

#include <string>
#include <boost/optional.hpp>

#include "NSWConfiguration/Utility.h"

nsw::L1DDCConfig::L1DDCConfig(const boost::property_tree::ptree& config) : m_gbtx0(){

    ERS_DEBUG(2, "Constructor for nsw::L1DDCConfig::L1DDCConfig\n"); 

    // Make configuration class for GBTx0
    boost::property_tree::ptree gbtx0Config = config.get_child("gbtx0");
    // Printout configuration tree for debugging
    ERS_DEBUG(2, nsw::dumpTree(gbtx0Config));

    // Import configuration tree to GBTx0 configuration
    m_gbtx0.setConfigFromPTree(gbtx0Config);
    ERS_DEBUG(2, "Done setting setConfigFromPTree\n"); 

    try {
        m_portToGBTx   = config.get<std::size_t>("portToGBTx");
        m_portFromGBTx = config.get<std::size_t>("portFromGBTx");
        m_elinkId      = config.get<std::size_t>("elinkId");
        m_opcServerIp  = config.get<std::string>("OpcServerIp");
    }
    catch (const boost::property_tree::ptree_bad_path&){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "Error getting L1DDC info from JSON. Missing entries may be portToGBTx, portFromGBTx, elinkId");
        ers::error(issue);
        throw issue;
    }

    // TODO: Future configuration for GBTx1, GBTx2 can go here

    ERS_DEBUG(2, "Constructor for nsw::L1DDCConfig::L1DDCConfig DONE\n"); 

}


std::vector<std::uint8_t> nsw::L1DDCConfig::getGbtxBytestream(std::size_t gbtxId)const {
    // Return the configuration bytestream for a given GBTx
    ERS_LOG("get GBTx bytestream for gbtxId="<<gbtxId<<"\n"); 

    if (gbtxId==0) {
        return m_gbtx0.configAsVector();
    }
    
    nsw::NSWL1DDCIssue issue(ERS_HERE, "Non-zero gbtx's not implemented yet");
    ers::error(issue);
    throw issue;
}

