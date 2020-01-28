#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/RouterConfig.h"

#include "boost/property_tree/json_parser.hpp"

nsw::RouterConfig::RouterConfig(ptree config):
    FEConfig(config)
{
    // std::cout << std::endl;
}

bool nsw::RouterConfig::CrashOnClkReadyFailure() const {
    return m_config.get<bool>("CrashOnClkReadyFailure");
}

bool nsw::RouterConfig::CrashOnConfigFailure() const {
    return m_config.get<bool>("CrashOnConfigFailure");
}

void nsw::RouterConfig::dump() {
    // std::cout << std::endl;
}

