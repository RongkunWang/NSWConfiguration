/// ROC Configuration class

#ifndef NSWCONFIGURATION_ROCCONFIG_H_
#define NSWCONFIGURATION_ROCCONFIG_H_

#include <bitset>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/I2cFEConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

namespace nsw {

//! Class to encode/decode ROC configuration between ptrees and bytestreams

class ROCConfig: public I2cFEConfig {
 public:
    explicit ROCConfig(ptree config): I2cFEConfig(config, ROC_REGISTER_SIZE) {}
    ~ROCConfig() {}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCONFIG_H_
