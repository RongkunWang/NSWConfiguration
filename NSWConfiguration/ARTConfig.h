/// ART Configuration class

#ifndef NSWCONFIGURATION_ARTCONFIG_H_
#define NSWCONFIGURATION_ARTCONFIG_H_

#include <string>
#include <vector>
#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/FEConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

using boost::property_tree::ptree;

namespace nsw {

class ARTConfig: public FEConfig {
 private:
    std::string name;

 public:
    // nsw::I2cMasterConfig core;
    // nsw::I2cMasterConfig ps;

    explicit ARTConfig(ptree config);
    ~ARTConfig() {}

    void dump() {}

    std::string getName() { return name; }
    void setName(std::string str) { name = str; }

    int register0_test_00();

};

}  // namespace nsw

#endif  // NSWCONFIGURATION_ARTCONFIG_H_
