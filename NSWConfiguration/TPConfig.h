
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
#include "NSWConfiguration/I2cRegisterMappingsTP.h"

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
    std::map<std::string,I2cMasterConfig*> m_registerFiles;
    I2cMasterConfig *m_I2cMasterConfigPtrArr[NUM_REGISTER_FILES] = {NULL};
public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
  
    explicit TPConfig(ptree config);
    ~TPConfig() ;
        
    uint32_t getRegisterValue(std::string master, std::string slave, std::string register_name);        
    void setRegisterValue(std::string master, std::string slave, std::string register_name, uint32_t value);
    void dump();
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_TPCONFIG_H_


