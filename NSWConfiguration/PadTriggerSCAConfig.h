/// ADDC Configuration class

#ifndef NSWCONFIGURATION_PADTRIGGERSCA_H_
#define NSWCONFIGURATION_PADTRIGGERSCA_H_

#include <vector>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "NSWConfiguration/FEConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class PadTriggerSCAConfig: public FEConfig {
 private:
    bool dummy;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit PadTriggerSCAConfig(ptree config);
    ~PadTriggerSCAConfig() {}

    void dump();

};

}

#endif  // NSWCONFIGURATION_PADTRIGGERSCA_H_
