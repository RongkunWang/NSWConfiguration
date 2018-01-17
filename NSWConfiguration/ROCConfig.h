/// ROC Configuration class

#ifndef NSWCONFIGURATION_ROCCONFIG_H_
#define NSWCONFIGURATION_ROCCONFIG_H_

#include <bitset>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/I2cFEConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class ROCConfig: public I2cFEConfig {
 public:
    explicit ROCConfig(ptree config);
    ~ROCConfig() {}
    ROCConfig(const ROCConfig&) = delete;
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCONFIG_H_
