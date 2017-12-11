// ROC Encoder/Decoder class to convert ptree/bitsets to each other
// This is a singleton function that should be created as follows:
// nsw::ROCCodec& roccodec = nsw::ROCCodec::Instance();

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

using boost::property_tree::ptree;

namespace nsw {

/// Class to encode/decode ROC configuration between ptrees and bytestreams
class ROCCodec {
 public:
    static ROCCodec& Instance();

 private:
    /// Private ROCCodec for singleton class
    ROCCodec();
    ~ROCCodec() { std::cout << "Destroying ROCCodec" << std::endl;}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_ROCCODEC_H_
