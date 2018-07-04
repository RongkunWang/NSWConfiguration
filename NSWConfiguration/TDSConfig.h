/// TDS Configuration class

#ifndef NSWCONFIGURATION_TDSCONFIG_H_
#define NSWCONFIGURATION_TDSCONFIG_H_

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

//! Class to encode/decode TDS configuration between ptrees and bytestreams

class TDSConfig: public FEConfig {
 public:
    nsw::I2cMasterConfig i2c;

    explicit TDSConfig(ptree config):
        FEConfig(config),
        i2c(config.get_child(TDS_NAME), TDS_NAME, TDS_REGISTERS) { }
    ~TDSConfig() {}

    void dump() {i2c.dump();}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_TDSCONFIG_H_
