// Utility functions

#ifndef NSWCONFIGURATION_UTILITY_H_
#define NSWCONFIGURATION_UTILITY_H_

#include <algorithm>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <unordered_set>

#include <fmt/core.h>

#include <boost/property_tree/ptree.hpp>
// https://stackoverflow.com/a/9433783
// namespace boost::property_tree {
//   template <class Key, class Data, class KeyCompare>
//   class basic_ptree;

//   typedef basic_ptree<std::string, std::string, std::less<std::string> > ptree;
// }

#include "NSWConfiguration/Constants.h"

#include <ers/Issue.h>

ERS_DECLARE_ISSUE(nsw,
                  RegisterOverflow,
                  message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  BadSectorName,
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

/**
 * \brief Overwrite a subset of bits of an integer
 *
 * \param original the original integer
 * \param value the new value to be inserted into the integer
 * \param position the first (least significant) bit position of the new value
 * \param nbits the number of bits to be overwritten
 *
 * e.g., Passing (0x00000000, 0xFF, 16, 8) returns 0x00FF0000.
 */
std::uint32_t overwriteBits(std::uint32_t original, std::uint32_t value,
                            std::uint32_t position, std::uint32_t nbits);

/// Returns byte vector for the value of size nbytes
std::vector<uint8_t> intToByteVector(uint32_t value, size_t nbytes, bool littleEndian = true);
std::vector<uint8_t> intToByteVector(uint8_t  value, size_t nbytes, bool littleEndian = true);

/**
 * \brief Transform an integer into a byte vector
 * 
 * \tparam Integer type of the integer
 * \param val value to be transformed
 * \return std::vector<std::uint8_t> byte vector
 * \todo Merge with \ref intToByteVector functions above (and concepts)
 */
template<typename Integer>
std::vector<std::uint8_t> integerToByteVector(const Integer val) {
  return integerToByteVector(val, sizeof(val));
}

/**
 * \brief Transform an integer into a byte vector
 * 
 * \tparam Integer type of the integer
 * \param val value to be transformed
 * \param nBytes number of bytes
 * \return std::vector<std::uint8_t> byte vector
 * \todo Merge with \ref intToByteVector functions above (and concepts)
 */
template<typename Integer>
std::vector<std::uint8_t> integerToByteVector(const Integer val, std::size_t nBytes) {
  static_assert(std::is_unsigned_v<Integer> or std::is_same_v<Integer, __uint128_t>, "Value must be an integer type");
  if (nBytes > sizeof(val)) {
    throw std::logic_error(fmt::format("Value has only {} bytes. Cannot split it into a byte vector of {} bytes", sizeof(val), nBytes));
  }
  std::vector<std::uint8_t> bytes(nBytes);
  for (std::size_t counter = 0; counter < nBytes; counter++) {
    bytes.at(nBytes - 1 - counter) = static_cast<std::uint8_t>(val >> (counter * nsw::NUM_BITS_IN_BYTE));
  }
  return bytes;
}

/// Returns bit pattern for the value
std::string bitString(unsigned value, size_t nbits);

/// Reverses a bit pattern of a long and returns a long with reversed bits
unsigned reversedBits(unsigned value, size_t nbits);

/// Reverses a bit pattern of a long and returns string with reversed bits
std::string reversedBitString(unsigned value, size_t nbits);

std::string getElementType(const std::string& element_name);

void checkOverflow(size_t register_size, unsigned value, const std::string& register_name);

std::vector<uint8_t> stringToByteVector(const std::string& bitstr);
std::vector<uint8_t> hexStringToByteVector(const std::string& hexstr, std::size_t length = nsw::NUM_BYTES_IN_WORD32, bool littleEndian = true);

/// Converts vector of bytes to 32-bit word
uint32_t byteVectorToWord32(std::vector<uint8_t> vec, bool littleEndian = false);

/// Converts string of bits to string of bytes in hexadecimal
std::string bitstringToHexString(const std::string& bitstr);

/// Converts vector of values to string of hex
std::string vectorToHexString(std::vector<uint8_t> vec, bool littleEndian = false);

/// Converts vector of values to string of bits
std::string vectorToBitString(std::vector<uint8_t> vec, bool littleEndian = false);


/// Build bitstream from an vector of name-size pairs, and a property tree with matching names and values
std::string buildBitstream(const std::vector<std::pair<std::string, size_t>>& name_sizes, const boost::property_tree::ptree& config);

/// Converts vector to ptree
// TODO: Concept
template<typename Container>
boost::property_tree::ptree buildPtreeFromVector(const Container& vec) {
    boost::property_tree::ptree temp;

    // This is the only way to create an array in a ptree
    for (const auto value : vec) {
        boost::property_tree::ptree child;
        child.put("", value);
        temp.push_back(std::make_pair("", child));
    }
    return temp;
}

/// Strips string "_READONLY" from end of string, used for i2c addresses
std::string stripReadonly(std::string str);

/// Guesses the sector name from a string
/// e.g., guessSector("191A-A06-MM-Calib") should return "A06"
std::string guessSector(const std::string& str);

/// Guesses the sector size (small, large) from the name
/// e.g., isLargeSector("A06") should return False
bool isLargeSector(const std::string& sector_name);
bool isSmallSector(const std::string& sector_name);

/// Return name of all elements that match the regular expression in ptree
/// \param regexp Regular expression to match
/// \param pt input ptree
/// \param current_node Current ptree node we are at, required for recursive calls
/// \return set of matched elements. Each element has the full path of each node
std::set<std::string> matchRegexpInPtree(const std::string& regexp, const boost::property_tree::ptree& pt,
    const std::string& current_node = "");

/// Sleep for some amount of time
/// Default argument: 1 second
void snooze(const std::chrono::duration<float> dur = std::chrono::seconds(1));

/**
 * @brief Dump a ptree into a JSON formatted string
 * 
 * @param pt ptree
 * @return std::string JSON string
 */
std::string dumpTree(const boost::property_tree::ptree &pt);

std::string getPrintableGbtxConfig(std::vector<uint8_t> data);

/**
 * \brief Get all complete paths from a ptree
 *
 * \param tree The ptree
 * \return std::unordered_set<std::string> Complete set of paths
 */
std::unordered_set<std::string> getPathsFromPtree(const boost::property_tree::ptree& tree);

/**
 * \brief Transform a map into a ptree of the form key : value
 *
 * \tparam Key Type of the key of the map
 * \tparam Value Type of the value of the map
 * \param map Map to be transformed (keys can contain '.' for nested ptrees)
 * \return boost::property_tree::ptree Ptree representation of the map
 */
template<typename Key, typename Value>
boost::property_tree::ptree transformMapToPtree(const std::map<Key, Value>& map) {
  boost::property_tree::ptree result;
  for (const auto& [key, value] : map) {
    result.put(key, value);
  }
  return result;
}

/**
 * \brief Transform a ptree into a map
 * 
 * \tparam Value Type of the value of the map
 * \param tree Tree to be transformed (nested structures will be converted to keys with '.')
 * \return boost::property_tree::ptree Map representation of the ptree
 */
template<typename Value>
std::map<std::string, Value> transformPtreetoMap(const boost::property_tree::ptree& tree) {
  std::map<std::string, Value> result{};
  for (const auto& key : getPathsFromPtree(tree)) {
    result[key] = tree.get<Value>(key);
  }
  return result;
}
std::vector<std::string> tokenizeString(const std::string& s, const std::string& delimiter);

}  // namespace nsw

namespace boost::property_tree {

/**
 * \brief Add ptree support for __uint128_t
 *
 * \tparam  __uint128_t to std::string
 */
template<>
struct translator_between<std::string, __uint128_t>
{
  struct type {
    using internal_type = std::string;
    using external_type = __uint128_t;

    /**
     * \brief Translation from std::string to __uint128_t
     *
     * \param str stringified value
     * \return boost::optional<external_type> value
     */
    static boost::optional<external_type> get_value(const internal_type& str);

    /**
     * \brief Translation from __uint128_t to std::string
     *
     * \param obj value
     * \return boost::optional<internal_type> stringified value
     */
    static boost::optional<internal_type> put_value(const external_type& obj);
  };
};

} // namespace boost::property_tree

/**
 * \brief Stream operator for 128 bit integers
 *
 * \param out output stream
 * \param x value to be streamed
 * \return std::ostream& stream with value
 */
std::ostream& operator<<(std::ostream& out, __uint128_t x);


#endif  // NSWCONFIGURATION_UTILITY_H_
