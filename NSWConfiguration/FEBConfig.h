/// MMFE8 Configuration class

#ifndef NSWCONFIGURATION_FEBCONFIG_H_
#define NSWCONFIGURATION_FEBCONFIG_H_

#include <bitset>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/FEConfig.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

namespace nsw {

//! Base class or configuration for any front end board that has
//! - SCA
//! - ROC and VMM(multiple)
//! - TDS(zero or multiple)
//!
//! Used as base class for MMFE8, PFEB and SBEF Configurations

class FEBConfig: public FEConfig {
 private:
    std::vector<VMMConfig> m_vmms;
    I2cMasterConfig m_roc_analog;
    I2cMasterConfig m_roc_digital;
    std::vector<I2cMasterConfig> m_tdss;
    std::vector<std::string> m_gpios;  // List of GPIO names in the FEB

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    //! - rocPllCoreAnalog, rocCoreDigital
    //! - Multiple vmm instances named vmm0 to vmmN
    //! - Multiple tds instances named tds0 to tdsN (optional)
    explicit FEBConfig(ptree config);
    ~FEBConfig() {}

    void dump() {
        m_roc_analog.dump();
        m_roc_digital.dump();
        for (auto tds : m_tdss) {tds.dump();}
    }

    const std::vector<VMMConfig> & getVmms() const {return m_vmms;}
    const I2cMasterConfig & getRocAnalog() const {return m_roc_analog;}
    const I2cMasterConfig & getRocDigital() const {return m_roc_digital;}
    const std::vector<I2cMasterConfig> & getTdss() const {return m_tdss;}

    I2cMasterConfig & getRocAnalog() {
        return const_cast<I2cMasterConfig &>(static_cast<const FEBConfig&>(*this).getRocAnalog());
    }

    I2cMasterConfig & getRocDigital() {
        return const_cast<I2cMasterConfig &>(static_cast<const FEBConfig&>(*this).getRocDigital());
    }

    std::vector<VMMConfig> & getVmms() {
        return const_cast<std::vector<VMMConfig>&>(static_cast<const FEBConfig&>(*this).getVmms());
    }

    std::vector<I2cMasterConfig> & getTdss() {
        return const_cast<std::vector<I2cMasterConfig>&>(static_cast<const FEBConfig&>(*this).getTdss());
    }
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_FEBCONFIG_H_
