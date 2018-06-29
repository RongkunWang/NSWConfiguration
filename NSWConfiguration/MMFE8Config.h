/// MMFE8 Configuration class

#ifndef NSWCONFIGURATION_MMFE8CONFIG_H_
#define NSWCONFIGURATION_MMFE8CONFIG_H_

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

//! Master class that holds the information of MMFE8 elements

class MMFE8Config: public FEConfig {
 private:
    std::vector<VMMConfig> vmms;
    I2cMasterConfig roc_analog;
    I2cMasterConfig roc_digital;

 public:
    explicit MMFE8Config(ptree config);
    ~MMFE8Config() {}

    void dump() {roc_analog.dump(); roc_digital.dump();}

    const std::vector<VMMConfig> & getVmms() const {return vmms;}
    const I2cMasterConfig & getRocAnalog() const {return roc_analog;}
    const I2cMasterConfig & getRocDigital() const {return roc_digital;}

    I2cMasterConfig & getRocAnalog() {
        return const_cast<I2cMasterConfig &>(static_cast<const MMFE8Config&>(*this).getRocAnalog());
    }

    I2cMasterConfig & getRocDigital() {
        return const_cast<I2cMasterConfig &>(static_cast<const MMFE8Config&>(*this).getRocDigital());
    }

    std::vector<VMMConfig> & getVmms() {
        return const_cast<std::vector<VMMConfig>&>(static_cast<const MMFE8Config&>(*this).getVmms());
    }

};
}  // namespace nsw

#endif  // NSWCONFIGURATION_MMFE8CONFIG_H_
