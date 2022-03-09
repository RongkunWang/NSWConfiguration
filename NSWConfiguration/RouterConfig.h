#ifndef NSWCONFIGURATION_ROUTER_H_
#define NSWCONFIGURATION_ROUTER_H_

#include "NSWConfiguration/SCAConfig.h"

namespace nsw {

class RouterConfig: public SCAConfig {
 public:
    explicit RouterConfig(const boost::property_tree::ptree& config);
    ~RouterConfig() = default;
};

}

#endif  // NSWCONFIGURATION_ROUTER_H_
