/// MMFE8 Configuration class

#ifndef NSWCONFIGURATION_TPCONFIG_H_
#define NSWCONFIGURATION_TPCONFIG_H_

#include <bitset>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/FEConfig.h"
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

class TPConfig: public FEConfig {
 private:
    // std::vector<VMMConfig> m_vmms;
    // I2cMasterConfig m_tp_analog;
    // I2cMasterConfig m_tp_digital;
    std::vector<I2cMasterConfig> m_config_elements;
    // std::vector<std::string> m_gpios;  // List of GPIO names in the FEB

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    //! - rocPllCoreAnalog, rocCoreDigital
    //! - Multiple vmm instances named vmm0 to vmmN
    //! - Multiple tds instances named tds0 to tdsN (optional)
    explicit TPConfig(ptree config);
    ~TPConfig() {}

    void dump();

    // const std::vector<VMMConfig> & getVmms() const {return m_vmms;}
    // const I2cMasterConfig & getTpAnalog() const {return m_tp_analog;}
    // const I2cMasterConfig & getTpDigital() const {return m_tp_digital;}

    // this will need to be renamed once we understand what each master corresponds to
    const std::vector<I2cMasterConfig> & getConfigElements() const {return m_config_elements;}

    // I2cMasterConfig & getTpAnalog() {
    //     return const_cast<I2cMasterConfig &>(static_cast<const TPConfig&>(*this).getTpAnalog());
    // }

    // I2cMasterConfig & getTpDigital() {
    //     return const_cast<I2cMasterConfig &>(static_cast<const TPConfig&>(*this).getTpDigital());
    // }

    // std::vector<VMMConfig> & getVmms() {
    //     return const_cast<std::vector<VMMConfig>&>(static_cast<const TPConfig&>(*this).getVmms());
    // }

    std::vector<I2cMasterConfig> & getConfigElements() {
        return const_cast<std::vector<I2cMasterConfig>&>(static_cast<const TPConfig&>(*this).getConfigElements());
    }
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_TPCONFIG_H_
