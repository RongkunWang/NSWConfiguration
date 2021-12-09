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
    bool ConfigFPGA() const;
    bool ConfigRepeaters() const;
    bool ConfigVTTx() const;
    const I2cMasterConfig & getFpga() const {return m_padtriggerfpga;}
    int LatencyScanStart() const;
    int LatencyScanNBC() const;
    void SetLatencyScanStart(int val)  {m_LatencyScanStart  = val;}
    void SetLatencyScanNBC(int val)    {m_LatencyScanNBC    = val;}
    std::vector<uint32_t> PFEBBCIDs(uint32_t val_07_00,
                                    uint32_t val_15_08,
                                    uint32_t val_23_16
                                    ) const;

 private:
    I2cMasterConfig m_padtriggerfpga;
    int m_LatencyScanStart;
    int m_LatencyScanNBC;

};

}

#endif  // NSWCONFIGURATION_PADTRIGGERSCA_H_
