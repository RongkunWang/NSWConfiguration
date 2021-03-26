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

    explicit ARTConfig(const ptree& config);
    ~ARTConfig() {}

    ptree getConfig() const {return m_config;}
    void dump() const {}
    void dump_core() const { core.dump(); }
    void dump_ps()   const { ps  .dump(); }

    std::string getName() const { return name; }
    void setName(std::string str) { name = std::move(str); }

    std::string getNameCore() const { return getName() + "Core." + getName() + "Core"; }
    std::string getNamePs()   const { return getName() + "Ps."   + getName() + "Ps"; }
    std::string getNameGbtx() const { return "gbtx" + std::to_string(index()) + ".gbtx" + std::to_string(index()); }

    int index() const { return i_art; }
    void setIndex(int i) { i_art = i; }

    size_t NPhase() const { return n_phase; }
    std::string PhaseToString(uint phase) const;
    int register0_test_00() const;
    int art_core_cfg_deser_flagmask() const;
    int TP_GBTxAlignmentBit() const;
    int TP_GBTxAlignmentSleepTime() const;
    bool TP_GBTxAlignmentSkip() const;
    std::vector<uint> TP_GBTxAlignmentCommonPhases() const;
    std::vector<uint> TP_GBTxAlignmentPhasesToTest() const;
    std::string TP_GBTxAlignmentPhase() const;
    std::string getOpcServerIp_TP() const;
    std::string getOpcNodeId_TP() const;
    uint8_t BcidFromTp(const std::vector<uint8_t>& vec) const;
    bool IsAlignedWithTP(const std::vector<uint8_t>& vec) const;
    bool IsMyTP(const std::string& ServerIp, const std::string& NodeId) const;
    bool failsafe() const;

};

}  // namespace nsw

#endif  // NSWCONFIGURATION_ARTCONFIG_H_
