/// Pad Trigger Configuration class

#ifndef NSWCONFIGURATION_PADTRIGGERSCA_H_
#define NSWCONFIGURATION_PADTRIGGERSCA_H_

#include <vector>
#include <iostream>

#include "NSWConfiguration/SCAConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"

namespace nsw {

class PadTriggerSCAConfig: public SCAConfig {
 private:
    bool dummy;

 public:
    explicit PadTriggerSCAConfig(const boost::property_tree::ptree& config);
    ~PadTriggerSCAConfig() = default;

    void dump();
    std::string firmware() const;
    uint32_t firmware_dateword() const;
    bool ConfigRepeaters() const;
    bool ConfigVTTx() const;
    bool ConfigControlRegister() const;
    uint32_t ControlRegister() const;
    const I2cMasterConfig & getFpga() const {return m_padtriggerfpga;}
    int L1AReadoutLatency() const;
    int L1AReadoutNBC() const;
    int L1AReadoutNBCMode() const;
    int L1AReadoutEnable() const;
    int pFEBBCIDOffset() const;
    int StartIdleState() const;
    int OCREnable() const;
    int TTCCalib() const;
    int SelfTriggerEnable() const;
    int LatencyScanStart() const;
    int LatencyScanNBC() const;
    void SetL1AReadoutLatency(int val) {m_L1AReadoutLatency = val;}
    void SetL1AReadoutNBC(int val)     {m_L1AReadoutNBC     = val;}
    void SetL1AReadoutEnable(int val)  {m_L1AReadoutEnable  = val;}
    void SetpFEBBCIDOffset(int val)    {m_pFEBBCIDOffset    = val;}
    void SetStartIdleState(int val)    {m_StartIdleState    = val;}
    void SetOCREnable(int val)         {m_OCREnable         = val;}
    void SetTTCCalib(int val)          {m_TTCCalib          = val;}
    void SetSelfTriggerEnable(int val) {m_SelfTriggerEnable = val;}
    void SetLatencyScanStart(int val)  {m_LatencyScanStart  = val;}
    void SetLatencyScanNBC(int val)    {m_LatencyScanNBC    = val;}
    void SetL1AReadoutEnable()  {m_L1AReadoutEnable = 1;}
    void SetL1AReadoutDisable() {m_L1AReadoutEnable = 0;}
    std::vector<uint32_t> PFEBBCIDs(uint32_t val_07_00,
                                    uint32_t val_15_08,
                                    uint32_t val_23_16
                                    ) const;

 private:
    I2cMasterConfig m_padtriggerfpga;
    int m_L1AReadoutLatency;
    int m_L1AReadoutNBC;
    int m_L1AReadoutEnable;
    int m_pFEBBCIDOffset;
    int m_StartIdleState;
    int m_OCREnable;
    int m_TTCCalib;
    int m_SelfTriggerEnable;
    int m_LatencyScanStart;
    int m_LatencyScanNBC;

    static constexpr std::uint32_t BIT_OLDFW_L1AReadoutLatency = 0;
    static constexpr std::uint32_t BIT_OLDFW_L1AReadoutNBCMode = 7;
    static constexpr std::uint32_t BIT_OLDFW_pFEBBCIDOffset    = 11;
    static constexpr std::uint32_t BIT_OLDFW_L1AReadoutEnable  = 15;

    static constexpr std::uint32_t BIT_L1AReadoutLatency = 0;
    static constexpr std::uint32_t BIT_L1AReadoutNBCMode = 7;
    static constexpr std::uint32_t BIT_L1AReadoutEnable  = 11;
    static constexpr std::uint32_t BIT_pFEBBCIDOffset    = 12;
    static constexpr std::uint32_t BIT_StartIdleState    = 24;
    static constexpr std::uint32_t BIT_OCREnable         = 25;
    static constexpr std::uint32_t BIT_TTCCalib          = 26;
    static constexpr std::uint32_t BIT_SelfTriggerEnable = 30;
};

}

#endif  // NSWCONFIGURATION_PADTRIGGERSCA_H_
