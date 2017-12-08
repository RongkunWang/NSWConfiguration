#include <string>

#include "NSWConfiguration/Utility.h"

// template<size_t N1, size_t N2>
// std::bitset<N1 + N2> concatenate(std::bitset<N1> b1, std::bitset<N2> b2);

std::string nsw::reversedBitString(unsigned value, size_t nbits) {
    std::bitset<32> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    std::reverse(str.begin(), str.end());
    return str;
}

std::string nsw::bitString(unsigned value, size_t nbits) {
    std::bitset<32> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    std::reverse(str.begin(), str.end());
    return str;
}
