#include <memory>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/I2cFECodec.h"
#include "NSWConfiguration/ROCConfig.h"

using boost::property_tree::ptree;

nsw::ROCConfig::ROCConfig(ptree config): I2cFEConfig(config) {
    codec = std::make_unique<ROCCodec>();
    m_address_bitstream = codec->buildConfig(config);
}
