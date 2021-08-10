#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/GBTxRegisterMap.h"
#include "NSWConfiguration/Types.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

nsw::GBTxConfig::GBTxConfig() :
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
    if (r>=NUM_GBTX_REGISTERS){
        nsw::NSWBoardIssue issue(ERS_HERE, "Tried to configure GBTx register number above the maximum: " + std::to_string(r));
        ers::error(issue);
        throw issue;
    }

    const std::size_t shifted=value<<shift;
    ERS_DEBUG(5, ">>>> Setting "<<r<<" to 0x"<<std::hex<<static_cast<unsigned int>(value)<<std::dec<<" with shift "<<shift);
    ERS_DEBUG(5, ">>>> Initial:"<<m_config.at(r));
    ERS_DEBUG(5, ">>>> Shifted:"<<shifted);
    m_config.at(r) |= shifted;
}

void nsw::GBTxConfig::set(const std::string& name, std::size_t value){
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
        ERS_DEBUG(5, ">> Register value ");
        ERS_DEBUG(5, ">> "<<rg.registers.at(i));
        ERS_DEBUG(5, ">> Shift value ");
        ERS_DEBUG(5, ">> "<<rg.shifts.at(i));
        setRegister(rg.registers.at(i), value, rg.shifts.at(i));
    }
}

void nsw::GBTxConfig::setConfigFromPTree(const boost::property_tree::ptree& pt){
    // update registers with ptree info
    ERS_DEBUG(2, ">> About to loop over ptree");
    for (const auto& it: pt){
        ERS_DEBUG(2, "Setting from ptree: "<<it.first<<":"<<it.second.data());
        set(it.first,it.second.get_value<int>());
    }
    ERS_DEBUG(2, "DONE GBTxConfig::setConfigFromPTree\n");
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
    ss<<"reg |";
    for (std::size_t i=0; i<NUM_GBTX_REGISTERS; i++){
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

void nsw::GBTxConfig::debugPrintRegisterMaps(){
    ERS_LOG(getPrintableConfig()<<std::endl);
}