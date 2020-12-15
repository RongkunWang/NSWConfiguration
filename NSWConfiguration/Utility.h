// Utility functions

#ifndef NSWCONFIGURATION_UTILITY_H_
#define NSWCONFIGURATION_UTILITY_H_

#include <algorithm>
#include <iostream>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <utility>
#include <set>

#include "boost/property_tree/ptree.hpp"

#include "ers/ers.h"

using boost::property_tree::ptree;

ERS_DECLARE_ISSUE(nsw,
                  RegisterOverflow,
                  message,
                  ((const char *)message)
                  )

namespace nsw {

template<size_t N1, size_t N2>
std::bitset<N1 + N2> concatenate(const std::bitset<N1>& b1, const std::bitset<N2>& b2) {
    auto str1 = b1.to_string();
    auto str2 = b2.to_string();
    return std::bitset<N1 + N2>(str1+str2);
}

template<size_t N1, size_t N2, size_t N3>
std::bitset<N1 + N2 + N3> concatenate(const std::bitset<N1>& b1, const std::bitset<N2>& b2, const std::bitset<N3>& b3) {
    return concatenate(concatenate(b1, b2), b3);
}

/// Returns byte vector for the value of size nbytes
std::vector<uint8_t> intToByteVector(uint32_t value, size_t nbytes, bool littleEndian = true);

/// Returns bit pattern for the value
std::string bitString(unsigned value, size_t nbits);

/// Reverses a bit pattern of a long and returns string with reversed bits
std::string reversedBitString(unsigned value, size_t nbits);

/// Convert bitset to string of hex numbers
template<size_t N>
std::string bitsetToHexString(const std::bitset<N>& b) {
    std::ostringstream ss;
    for (int i=N-8; i >= 0; i=i-8) {
        auto val = std::bitset<N>(0xff) & (b >> i);
        ss << std::hex << std::setfill('0') << std::setw(2) << val.to_ulong();
    }
    return ss.str();
}

std::string getElementType(const std::string& element_name);

void checkOverflow(size_t register_size, unsigned value, const std::string& register_name);

std::vector<uint8_t> stringToByteVector(const std::string& bitstr);
std::vector<uint8_t> hexStringToByteVector(const std::string& hexstr, int length = 4, bool littleEndian = true);

/// Converts string of bits to string of bytes in hexadecimal
std::string bitstringToHexString(const std::string& bitstr);

/// Converts vector of values to string of hex
std::string vectorToHexString(std::vector<uint8_t> vec, bool littleEndian = false);

/// Converts vector of values to string of bits
std::string vectorToBitString(std::vector<uint8_t> vec, bool littleEndian = false);


/// Build bitstream from an vector of name-size pairs, and a property tree with matching names and values
std::string buildBitstream(const std::vector<std::pair<std::string, size_t>>& name_sizes, const ptree& config);

/// Converts vector to ptree
ptree buildPtreeFromVector(const std::vector<unsigned>& channelarray);

/// Strips string "_READONLY" from end of string, used for i2c addresses
std::string stripReadonly(std::string str);

/// Guesses the sector name from a string
/// e.g., guessSector("191A-A06-MM-Calib") should return "A06"
std::string guessSector(const std::string& str);

/// Return name of all elements that match the regular expression in ptree
/// \param regexp Regular expression to match
/// \param pt input ptree
/// \param current_node Current ptree node we are at, required for recursive calls
/// \return set of matched elements. Each element has the full path of each node
std::set<std::string> matchRegexpInPtree(const std::string& regexp, const ptree& pt,
    const std::string& current_node = "");

}  // namespace nsw
#endif  // NSWCONFIGURATION_UTILITY_H_
