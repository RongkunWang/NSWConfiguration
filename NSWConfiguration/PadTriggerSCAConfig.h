/// Pad Trigger Configuration class

#ifndef NSWCONFIGURATION_PADTRIGGERSCA_H_
#define NSWCONFIGURATION_PADTRIGGERSCA_H_

#include <vector>
#include <iostream>

#include "NSWConfiguration/SCAConfig.h"

namespace nsw {

class PadTriggerSCAConfig: public SCAConfig {
 private:
    bool dummy;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit PadTriggerSCAConfig(const boost::property_tree::ptree& config);
    ~PadTriggerSCAConfig() = default;

    void dump();
    std::string firmware() const;
    bool ConfigRepeaters() const;
    bool ConfigVTTx() const;
    bool ConfigControlRegister() const;
    int ControlRegister() const;
    int L1AReadoutLatency() const;
    int L1AReadoutNBC() const;
    int L1AReadoutNBCMode() const;
    int L1AReadoutEnable() const;
    int pFEBBCIDOffset() const;
    int StartIdleState() const;
    int OCREnable() const;
    int TTCCalib() const;
    int LatencyScanStart() const;
    int LatencyScanNBC() const;
    void SetL1AReadoutLatency(int val) {m_L1AReadoutLatency = val;}
    void SetL1AReadoutNBC(int val)     {m_L1AReadoutNBC     = val;}
    void SetL1AReadoutEnable(int val)  {m_L1AReadoutEnable  = val;}
    void SetpFEBBCIDOffset(int val)    {m_pFEBBCIDOffset    = val;}
    void SetStartIdleState(int val)    {m_StartIdleState    = val;}
    void SetOCREnable(int val)         {m_OCREnable         = val;}
    void SetTTCCalib(int val)          {m_TTCCalib          = val;}
    void SetLatencyScanStart(int val)  {m_LatencyScanStart  = val;}
    void SetLatencyScanNBC(int val)    {m_LatencyScanNBC    = val;}
    void SetL1AReadoutEnable()  {m_L1AReadoutEnable = 1;}
    void SetL1AReadoutDisable() {m_L1AReadoutEnable = 0;}

 private:
    int m_L1AReadoutLatency;
    int m_L1AReadoutNBC;
    int m_L1AReadoutEnable;
    int m_pFEBBCIDOffset;
    int m_StartIdleState;
    int m_OCREnable;
    int m_TTCCalib;
    int m_LatencyScanStart;
    int m_LatencyScanNBC;

};

}

#endif  // NSWCONFIGURATION_PADTRIGGERSCA_H_
