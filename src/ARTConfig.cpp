#include "NSWConfiguration/ARTConfig.h"

#include <utility>
#include <iostream>

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

nsw::ARTConfig::ARTConfig(const ptree& config):
    m_config(config),
    core(config.get_child(ART_CORE_NAME), ART_CORE_NAME, ART_CORE_REGISTERS),
    ps  (config.get_child(ART_PS_NAME),   ART_PS_NAME,   ART_PS_REGISTERS)
{
    // std::cout << "ART Constructor!" << std::endl;
}

bool nsw::ARTConfig::SkipConfigure() const {
    return m_config.get("SkipConfigure", false);
}

bool nsw::ARTConfig::MustConfigure() const {
    return m_config.get("MustConfigure", true);
}

std::string nsw::ARTConfig::getOpcServerIp_TP() const {
    return m_config.get<std::string>("OpcServerIp_TP");
}

std::string nsw::ARTConfig::getOpcNodeId_TP() const {
    return m_config.get<std::string>("OpcNodeId_TP");
}

bool nsw::ARTConfig::failsafe() const {
    return static_cast<bool>(m_config.get<int>("failsafe"));
}

std::size_t nsw::ARTConfig::TP_GBTxAlignmentBit() const {
    return m_config.get<std::size_t>("TP_GBTxAlignmentBit");
}

bool nsw::ARTConfig::TP_GBTxAlignmentSkip() const {
    return static_cast<bool>(m_config.get<int>("TP_GBTxAlignmentSkip"));
}

std::vector<uint> nsw::ARTConfig::TP_GBTxAlignmentCommonPhases() const {
    std::vector<uint> phases = {};
    for (const auto& ph : m_config.get_child("TP_GBTxAlignmentCommonPhases"))
        phases.push_back(ph.second.get_value<uint>());
    return phases;
}

std::string nsw::ARTConfig::TP_GBTxAlignmentPhase() const {
    return m_config.get<std::string>("TP_GBTxAlignmentPhase");
}

std::vector<uint> nsw::ARTConfig::TP_GBTxAlignmentPhasesToTest() const {
    std::vector<uint> phases = {};
    for (auto ph: TP_GBTxAlignmentCommonPhases())
        phases.push_back(ph);
    for (uint ph = 0; ph < NPhase(); ph++)
        phases.push_back(ph);
    return phases;
}

int nsw::ARTConfig::TP_GBTxAlignmentSleepTime() const {
    return m_config.get<int>("TP_GBTxAlignmentSleepTime");
}

bool nsw::ARTConfig::IsAlignedWithTP(const std::vector<uint8_t>& vec) const {
    // bit of interest
    const auto boi = TP_GBTxAlignmentBit();
    if (boi >= NPhase())
        throw std::runtime_error("Alignment bit is greater than or equal to " + std::to_string(NPhase()) + ": " + std::to_string(boi));
    if (vec.size() != 4)
        throw std::runtime_error("Need a vector of bytes of size=4, but got size = " + std::to_string(vec.size()));

    uint32_t reg32 = (static_cast<uint32_t>(vec.at(0)) << 0*nsw::NUM_BITS_IN_BYTE) +
                     (static_cast<uint32_t>(vec.at(1)) << 1*nsw::NUM_BITS_IN_BYTE) +
                     (static_cast<uint32_t>(vec.at(2)) << 2*nsw::NUM_BITS_IN_BYTE) +
                     (static_cast<uint32_t>(vec.at(3)) << 3*nsw::NUM_BITS_IN_BYTE);
    return reg32 & static_cast<uint32_t>(pow(2, boi));
}

uint8_t nsw::ARTConfig::BcidFromTp(const std::vector<uint8_t>& vec) const {
    const auto boi = TP_GBTxAlignmentBit();
    constexpr size_t num_bytes_for_32fibers_and_4bits_per_fiber = 16;
    if (vec.size() != num_bytes_for_32fibers_and_4bits_per_fiber) {
        throw std::runtime_error("Need a vector of bytes of size=16, but got size = " + std::to_string(vec.size()));
    }
    uint8_t bcid = 0;
    constexpr uint8_t fourLSB = 0x0f;
    const auto byte = vec.at(boi / 2);
    if (boi % 2 == 0) {
        bcid = (byte >> 0) & fourLSB;
    } else {
        bcid = (byte >> 4) & fourLSB;
    }
    return bcid;
}

std::string nsw::ARTConfig::PhaseToString(uint phase) const {
    std::stringstream zeropad;
    zeropad << std::hex << std::setfill('0') << std::setw(2) << phase;
    return zeropad.str();
}

bool nsw::ARTConfig::IsMyTP(const std::string& ServerIp, const std::string& NodeId) const {
    return ServerIp==getOpcServerIp_TP() && NodeId==getOpcNodeId_TP();
}
