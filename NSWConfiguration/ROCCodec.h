// ROC Encoder/Decoder class to convert ptree/bitsets to each other

#ifndef NSWCONFIGURATION_ROCCODEC_H_
#define NSWCONFIGURATION_ROCCODEC_H_

#include <bitset>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/I2cFECodec.h"

using boost::property_tree::ptree;

namespace nsw {

//! Class to encode/decode ROC configuration between ptrees and bytestreams
class ROCCodec: public I2cFECodec {
 public:
    ROCCodec();
    ~ROCCodec() { std::cout << "Destroying ROCCodec" << std::endl;}

    // Set ROC registers
    void setRegisterMapping();
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCODEC_H_
