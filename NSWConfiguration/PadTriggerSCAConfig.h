/// ADDC Configuration class

#ifndef NSWCONFIGURATION_PADTRIGGERSCA_H_
#define NSWCONFIGURATION_PADTRIGGERSCA_H_

#include <vector>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "NSWConfiguration/SCAConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class PadTriggerSCAConfig: public SCAConfig {
 private:
    bool dummy;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit PadTriggerSCAConfig(const ptree& config);
    ~PadTriggerSCAConfig() {}

    void dump();
    bool ConfigRepeaters() const;
    bool ConfigVTTx() const;
    int UserControlRegister() const;
    int UserL1AReadoutLatency() const;
    int UserL1AReadoutNBC() const;
    int UserL1AReadoutNBCMode() const;
    int UserL1AReadoutEnable() const;
    int UserpFEBBCIDOffset() const;
    void SetUserL1AReadoutLatency(int val) {m_UserL1AReadoutLatency = val;}
    void SetUserL1AReadoutNBC(int val)     {m_UserL1AReadoutNBC     = val;}
    void SetUserL1AReadoutEnable(int val)  {m_UserL1AReadoutEnable  = val;}
    void SetUserpFEBBCIDOffset(int val)    {m_UserpFEBBCIDOffset    = val;}
    void SetUserL1AReadoutEnable()  {m_UserL1AReadoutEnable  = 1;}
    void SetUserL1AReadoutDisable() {m_UserL1AReadoutEnable  = 0;}

    int RealControlRegister() const {return m_RealControlRegister;}
    int RealL1AReadoutLatency() const {return m_RealL1AReadoutLatency;}
    int RealL1AReadoutNBC() const;
    int RealL1AReadoutNBCMode() const {return m_RealL1AReadoutNBCMode;}
    int RealL1AReadoutEnable() const {return m_RealL1AReadoutEnable;}
    int RealpFEBBCIDOffset() const {return m_RealpFEBBCIDOffset;}
    void SetRealControlRegister(int val);
    void SetRealL1AReadoutLatency(int val) {m_RealL1AReadoutLatency = val;}
    void SetRealL1AReadoutNBCMode(int val) {m_RealL1AReadoutNBCMode = val;}
    void SetRealL1AReadoutEnable(int val)  {m_RealL1AReadoutEnable  = val;}
    void SetRealpFEBBCIDOffset(int val)    {m_RealpFEBBCIDOffset    = val;}

 private:
    int m_UserL1AReadoutLatency;
    int m_UserL1AReadoutNBC;
    int m_UserL1AReadoutEnable;
    int m_UserpFEBBCIDOffset;
    int m_RealControlRegister;
    int m_RealL1AReadoutLatency;
    int m_RealL1AReadoutNBCMode;
    int m_RealL1AReadoutEnable;
    int m_RealpFEBBCIDOffset;

};

}

#endif  // NSWCONFIGURATION_PADTRIGGERSCA_H_
