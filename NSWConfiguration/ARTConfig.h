/// ART Configuration class

#ifndef NSWCONFIGURATION_ARTCONFIG_H_
#define NSWCONFIGURATION_ARTCONFIG_H_

#include <string>
#include <vector>
#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/SCAConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

namespace nsw {

class ARTConfig {
 protected:
    ptree m_config;

 private:
    std::string name;
    size_t i_art;
    size_t n_phase = 32;

 public:
    nsw::I2cMasterConfig core;
    nsw::I2cMasterConfig ps;

    explicit ARTConfig(ptree config);
    ~ARTConfig() {}

    ptree getConfig() const {return m_config;}
    void dump() {}
    void dump_core() { core.dump(); }
    void dump_ps()   { ps  .dump(); }

    std::string getName() { return name; }
    void setName(std::string str) { name = str; }

    std::string getNameCore() { return getName() + "Core." + getName() + "Core"; }
    std::string getNamePs()   { return getName() + "Ps."   + getName() + "Ps"; }
    std::string getNameGbtx() { return "gbtx" + std::to_string(index()) + ".gbtx" + std::to_string(index()); }

    int index() { return i_art; }
    void setIndex(int i) { i_art = i; }

    size_t NPhase() { return n_phase; }
    std::string PhaseToString(uint phase);
    int register0_test_00();
    int art_core_cfg_deser_flagmask();
    int TP_GBTxAlignmentBit();
    int TP_GBTxAlignmentSleepTime();
    bool TP_GBTxAlignmentSkip();
    std::vector<uint> TP_GBTxAlignmentCommonPhases();
    std::vector<uint> TP_GBTxAlignmentPhasesToTest();
    std::string TP_GBTxAlignmentPhase();
    std::string getOpcServerIp_TP();
    std::string getOpcNodeId_TP();
    bool IsAlignedWithTP(std::vector<uint8_t> vec);
    bool IsMyTP(std::string ServerIp, std::string NodeId);
    bool failsafe();

};

}  // namespace nsw

#endif  // NSWCONFIGURATION_ARTCONFIG_H_
