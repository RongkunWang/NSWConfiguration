/// Router Configuration class

#ifndef NSWCONFIGURATION_ROUTER_H_
#define NSWCONFIGURATION_ROUTER_H_

#include <vector>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class RouterConfig: public FEConfig {
 private:
    bool dummy;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit RouterConfig(ptree config);
    ~RouterConfig() {}

    bool CrashOnClkReadyFailure() const;
    bool CrashOnConfigFailure() const;
    void dump();

};

}

#endif  // NSWCONFIGURATION_ROUTER_H_
