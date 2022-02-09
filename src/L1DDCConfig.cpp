#include "NSWConfiguration/L1DDCConfig.h"

#include <string>
#include <boost/optional.hpp>

#include "NSWConfiguration/Utility.h"

nsw::L1DDCConfig::L1DDCConfig(const boost::property_tree::ptree& config) {

    ERS_DEBUG(2, "Constructor for nsw::L1DDCConfig::L1DDCConfig\n");

    try {
        m_portToGBTx     = config.get<std::size_t>("portToGBTx");
        m_portFromGBTx   = config.get<std::size_t>("portFromGBTx");
        m_elinkId        = config.get<std::size_t>("elinkId");
        m_felixServerIp  = config.get<std::string>("FelixServerIp");
        m_opcServerIp    = config.get<std::string>("OpcServerIp");
        m_opcNodeId      = config.get<std::string>("OpcNodeId");
        m_boardType      = config.get<std::string>("boardType");
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
    m_i2cDelay                = config.get("i2cDelay",10000);
    m_i2cBlockSize            = config.get("i2cBlockSize",16);
    m_configOption            = config.get("configOption",0);
    m_GBTxPhaseOutputDBPath   = config.get("GBTxPhaseOutputDBPath","");
    m_nodeName                = config.get("nodeName","defaultL1ddcNode");
    ERS_DEBUG(2,"L1DDC read GBTx training settings (note, these may be default values): toggle = "<<m_trainGBTxPhaseAlignment<<" and wait time = "<<m_trainGBTxPhaseWaitTime);

    // Determine board type and init apropriate gbtx
    initGBTxs(config);
    ERS_DEBUG(4, "Finished initializing L1DDC");

}

nsw::L1DDCConfig::L1DDCConfig(const nsw::GBTxSingleConfig& config) :
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
        m_GBTxContainers.emplace_back(GBTxConfig(),true,true);
        getGBTx(0).setType(m_boardType);
        getGBTx(0).setConfigFromFile(config.iPath);
        getGBTx(0).setActive();
    }
    else{
        nsw::NSWL1DDCIssue issue(ERS_HERE, fmt::format("Invalid L1DDC board type: {}. Check configuration for L1DDC",m_boardType));
        ers::error(issue);
        throw issue;
    }

}

void nsw::L1DDCConfig::initGBTx(const boost::property_tree::ptree& config, const std::size_t gbtxId){
    // Configure individual GBTx of type and gbtxId using config
    ERS_DEBUG(4, fmt::format("Setting up GBTx on L1DDC {} as {} type using GBTx{} configuration",m_name,m_boardType,gbtxId));
    std::string type = fmt::format("{}{}",m_boardType,gbtxId); // format is mmg0, mmg1, mmg2, pfeb0, sfeb1, etc
    std::string configTreeName = fmt::format("{}_gbtx{}",m_boardType,gbtxId); // format is sfeb_gbtx0, mmg_gbtx0 etc
    ERS_DEBUG(4, fmt::format("Using configTreeName: {}",configTreeName));
    getGBTx(gbtxId).setType(type);
    ERS_DEBUG(4, fmt::format("Setting GBTx{} from config ptree",gbtxId));
    getGBTx(gbtxId).setConfigFromPTree(config.get_child(configTreeName));
    getGBTx(gbtxId).setActive();
    ERS_DEBUG(4, fmt::format("Finished setting up GBTx on L1DDC {} as {} type using GBTx{} configuration",m_name,m_boardType,gbtxId));
}

void nsw::L1DDCConfig::initGBTxs(const boost::property_tree::ptree& config){
    // Set up GBTx objects
    // Configuration and training behavior depend on which board type
    std::size_t  nGBTx{};
    if (m_boardType=="mmg")       nGBTx = l1ddc::MMG_L1DDC_NUMBER_GBTx;
    else if (m_boardType=="rim")  nGBTx = l1ddc::RIM_L1DDC_NUMBER_GBTx;
    else if (m_boardType=="sfeb") nGBTx = l1ddc::SFEB_L1DDC_NUMBER_GBTx;
    else if (m_boardType=="pfeb") nGBTx = l1ddc::PFEB_L1DDC_NUMBER_GBTx;
    else{
        nsw::NSWL1DDCIssue issue(ERS_HERE, fmt::format("Invalid L1DDC board type: {}. Check configuration for L1DDC",m_boardType));
        ers::error(issue);
        throw issue;
    }

    for (std::size_t gbtxId=0; gbtxId<nGBTx; gbtxId++){
        ERS_DEBUG(2,fmt::format("Initializing GBTx{}",gbtxId));
        std::string configureGBTxSettingName = fmt::format("configureGBTx{}",gbtxId);
        std::string ecElinkTrainSettingName = fmt::format("ecElinkTrain{}",gbtxId);
        ERS_DEBUG(4,fmt::format("configureGBTxSettingName={}, ecElinkTrainSettingName={}",configureGBTxSettingName,ecElinkTrainSettingName));
        m_GBTxContainers.emplace_back(GBTxConfig(),config.get(configureGBTxSettingName,false),config.get(ecElinkTrainSettingName,false));
        ERS_DEBUG(4,"Done");
        if (!m_GBTxContainers.at(gbtxId).configureGBTx) continue;
        initGBTx(config,gbtxId);
    }
    ERS_DEBUG(4, "Finished initializing GBTxs");
}

std::vector<std::uint8_t> nsw::L1DDCConfig::getGBTxBytestream(const std::size_t gbtxId) const {
    // Return the configuration bytestream for a given GBTx
    ERS_LOG("Get GBTx bytestream for gbtxId="<<gbtxId<<"\n");

    std::vector<std::uint8_t> ret;
    if (gbtxId==0||gbtxId==1||gbtxId==2) {
        ret = getGBTx(gbtxId).configAsVector();
    }
    else{
        nsw::NSWL1DDCIssue issue(ERS_HERE, "Invalid GBTx number");
        ers::error(issue);
        throw issue;
    }

    return ret;
}


void nsw::L1DDCConfig::trainGbtxsOn(){
    // Update config objects in each GBTx instance to start training e-links
    // The config can subsequently be read using getGbtxBytestream
    ERS_LOG("Set registers to start training GBTx phase alignment");
    if (!m_trainGBTxPhaseAlignment){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "trainGBTxPhaseAlignment has not been set to true in the configuration. Are you sure you want to train the phase alignment?");
        ers::error(issue);
    }
    for (auto& container : m_GBTxContainers){
        if (!container.configureGBTx) continue;
        container.GBTx.setResetChannelsOn();
        container.GBTx.setTrainingRegistersOn();
        if (container.ecElinkTrain){
            container.GBTx.setEcTrainingRegisters(true);
        }
    }
}

void nsw::L1DDCConfig::trainGbtxsOff(){
    // Update config objects in each GBTx instance to stop training e-links
    // The config can subsequently be read using getGbtxBytestream
    ERS_LOG("Set registers to stop training GBTx phase alignment");
    if (!m_trainGBTxPhaseAlignment){
        nsw::NSWL1DDCIssue issue(ERS_HERE, "trainGBTxPhaseAlignment has not been set to true in the configuration. Are you sure you want to train the phase alignment?");
        ers::error(issue);
    }
    for (auto& container : m_GBTxContainers){
        if (!container.configureGBTx) continue;
        container.GBTx.setResetChannelsOff();
        container.GBTx.setTrainingRegistersOff();
        if (container.ecElinkTrain){
            container.GBTx.setEcTrainingRegisters(false);
        }
    }
}
