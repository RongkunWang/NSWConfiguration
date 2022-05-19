// Utility functions

#ifndef NSWCONFIGURATION_UTILITY_H_
#define NSWCONFIGURATION_UTILITY_H_

#include <algorithm>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
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
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  BadSectorName,
                  message,
                  ((std::string)message)
                  )

namespace nsw {

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

/**
 * \brief Return byte vector for the value of size nbytes
 *
 * \param value Value to be split up
 * \param nbytes Number of bytes in vector
 * \param littleEndian Little or big endian
 * \return std::vector<std::uint8_t> byte vector
 */
std::vector<std::uint8_t> intToByteVector(std::uint32_t value, std::size_t nbytes, bool littleEndian = true);
std::vector<std::uint8_t> intToByteVector(std::uint8_t  value, std::size_t nbytes, bool littleEndian = true); //!< \overload

/**
 * \brief Transform an integer into a byte vector
 *
 * \param val value to be transformed
 * \param nBytes number of bytes
 * \return std::vector<std::uint8_t> byte vector
 * \todo Merge with \ref intToByteVector functions above
 */
template<typename Integral> requires std::unsigned_integral<Integral> or std::same_as<Integral, __uint128_t>
std::vector<std::uint8_t> integerToByteVector(const Integral val, const std::size_t nBytes) {
  if (nBytes > sizeof(val)) {
    throw std::logic_error(fmt::format("Value has only {} bytes. Cannot split it into a byte vector of {} bytes", sizeof(val), nBytes));
  }
  std::vector<std::uint8_t> bytes(nBytes);
  for (std::size_t counter = 0; counter < nBytes; counter++) {
    bytes.at(nBytes - 1 - counter) = static_cast<std::uint8_t>(val >> (counter * nsw::NUM_BITS_IN_BYTE));
  }
  return bytes;
}

/**
 * \brief Transform an integer into a byte vector
 *
 * \param val value to be transformed
 * \return std::vector<std::uint8_t> Byte vector
 * \todo Merge with \ref intToByteVector functions above
 */
template<typename Integral> requires std::unsigned_integral<Integral> or std::same_as<Integral, __uint128_t>
std::vector<std::uint8_t> integerToByteVector(const Integral val) {
  return integerToByteVector(val, sizeof(val));
}

/**
 * \brief Returns bit pattern for the value
 *
 * \param value Value
 * \param nbits Number of bits to be included in string (fills 0 left)
 * \return std::string Bit string
 */
std::string bitString(unsigned value, std::size_t nbits);

/// Reverses a bit pattern of a long and returns string with reversed bits

/**
 * \brief Returns reversed bit pattern for the value
 *
 * \param value Value
 * \param nbits Number of bits to be included in string (fills 0 right)
 * \return std::string Reversed bit string
 */
std::string reversedBitString(unsigned value, std::size_t nbits);

/**
 * \brief Return the type of a device
 *
 * Possible values: MMFE8, SFEB6, SFEB8, PFEB, TP, TPCarrier, STGCTP, ADDC, GBTX, Router, PadTrigger, L1DDC
 *
 * \param element_name JSON node name or geo ID
 * \return std::string Type of device
 */
std::string getElementType(const std::string& element_name);

/**
 * \brief Check if a value fits into a register of given size
 *
 * \param register_size Size of the register
 * \param value Value to be checked
 * \param register_name Name of the register (for warning output)
 */
void checkOverflow(std::size_t register_size, unsigned value, std::string_view register_name);

/**
 * \brief Convert a bit string into a byte vector
 *
 * \param bitstr String of 0 and 1
 * \return std::vector<std::uint8_t> Vector of bytes
 */
std::vector<std::uint8_t> stringToByteVector(std::string_view bitstr);

/**
 * \brief Convert a hex string into a byte vector
 *
 * \param hexstr String of a hex number
 * \param length Size of the output vector (add 0 bytes if neccessary)
 * \param littleEndian Little or big endian
 * \return std::vector<std::uint8_t> Vector of bytes
 */
std::vector<std::uint8_t> hexStringToByteVector(std::string_view hexstr, std::size_t length = nsw::NUM_BYTES_IN_WORD32, bool littleEndian = true);

/**
 * \brief Converts vector of bytes to 32-bit number
 *
 * \param vec Byte vector
 * \param littleEndian Little or big endian
 * \return std::uint32_t 32 bit number
 */
std::uint32_t byteVectorToWord32(std::vector<std::uint8_t> vec, bool littleEndian = false);

/**
 * \brief Convert a bit string into a hex string
 *
 * \param bitstr Bit string
 * \return std::string Hex string
 */
std::string bitstringToHexString(const std::string& bitstr);

/**
 * \brief Converts vector of bytes to a hex string
 *
 * \param vec Byte vector
 * \param littleEndian Little or big endian
 * \return std::string Hex string
 */
std::string vectorToHexString(std::span<const std::uint8_t> vec, bool littleEndian = false);

/**
 * \brief Converts vector of bytes to a bit string
 *
 * \param vec Byte vector
 * \param littleEndian Little or big endian
 * \return std::string Bit string
 */
std::string vectorToBitString(std::span<const std::uint8_t> vec, bool littleEndian = false);

/**
 * \brief Converts vector to ptree
 *
 * \param vec Span of unsigned integers
 * \return boost::property_tree::ptree Boost ptree containing values
 */
boost::property_tree::ptree buildPtreeFromVector(std::span<const unsigned int> vec);

/**
 * \brief Strips string "_READONLY" from end of string, used for i2c addresses
 *
 * \param str String possibly containing "_READONLY"
 * \return std::string String without "_READONLY"
 */
std::string stripReadonly(std::string str);

/**
 * \brief Guesses the sector name from a string
 *
 * e.g., guessSector("191A-A06-MM-Calib") should return "A06"
 *
 * \param str String containing wheel and sector
 * \return std::string Wheel and sector
 */
std::string guessSector(std::string_view str);

/**
 * \brief Guesses the sector size (small, large) from the name
 *
 * e.g., isLargeSector("A06") should return false
 *
 * \param sector_name String containing wheel and sector
 * \return true Is large sector
 * \return false Is small sector
 */
bool isLargeSector(std::string_view sector_name);

/**
 * \brief Guesses the sector size (small, large) from the name
 *
 * e.g., isLargeSector("A06") should return true
 *
 * \param sector_name String containing wheel and sector
 * \return true Is small sector
 * \return false Is large sector
 */
bool isSmallSector(std::string_view sector_name);

/**
 * \brief Return name of all elements that match the regular expression in ptree
 *
 * \param regexp Regular expression to match
 * \param pt Input ptree
 * \param current_node Current ptree node we are at, required for recursive calls
 * \return std::set<std::string> Set of matched elements. Each element has the full path of each node
 */
std::set<std::string> matchRegexpInPtree(const std::string& regexp, const boost::property_tree::ptree& pt,
    const std::string& current_node = "");

/**
 * \brief Sleep for some amount of time
 *
 * \param dur Time (default 1 second)
 */
void snooze(std::chrono::duration<float> dur = std::chrono::seconds(1));

/**
 * \brief Dump a ptree into a JSON formatted string
 *
 * \param pt ptree
 * \return std::string JSON string
 */
std::string dumpTree(const boost::property_tree::ptree &pt);

std::string getPrintableGbtxConfig(std::vector<uint8_t> data);

/**
 * \brief Get a set from a ptree
 *
 * \tparam Type Type of the items contained in the set
 * \param tree The ptree
 * \param key The key of the set
 * \return std::set<Type> The set of items of type Type in the ptree matching the key
 */
template<typename Type>
std::set<Type> getSetFromPtree(const boost::property_tree::ptree& tree,
                               const std::string& key) {
  std::set<Type> items;
  for (const auto& item: tree.get_child(key)) {
    items.emplace(item.second.get_value<Type>());
  }
  return items;
}

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
 * \brief Get environment variable
 *
 * \param name of environment variable
 * \return std::string environment variable
 */
std::string getenv(const std::string& env_var);

/**
 * \brief Check if string contains substring
 *
 * Banish this function upon arrival of c++23!
 * https://en.cppreference.com/w/cpp/string/basic_string/contains
 *
 * \param s1 the string which may contain s2
 * \param s2 the string which may be contained within s1
 */
bool contains(const std::string_view& s1, const std::string_view& s2);

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
std::vector<std::string> tokenizeString(const std::string& s, std::string_view delimiter);

/**
 * \brief Function to construct 128 bit unsigned integers from standard 64-bit unsigned integers
 *
 * \param msbs most significant 64 bits
 * \param lsbs least significant 64 bits
 * \return 128 bit
 */
__uint128_t constructUint128t(std::uint64_t msbs, std::uint64_t lsbs);


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
