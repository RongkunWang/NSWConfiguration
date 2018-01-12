#include <map>
#include <cmath>
#include <string>
#include <exception>
#include <vector>

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/Utility.h"

nsw::ROCCodec::ROCCodec() {
}

nsw::ROCCodec& nsw::ROCCodec::Instance() {
    static nsw::ROCCodec c;
    return c;
}


