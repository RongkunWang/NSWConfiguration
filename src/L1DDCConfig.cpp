#include "NSWConfiguration/L1DDCConfig.h"

#include <string>
#include <boost/optional.hpp>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/Utility.h"

nsw::L1DDCConfig::L1DDCConfig(const boost::property_tree::ptree& config, const bool phase2) :
    m_gbtx0(nsw::GBTxConfig::Mode::IC),
    m_gbtx1([&config, &phase2]() {
        if (not phase2) {
            return nsw::GBTxConfig(nsw::GBTxConfig::Mode::I2C);
        }
        if (config.get("boardType", "none") != "mmg") {
            return nsw::GBTxConfig(nsw::GBTxConfig::Mode::I2C);
        }
        return nsw::GBTxConfig(nsw::GBTxConfig::Mode::IC);
    }())
{
    ERS_DEBUG(2, "Constructor for nsw::L1DDCConfig::L1DDCConfig\n");

    try {
        m_portToGBTx     = config.get<std::size_t>("portToGBTx");
        m_portFromGBTx   = config.get<std::size_t>("portFromGBTx");
        m_elinkId        = config.get<std::size_t>("elinkId");
        m_felixServerIp  = config.get<std::string>("FelixServerIp");
        m_opcServerIp    = config.get<std::string>("OpcServerIp");
        m_opcNodeId      = config.get<std::string>("OpcNodeId");
    }
    catch (const boost::property_tree::ptree_bad_path&){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "Error getting L1DDC info from JSON. Missing entries may be portToGBTx, portFromGBTx, elinkId");
        ers::error(issue);
        throw issue;
    }

    m_name = fmt::format("L1DDC-{}:{}/{}/{}/{}",m_boardType,m_felixServerIp,m_portToGBTx,m_portFromGBTx,m_elinkId);

    // Optional Calibration configuration passed in ptree
    m_trainGBTxPhaseAlignment = config.get("trainGBTxPhaseAlignment", false);
    m_trainGBTxPhaseWaitTime  = config.get("trainGBTxPhaseWaitTime", 0);
    m_configureGBTx0          = config.get("configureGBTx0",false);
    m_configureGBTx1          = config.get("configureGBTx1",false);
    m_i2cDelay                = config.get("i2cDelay",10000);
    m_i2cBlockSize            = config.get("i2cBlockSize",16);
    m_configOption            = config.get("configOption",0);
    ERS_DEBUG(2,"L1DDC read GBTx training settings (note, these may be default values): toggle = "<<m_trainGBTxPhaseAlignment<<" and wait time = "<<m_trainGBTxPhaseWaitTime);

    // TODO: Future configuration for GBTx1, GBTx2 can go here
    // Determine board type and init apropriate gbtx
    m_boardType = config.get("boardType", "none");
    initGBTx(config);

}

nsw::L1DDCConfig::L1DDCConfig(const nsw::GBTxSingleConfig& config) :
    m_gbtx0(nsw::GBTxConfig::Mode::IC),
    m_gbtx1(nsw::GBTxConfig::Mode::I2C),
    m_portToGBTx(config.portToGBTx),
    m_boardType(config.boardType),
    m_portFromGBTx(config.portFromGBTx),
    m_elinkId(config.elinkId),
    m_trainGBTxPhaseAlignment(config.trainGBTxPhaseAlignment),
    m_trainGBTxPhaseWaitTime(config.trainGBTxPhaseWaitTime),
    m_felixServerIp(config.felixServerIp),
    m_opcNodeId(config.opcNodeId),
    m_opcServerIp(config.opcServerIp),
    m_name(fmt::format("L1DDC:{}/{}/{}/{}",config.felixServerIp,config.portToGBTx,config.portFromGBTx,config.elinkId)) {
    // This constructor is used by configure_gbtx to load the configuration from an XML file
    // It only configures gbtx0

    if (m_boardType=="mmg"||m_boardType=="pfeb"||m_boardType=="rim"){
        m_gbtx0.setType(m_boardType);
        m_gbtx0.setConfigFromFile(config.iPath);
    }
    else{
        nsw::NSWL1DDCIssue issue(ERS_HERE, fmt::format("Invalid L1DDC board type: {}. Check configuration for L1DDC",m_boardType));
        ers::error(issue);
        throw issue;
    }

}

void nsw::L1DDCConfig::initGBTx(const boost::property_tree::ptree& config){
    // Set up GBTx objects
    // Configuration and training behavior depend on which board type
    if (m_boardType=="mmg"){
        ERS_DEBUG(4, fmt::format("Configuring {} as MMG L1DDC with GBTx Configurations:",m_name));
        const boost::property_tree::ptree gbtx0Config = config.get_child("mmg_gbtx0");
        ERS_DEBUG(5, nsw::dumpTree(gbtx0Config));
        m_gbtx0.setType("mmg0");
        m_gbtx0.setConfigFromPTree(gbtx0Config);
    }
    else if (m_boardType=="rim"){
        ERS_DEBUG(2, fmt::format("Configuring {} as RIM L1DDC with GBTx Configurations:",m_name));
        const boost::property_tree::ptree gbtx0Config = config.get_child("rim_gbtx0");
        ERS_DEBUG(5, nsw::dumpTree(gbtx0Config));
        m_gbtx0.setType("rim0");
        m_gbtx0.setConfigFromPTree(gbtx0Config);
    }
    else if (m_boardType=="sfeb"){
        ERS_DEBUG(2, fmt::format("Configuring {} as sfeb L1DDC with GBTx Configurations:",m_name));
        const boost::property_tree::ptree gbtx0Config = config.get_child("sfeb_gbtx0");
        m_gbtx0.setType("sfeb0");
        m_gbtx0.setConfigFromPTree(gbtx0Config);
        // Optional configuration for GBTx1
        if (m_configureGBTx1){
            ERS_DEBUG(2, fmt::format("Configuring {} as sfeb L1DDC with GBTx1 Configurations:",m_name));
            const boost::property_tree::ptree gbtx1Config = config.get_child("sfeb_gbtx1");
            m_gbtx1.setType("sfeb1");
            m_gbtx1.setConfigFromPTree(gbtx1Config);
        }
    }
    else if (m_boardType=="pfeb"){
        ERS_DEBUG(2, fmt::format("Configuring {} as pfeb L1DDC with GBTx Configurations:",m_name));
        const boost::property_tree::ptree gbtx0Config = config.get_child("pfeb_gbtx0");
        m_gbtx0.setType("pfeb0");
        m_gbtx0.setConfigFromPTree(gbtx0Config);
        // Optional configuration for GBTx1
        // The difference between pfeb and sfeb is that the GBTx1 on the pfeb is not connected to VTRX.
        // However it should still be configured to prevent noise
        if (m_configureGBTx1){
            ERS_DEBUG(2, fmt::format("Configuring {} as pfeb L1DDC with GBTx1 Configurations:",m_name));
            const boost::property_tree::ptree gbtx1Config = config.get_child("pfeb_gbtx1");
            m_gbtx1.setType("pfeb1");
            m_gbtx1.setConfigFromPTree(gbtx1Config);
        }
    }
    else if (m_boardType=="rim"){
        // Not implemented yet
        nsw::NSWL1DDCIssue issue(ERS_HERE, "RIM L1DDC not implemented yet");
        ers::error(issue);
        throw issue;
    }
    else{
        nsw::NSWL1DDCIssue issue(ERS_HERE, fmt::format("Invalid L1DDC board type: {}. Check configuration for L1DDC",m_boardType));
        ers::error(issue);
        throw issue;
    }
}

std::vector<std::uint8_t> nsw::L1DDCConfig::getGBTxBytestream(const std::size_t gbtxId) const {
    // Return the configuration bytestream for a given GBTx
    ERS_LOG("get GBTx bytestream for gbtxId="<<gbtxId<<"\n");

    if (gbtxId==0||gbtxId==1) {
        return m_gbtx0.configAsVector();
    }

    nsw::NSWL1DDCIssue issue(ERS_HERE, "GBTx's with number not 0,1 not implemented yet");
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

const nsw::GBTxConfig& nsw::L1DDCConfig::getGBTxConfig(const std::size_t gbtxId) const
{
    if (gbtxId == 0) {
        return m_gbtx0;
    }
    if (gbtxId == 1) {
        return m_gbtx1;
    }
    nsw::NSWL1DDCIssue issue(ERS_HERE, fmt::format("No support for GBTx{}", gbtxId));
    ers::error(issue);
    throw issue;
}
