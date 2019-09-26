#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/PadTriggerSCAConfig.h"

#include "boost/property_tree/json_parser.hpp"

nsw::PadTriggerSCAConfig::PadTriggerSCAConfig(ptree config):
    FEConfig(config)
{
    // std::cout << std::endl;
}

void nsw::PadTriggerSCAConfig::dump() {
    // std::cout << std::endl;
}

