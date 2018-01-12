#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/ROCConfig.h"

using boost::property_tree::ptree;

nsw::ROCConfig::ROCConfig(ptree config): FEConfig(config) {
}
