
/// TP Configuration class

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
#include "NSWConfiguration/TP_I2cRegisterMappings.h"

using boost::property_tree::ptree;

namespace nsw {

    class TPConfig: public FEConfig {

    private:
    const uint8_t m_numMasters;
    std::map<std::string,I2cMasterConfig*> m_registerFiles;
    I2cMasterConfig *m_I2cMasterConfigPtrArr[NUM_REGISTER_FILES] = {NULL};
    void restructureConfig();

  public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit TPConfig(ptree config);
    ~TPConfig() ;

    uint32_t getRegisterValue(std::string master, std::string slave, std::string register_name = "register");
    void setRegisterValue(std::string master, std::string slave, uint32_t value, std::string register_name = "register");
    void dump();

    std::map<std::string,I2cMasterConfig*> getI2cMastersMap() { return m_registerFiles; }
    const uint8_t getNumMasters() { return m_numMasters; }
  };

}  // namespace nsw

#endif  // NSWCONFIGURATION_TPCONFIG_H_


