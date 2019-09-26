#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/RouterConfig.h"

#include "boost/property_tree/json_parser.hpp"

nsw::RouterConfig::RouterConfig(ptree config):
    FEConfig(config)
{
    // std::cout << std::endl;
}

void nsw::RouterConfig::dump() {
    // std::cout << std::endl;
}

