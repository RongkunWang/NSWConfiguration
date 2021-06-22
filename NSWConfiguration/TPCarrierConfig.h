
/// TP Configuration class

#ifndef NSWCONFIGURATION_TPCARRIERCONFIG_H_
#define NSWCONFIGURATION_TPCARRIERCONFIG_H_

#include "NSWConfiguration/SCAConfig.h"

namespace nsw {

class TPCarrierConfig: public SCAConfig {

 public:
    explicit TPCarrierConfig(const boost::property_tree::ptree& config);
    ~TPCarrierConfig() = default;

 public:
    uint32_t RJOutSel() const;

};

}

#endif  // NSWCONFIGURATION_TPCARRIERCONFIG_H_


