/// ADDC Configuration class

#ifndef NSWCONFIGURATION_ADDC_H_
#define NSWCONFIGURATION_ADDC_H_

#include <vector>
#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/FEConfig.h"
#include "NSWConfiguration/ARTConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class ADDCConfig: public FEConfig {
 private:
    std::vector<ARTConfig> m_arts;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit ADDCConfig(ptree config);
    ~ADDCConfig() {}

    void dump();

    ARTConfig & getART(size_t i) {return m_arts[i];}
    const std::vector<ARTConfig> & getARTs() const {return m_arts;}

};

}  // namespace nsw

#endif  // NSWCONFIGURATION_ADDC_H_
