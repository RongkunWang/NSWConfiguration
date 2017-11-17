// Utility functions
#include <iostream>
#include <bitset>

#ifndef NSWCONFIGURATION_UTILITY_H_
#define NSWCONFIGURATION_UTILITY_H_

namespace nsw {

template<size_t N1, size_t N2>
std::bitset<N1 + N2> concatenate(std::bitset<N1> b1, std::bitset<N2> b2) {
    auto str1 = b1.to_string();
    auto str2 = b2.to_string();
    return std::bitset<N1 + N2>(str1+str2);
}

template<size_t N1, size_t N2, size_t N3>
std::bitset<N1 + N2 + N3> concatenate(std::bitset<N1> b1, std::bitset<N2> b2, std::bitset<N3> b3) {
    return concatenate(concatenate(b1, b2), b3);
}

}  // namespace nsw
#endif  // NSWCONFIGURATION_UTILITY_H_
