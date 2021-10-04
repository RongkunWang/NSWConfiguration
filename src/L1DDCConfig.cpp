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
        m_felixServerIp  = config.get<std::string>("FelixServerIp");
    }
    catch (const boost::property_tree::ptree_bad_path&){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "Error getting L1DDC info from JSON. Missing entries may be portToGBTx, portFromGBTx, elinkId");
        ers::error(issue);
        throw issue;
    }

    m_name = fmt::format("L1DDC:{}/{}/{}/{}",m_felixServerIp,m_portToGBTx,m_portFromGBTx,m_elinkId);

    // Optional Calibration configuration passed in ptree
    m_trainGBTxPhaseAlignment = config.get("trainGBTxPhaseAlignment", false);
    m_trainGBTxPhaseWaitTime  = config.get("trainGBTxPhaseWaitTime", 1);
    ERS_DEBUG(2,"L1DDC read GBTx training settings (note, these may be default values): toggle = "<<m_trainGBTxPhaseAlignment<<" and wait time = "<<m_trainGBTxPhaseWaitTime);

    // TODO: Future configuration for GBTx1, GBTx2 can go here

}

nsw::L1DDCConfig::L1DDCConfig(const nsw::GBTxSingleConfig& config) :
    m_gbtx0(),
    // to be configured
    m_portToGBTx(config.portToGBTx),
    m_portFromGBTx(config.portFromGBTx),
    m_elinkId(config.elinkId),
    m_trainGBTxPhaseAlignment(config.trainGBTxPhaseAlignment),
    m_trainGBTxPhaseWaitTime(config.trainGBTxPhaseWaitTime),
    m_felixServerIp(config.felixServerIp),
    m_name(fmt::format("L1DDC:{}/{}/{}/{}",config.felixServerIp,config.portToGBTx,config.portFromGBTx,config.elinkId)) {
    m_gbtx0.setConfigFromFile(config.iPath);
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


void nsw::L1DDCConfig::trainGbtxsOn(){
    // Update config objects in each GBTx instance to start training e-links
    // The config can subsequently be read using getGbtxBytestream
    ERS_LOG("Set registers to start training GBTx phase alignment");

    if (!m_trainGBTxPhaseAlignment){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "trainGBTxPhaseAlignment has not been set to true in the configuration. Are you sure you want to train the phase alignment?");
        ers::error(issue);
    }

    m_gbtx0.setResetChannelsOn();
    m_gbtx0.setTrainingRegistersOn();
    // Todo: gbtx 1, 2
}

void nsw::L1DDCConfig::trainGbtxsOff(){
    // Update config objects in each GBTx instance to stop training e-links
    // The config can subsequently be read using getGbtxBytestream
    ERS_LOG("Set registers to stop training GBTx phase alignment");

    if (!m_trainGBTxPhaseAlignment){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "trainGBTxPhaseAlignment has not been set to true in the configuration. Are you sure you want to train the phase alignment?");
        ers::error(issue);
    }

    m_gbtx0.setResetChannelsOff();
    m_gbtx0.setTrainingRegistersOff();
    // Todo: gbtx 1, 2
}
