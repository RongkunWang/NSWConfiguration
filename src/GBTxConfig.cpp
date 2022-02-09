#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/GBTxRegisterMap.h"
#include "NSWConfiguration/Types.h"
#include "NSWConfiguration/Utility.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <fmt/core.h>

namespace pt = boost::property_tree;

nsw::GBTxConfig::GBTxConfig() :
    m_gbtxType("none"),
    m_active(false),
    m_registerMaps(compiledGbtxRegisterMap())
{
}

void nsw::GBTxConfig::setRegister(const std::size_t r, const std::uint8_t value, const std::size_t shift){
    // update register value
    constexpr std::size_t SIZE_GBTX_REGISTER = 8;
    if (shift>=SIZE_GBTX_REGISTER){
        nsw::NSWBoardIssue issue(ERS_HERE, "Tried to shift GBTx register too far: " + std::to_string(shift));
        ers::error(issue);
        throw issue;
    }
    if (r>=NUM_GBTX_WRITABLE_REGISTERS){
        nsw::NSWBoardIssue issue(ERS_HERE, "Tried to configure GBTx register number above the maximum: " + std::to_string(r));
        ers::error(issue);
        throw issue;
    }

    const std::uint8_t shifted=value<<shift;
    ERS_DEBUG(5, ">>>> Setting "<<r<<" to 0x"<<std::hex<<static_cast<unsigned int>(value)<<std::dec<<" with shift "<<shift);
    ERS_DEBUG(5, ">>>> Initial: 0x"<<std::hex<<static_cast<unsigned int>(m_config.at(r))<<std::dec);
    ERS_DEBUG(5, ">>>> Shifted: 0x"<<std::hex<<static_cast<unsigned int>(shifted)<<std::dec);
    m_config.at(r) |= shifted;
    ERS_DEBUG(5, ">>>> Initial: 0x"<<std::hex<<static_cast<unsigned int>(m_config.at(r))<<std::dec);
}

void nsw::GBTxConfig::reset(const std::string& name, const std::size_t value){
    // Set the registers corresponding to name to 0x00
    // This is dangerous, because it may errase bits used for other settings. It should only be used carefully
    // Then, set the value of the register as normal
    const gbtx::regMap rg = m_registerMaps.at(name);
    ERS_DEBUG(2, ">> Deleting "<<rg.name<<" full byte to 0x00");
    for (const auto& reg : rg.registers) {
        m_config.at(reg) = 0x00;
    }
    set(name,value);
}


void nsw::GBTxConfig::set(const std::string& name, const std::size_t value){
    // Sets the value in the config object for a given setting
    if (m_registerMaps.find(name)==m_registerMaps.end()){
        nsw::NSWBoardIssue issue(ERS_HERE, "Didn't find GBTx setting name " + name);
        ers::error(issue);
        throw issue;
    }
    gbtx::regMap rg = m_registerMaps.at(name);
    ERS_DEBUG(2, ">> Setting "<<rg.name<<" to "<<value);
    // number of registers to set
    const auto nReg = rg.registers.size();
    for (std::size_t i=0; i<nReg; i++){
        ERS_DEBUG(5, ">> Register value: "<<rg.registers.at(i));
        ERS_DEBUG(5, ">> Shift value: "<<rg.shifts.at(i));
        setRegister(rg.registers.at(i), value, rg.shifts.at(i));
    }
}

void nsw::GBTxConfig::setConfigFromPTree(const pt::ptree& pt){
    // update registers with ptree info
    ERS_DEBUG(2, ">> About to loop over ptree");
    for (const auto& it: pt){
        ERS_DEBUG(2, "Setting from ptree: "<<it.first<<":"<<it.second.data());
        set(it.first,it.second.get_value<int>());
    }
    ERS_DEBUG(2, "DONE GBTxConfig::setConfigFromPTree\n");
}

void nsw::GBTxConfig::setConfigFromFile(const std::string& iPath){
    // update registers with info from file
    std::ifstream ifile (iPath.c_str());
    if (ifile.is_open()){
        std::string line;
        std::size_t iLine = 0;
        while ( getline (ifile,line) ) {
            if (iLine>=nsw::NUM_GBTX_WRITABLE_REGISTERS){
                nsw::NSWBoardIssue issue(ERS_HERE, "Text config file is too long!");
                ers::error(issue);
                throw issue;
            }
            m_config.at(iLine) = std::stoi(line,0,16);
            iLine+=1;
        }
        ifile.close();
    }
    else{
        nsw::NSWBoardIssue issue(ERS_HERE, "Issue opening GBTx config file");
        ers::error(issue);
        throw issue;
    }
}

std::vector<std::uint8_t> nsw::GBTxConfig::configAsVector() const {
    // Return configuration as vector
    ERS_DEBUG(2, "Generating config as vector\n");
    std::vector<std::uint8_t> ret(std::begin(m_config), std::end(m_config));
    return ret;
}

std::string nsw::GBTxConfig::getPrintableConfig() const {
    // Return nicely formatted string with configuration details
    std::stringstream ss;
    ss<<"\nreg |";
    for (std::size_t i=0; i<NUM_GBTX_WRITABLE_REGISTERS; i++){
        if (i%16==0) {
            ss<<std::dec<<'\n'<<i;
            if (i<10) {ss<<" ";}
            if (i<100) {ss<<" ";}
            ss<<" | ";
        }
        if (m_config.at(i)<0x10) {ss<<"0";}
        ss<<std::hex<<static_cast<unsigned int>(m_config.at(i))<<std::dec<<" ";
    }
    ss<<std::endl;
    return ss.str();
}

void nsw::GBTxConfig::setConfig(const std::vector<uint8_t>& config) {
    // Update internal m_config values to match config
    if (m_config.size()!=config.size()){
        nsw::NSWBoardIssue issue(ERS_HERE, "Tried to set GBTx internal config with vector of wrong size");
        ers::error(issue);
        throw issue;
    }
    for (std::size_t i=0; i<m_config.size(); i++){
       m_config.at(i) = config.at(i);
    }
}

std::vector<uint8_t> nsw::GBTxConfig::getPhasesVector(const std::vector<uint8_t>& config) {
    // Parse full GBTx config, return phases
    // See page 199 and bits named "phaseSelectOutGroup*"
    // Each channel phase is 4 bits long, so two channels per byte
    // Register range is (Groups 0-6: 399-426, not used) (Groups 0-4: 399-418, used)
    // The group and channel is arranged sequentially:
        // for group 0, register 399: [ch1b0,ch1b1,ch1b2,ch1b3, ch0b0,ch0b1,ch0b2,ch0b3 ]
        // for group 0, register 400: [ch3b0,ch3b1,ch3b2,ch3b3, ch2b0,ch2b1,ch2b2,ch2b3 ]
        // for group 0, register 401: [ch5b0,ch5b1,ch5b2,ch5b3, ch4b0,ch4b1,ch4b2,ch4b3 ]
        // for group 0, register 402: [ch7b0,ch7b1,ch7b2,ch7b3, ch6b0,ch6b1,ch6b2,ch6b3 ]
        // For group 1, register 403: ...
    constexpr std::size_t minPhaseRegister = 399;
    constexpr std::size_t maxPhaseRegister = 426+1;
    constexpr std::size_t ecPhaseRegister  = 398;
    std::vector<uint8_t> ret;
    for (std::size_t i=minPhaseRegister; i<maxPhaseRegister; i++){
        const uint8_t chanA = config.at(i)%16; // last 4 bits
        const uint8_t chanB = config.at(i)/16; // first 4 bits
        ret.push_back(chanA); // check the order
        ret.push_back(chanB);
    }
    // insert EC phase at the end
    ret.push_back(config.at(ecPhaseRegister)%16);

    // print table of phases
    std::stringstream ss;
    ss<<"\n[GBTx Phases Read Back]\n";
    ss<<"Channel   0   1   2   3   4   5   6   7 \n";
    for (std::size_t i=0; i<ret.size()-1; i++){
        if (i%8==0) ss<<"Group "<<i/8<<" ";
        ss<<"| ";
        ss << std::hex << static_cast<int>(ret.at(i)) << std::dec << " ";
        if ((i-7)%8==0) ss<<'\n';
    }
    ss<<"EC: "<< std::hex << static_cast<int>(ret.back()) << std::dec;
    ERS_LOG(ss.str());

    return ret;
}

pt::ptree nsw::GBTxConfig::getPhasesTree(const std::vector<uint8_t>& config) {
    // Parse full GBTx config, return phases as tree
    pt::ptree ret;
    const std::vector<uint8_t> phaseList = getPhasesVector(config);
    for (std::size_t i=0; i<phaseList.size()-1; i++){
        const int group = i/8;
        const int chan = i%8;
        const std::string setting = fmt::format("paPhaseSelectGroup{}Channel{}",group,chan);
        ret.push_back(pt::ptree::value_type(setting,std::to_string(phaseList.at(i))));
    }
    // The final phase is the EC phase
    ret.push_back(pt::ptree::value_type("paPhaseSelectEC",std::to_string(phaseList.at(phaseList.size()-1))));
    ERS_DEBUG(5, ">>>> Tree of trained phases: "<<nsw::dumpTree(ret));
    return ret;
}

void nsw::GBTxConfig::debugPrintRegisterMaps(){
    ERS_LOG(getPrintableConfig()<<std::endl);
}

void nsw::GBTxConfig::setResetChannelsOn(){
    // Set input reset channels on
    //     Each is 8 bytes long for each channel in the group
    //     group0 = [84, 85, 86]  A, B, C
    //     group1 = [108,109,110] A, B, C
    //     group2 = [132,133,134] A, B, C
    //     group3 = [156,157,158] A, B, C
    //     group4 = [180,181,182] A, B, C
    // Todo: There should be a configured setting to determine which channels are used, via a mask
    // set("paResetGroup0",0xFF);
    // This mode is not recommended for environments were SEUs are a concern.
    if (isType("mmg") || isType("pfeb") || isType("sfeb") || isType("rim")){
        reset("paResetEC",0x01);
        reset("paResetGroup0",0x00);
        reset("paResetGroup1",0xFF);
        reset("paResetGroup2",0xFF);
        reset("paResetGroup3",0xFF);
        reset("paResetGroup4",0xFF);
        reset("paResetGroup5",0x00); // just in case
        reset("paResetGroup6",0x00); // just in case
    }
    else{
        nsw::InvalidGBTxTrainingType issue(ERS_HERE,m_gbtxType);
        ers::error(issue);
        throw issue;
    }
}

void nsw::GBTxConfig::setResetChannelsOff(){
    // Set input reset channels off
    // Todo: There should be a configured setting to determine which channels are used, via a mask
    // reset("paResetGroup0",0x00);
    if (isType("mmg")){
        reset("paResetEC",0x00);
        reset("paResetGroup0",0x00);
        reset("paResetGroup1",0x01); // in conf_l1_train_320_191.sh, this is kept 0x01
        reset("paResetGroup2",0x00);
        reset("paResetGroup3",0x00);
        reset("paResetGroup4",0x00);
        reset("paResetGroup5",0x00); // just in case
        reset("paResetGroup6",0x00); // just in case
    }
    else if (isType("pfeb") || isType("sfeb") || isType("rim")){
        reset("paResetEC",0x00);
        reset("paResetGroup0",0x00);
        reset("paResetGroup1",0x00);
        reset("paResetGroup2",0x00);
        reset("paResetGroup3",0x00);
        reset("paResetGroup4",0x00);
        reset("paResetGroup5",0x00);
        reset("paResetGroup6",0x00);
    }
    else{
        nsw::InvalidGBTxTrainingType issue(ERS_HERE,m_gbtxType);
        ers::error(issue);
        throw issue;
    }
}

void nsw::GBTxConfig::setEcTrainingRegisters(const bool on){
    // Train the EC e-link phase aligner
    reset("paTrainEC",static_cast<std::uint8_t>(on));
}

void nsw::GBTxConfig::setTrainingRegistersOn(){
    // Train the apropriate e-link phase aligners
    //     1) Reset phase alignment
    //     2) Set phase alignment tracking mode
    //     3) Turn on phase alignment
    // Channel reset
    // Todo: There should be a configured setting to determine which channels are used, via a mask
    // Todo: Currently, also calls paReset. Needs validation

    // Reset phase alignment (expected to last 100ns)
    // reset("paResetGroup0",0xFF);
    // reset("paResetGroup1",0xFF);
    // reset("paResetGroup2",0xFF);
    // reset("paResetGroup3",0xFF);
    // reset("paResetGroup4",0xFF);

    // Set the paMode
    // This is a 2-bit setting with triplet redundancy
    // <!!Only part of the register is used!!>
    // Register 62[5:0] = [paModeC[1:0], paModeB[1:0], paModeA[1:0]]
    // Meaning
    //     00 Static phase selection
    //     01 Training mode
    //     10 Automatic phase tracking
    // Note: It is possible that this setting should be configured in the DB
    reset("paMode",0b1);

    // Set the phase alignment training
    if (isType("mmg") || isType("pfeb") || isType("sfeb") || isType("rim")){
        reset("paTrainGroup0",0xFF);
        reset("paTrainGroup1",0xFF);
        reset("paTrainGroup2",0xFF);
        reset("paTrainGroup3",0xFF);
        reset("paTrainGroup4",0xFF);
        reset("paTrainGroup5",0x00);
        reset("paTrainGroup6",0x00);
    }
    else{
        nsw::InvalidGBTxTrainingType issue(ERS_HERE,m_gbtxType);
        ers::error(issue);
        throw issue;
    }
}

void nsw::GBTxConfig::setTrainingRegistersOff(){
    // Turn off phase alignment
    // Todo: There should be a configured setting to determine which channels are used, via a mask
    if (isType("mmg") || isType("pfeb") || isType("sfeb") || isType("rim")){
        reset("paTrainGroup0",0x00);
        reset("paTrainGroup1",0x00);
        reset("paTrainGroup2",0x00);
        reset("paTrainGroup3",0x00);
        reset("paTrainGroup4",0x00);
        reset("paTrainGroup5",0x00);
        reset("paTrainGroup6",0x00);
    }
    else{
        nsw::InvalidGBTxTrainingType issue(ERS_HERE,m_gbtxType);
        ers::error(issue);
        throw issue;
    }
}
