/// ROC Configuration class

#ifndef NSWCONFIGURATION_ROCCONFIG_H_
#define NSWCONFIGURATION_ROCCONFIG_H_

#include <bitset>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class ROCConfig: public FEConfig {
 private:
    ROCCodec& codec = ROCCodec::Instance();

 public:
    explicit ROCConfig(ptree config);
    ~ROCConfig() {}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCONFIG_H_
