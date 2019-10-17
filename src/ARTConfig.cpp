#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::ARTConfig::ARTConfig(ptree config):
    m_config(config),
    core(config.get_child(ART_CORE_NAME), ART_CORE_NAME, ART_CORE_REGISTERS),
    ps  (config.get_child(ART_PS_NAME),   ART_PS_NAME,   ART_PS_REGISTERS)
{
    // std::cout << "ART Constructor!" << std::endl;
}

int nsw::ARTConfig::register0_test_00() { 
    return m_config.get_child("register0").get<int>("test_00");
}

int nsw::ARTConfig::art_core_cfg_deser_flagmask() {
    return core.getRegisterValue("03", "cfg_deser_flagmask");
}

std::string nsw::ARTConfig::getOpcServerIp_TP() {
    return m_config.get<std::string>("OpcServerIp_TP");
}

std::string nsw::ARTConfig::getOpcNodeId_TP() {
    return m_config.get<std::string>("OpcNodeId_TP");
}

bool nsw::ARTConfig::failsafe() {
    return (bool)(m_config.get<int>("failsafe"));
}

int nsw::ARTConfig::TP_GBTxAlignmentBit() {
    return m_config.get<int>("TP_GBTxAlignmentBit");
}

bool nsw::ARTConfig::TP_GBTxAlignmentSkip() {
    return (bool)(m_config.get<int>("TP_GBTxAlignmentSkip"));
}

std::string nsw::ARTConfig::TP_GBTxAlignmentPhase() {
    return m_config.get<std::string>("TP_GBTxAlignmentPhase");
}

bool nsw::ARTConfig::IsAlignedWithTP(std::vector<uint8_t> vec) {
    // bit of interest
    int boi = TP_GBTxAlignmentBit();
    if (boi < 0)
        throw std::runtime_error("Alignment bit is less than 0: " + std::to_string(boi));
    if (boi >= (int)(NPhase()))
        throw std::runtime_error("Alignment bit is greater than or equal to " + std::to_string(NPhase()) + ": " + std::to_string(boi));
    if (vec.size() != 4)
        throw std::runtime_error("Need a vector of bytes of size=4, but got size = " + std::to_string(vec.size()));
    uint32_t reg32 = ((uint32_t)(vec[3]) <<  0) +
                     ((uint32_t)(vec[2]) <<  8) +
                     ((uint32_t)(vec[1]) << 16) +
                     ((uint32_t)(vec[0]) << 24);
    return reg32 & (uint32_t)(pow(2, boi));
}

std::string nsw::ARTConfig::PhaseToString(uint phase) {
    std::stringstream zeropad;
    zeropad << std::hex << std::setfill('0') << std::setw(2) << phase;
    return zeropad.str();
}
