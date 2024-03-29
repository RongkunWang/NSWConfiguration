#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/Constants.h"

#include <cstring>
#include <exception>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <ranges>

#include <ers/ers.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>

std::uint32_t nsw::overwriteBits(const std::uint32_t original,
                                 const std::uint32_t value,
                                 const std::uint32_t position,
                                 const std::uint32_t nbits) {
  constexpr std::uint32_t one{1};
  if (value >= (one << nbits)) {
    const auto* err = "Value exceeds number of bits";
    throw std::runtime_error(err);
  }
  if (position >= sizeof(original) * CHAR_BIT) {
    const auto* err = "Position exceeds original";
    throw std::runtime_error(err);
  }
  if (nbits >= sizeof(original) * CHAR_BIT) {
    const auto* err = "nbits exceeds original";
    throw std::runtime_error(err);
  }

  // bit mask
  std::uint32_t mask{0};
  for (std::uint32_t it = position; it < position + nbits; ++it) {
    mask += (one << it);
  }

  // update
  std::uint32_t update{original};
  update &= ~mask;
  update |= (value << position);

  return update;
}

std::vector<std::uint8_t> nsw::intToByteVector(const std::uint32_t value, const std::size_t nbytes, const bool littleEndian) {
    std::vector<std::uint8_t> byteVector(nbytes);
    for (std::size_t i = 0; i < nbytes; i++) {
        byteVector.at(i) = static_cast<std::uint8_t>(value >> (i * NUM_BITS_IN_BYTE));
    }
    if (!littleEndian) {
        std::reverse(byteVector.begin(), byteVector.end());
    }
    return byteVector;
}

std::vector<std::uint8_t> nsw::intToByteVector(const std::uint8_t value, const std::size_t nbytes, const bool littleEndian) {
  return nsw::intToByteVector(static_cast<std::uint32_t>(value), nbytes, littleEndian);
}

std::string nsw::reversedBitString(const unsigned value, const std::size_t nbits) {
    auto str = bitString(value, nbits);
    std::reverse(str.begin(), str.end());
    return str;
}

std::string nsw::bitString(const unsigned value, const std::size_t nbits) {
    if (nbits > MAX_REGISTER_SIZE) {
        throw std::out_of_range(fmt::format("Maximum size is {}", MAX_REGISTER_SIZE));
    }
    auto str = fmt::format("{0:0{1}b}", value, nbits);
    if (str.size() > nbits) {
      return str.substr(str.size() - nbits, str.size());
    }
    return str;
}

std::string nsw::getElementType(const std::string& element_name) {
    // Check whether it is a geopraphical ID
    if (element_name.find('/') != std::string::npos) {
        // TODO: Add missing devices (TP, TP Carrier)
        if (element_name.find("/PadTrig/") != std::string::npos) {
          return "PadTrigger";
        }
        if (element_name.find("/Rim-L1DDC/") != std::string::npos){
            return "RimL1DDC";
        }
        if (element_name.find("/L1DDC/") != std::string::npos){
            return "L1DDC";
        }
        for (auto&& name : {"ADDC", "Router"}) {
          if (element_name.find(fmt::format("/{}/", name)) != std::string::npos) {
            return name;
          }
        }
        if (element_name.starts_with("MM")) {
            if (element_name.find("/TrigProc/") != std::string::npos) {
                if (element_name.find("/E0") != std::string::npos) {
                    return "MMTP";
                } else if (element_name.find("/E1") != std::string::npos) {
                    return "TPCarrier";
                }
            }
            if (element_name.find("/Strip/") != std::string::npos) {
                return "MMFE8";
            }
        }
        if (element_name.starts_with("sTGC") or element_name.starts_with("STG")) {
            if (element_name.find("/Strip/") != std::string::npos) {
                if (element_name.ends_with("R0")) {
                    return "SFEB8";
                }
                return "SFEB6";
            }
            if (element_name.find("/Pad/") != std::string::npos) {
                return "PFEB";
            }
            if (element_name.find("/TrigProc/") != std::string::npos) {
                if (element_name.find("/E0") != std::string::npos) {
                    return "STGCTP";
                } else if (element_name.find("/E1") != std::string::npos) {
                    return "TPCarrier";
                }
            }
        }
        throw std::runtime_error(fmt::format("Unknown front end element type: {}", element_name));
    }
    for (auto name : nsw::ELEMENT_NAMES) {
        if (element_name.find(name) != std::string::npos) {
            ERS_DEBUG(2,"Found instance of " << name << " configuration: " << element_name);
            return name;
        }
    }
    throw std::runtime_error(fmt::format("Unknown front end element type: {}", element_name));
}

void nsw::checkOverflow(const std::size_t register_size, const unsigned value, const std::string_view register_name) {
    if (std::pow(2, register_size) <= value) {
      const auto err =
        fmt::format("Overflow, register: {}, size: {}, max value: {}, actual value: {}",
                    register_name,
                    register_size,
                    std::pow(2, register_size) - 1,
                    value);
      nsw::RegisterOverflow issue(ERS_HERE, err);
      ers::warning(issue);
      throw issue;
    }
}

std::vector<std::uint8_t> nsw::stringToByteVector(const std::string_view bitstr) {
    std::vector<std::uint8_t> vec;
    std::string substr;
    // Go 8 bit at a time and convert it to hex
    for (std::size_t pos = 0; pos < bitstr.length(); pos += NUM_BITS_IN_BYTE) {
        substr = bitstr.substr(pos, NUM_BITS_IN_BYTE);
        ERS_DEBUG(6, fmt::format("substr: {}", substr));
        const auto byte = static_cast<std::uint8_t>(std::stoul(substr, nullptr, nsw::BASE_BIN));
        vec.push_back(byte);
        ERS_DEBUG(6, fmt::format("{:#0x}", byte));
    }
    ERS_DEBUG(6, fmt::format("Vector size: {}", vec.size()));
    return vec;
}

std::vector<uint8_t> nsw::hexStringToByteVector(const std::string_view hexstr, const std::size_t length, const bool littleEndian) {
    std::vector<uint8_t> vec;
    vec.reserve(length);
    // Go 8 bit at a time and convert it to hex
    for (std::size_t pos = 0; pos < hexstr.length(); pos += 2) {
        const auto substr = std::string{hexstr.substr(pos, 2)};
        ERS_DEBUG(6, fmt::format("substr: {}", substr));
        auto byte = static_cast<std::uint8_t>(std::stoul(substr, nullptr, nsw::BASE_HEX));
        vec.push_back(byte);
        ERS_DEBUG(6, fmt::format("{:#0x}", byte));
    }

    if (littleEndian) {
        std::reverse(vec.begin(), vec.end());
        vec.resize(length);
    } else {
        // big endian, need to insert zeros at beginning
        if (vec.size() < length) {
            vec.insert(vec.begin(), length - vec.size(), 0);
        } else {
            // trim the vector
            vec.resize(length);
        }
    }

    ERS_DEBUG(6, "Vector size: " << std::dec << vec.size());
    return vec;
}

std::uint32_t nsw::byteVectorToWord32(std::vector<std::uint8_t> vec, const bool littleEndian) {
  if (vec.size() != nsw::NUM_BYTES_IN_WORD32) {
    throw std::runtime_error(fmt::format(
      "Cannot convert std::vector<uint8_t> with {} elements to uint32_t", std::size(vec)));
  }
  if (!littleEndian) {
    std::reverse(vec.begin(), vec.end());
  }
  std::uint32_t val{};
  std::memcpy(&val, &vec.at(0), std::size(vec));
  return val;
}

std::string nsw::vectorToHexString(const std::span<const std::uint8_t> vec, bool littleEndian) {
    const auto stringify = [] (auto&& data) {
      return fmt::format("{:02x}", fmt::join(data, ""));
    };
    if (littleEndian) {
      return stringify(std::views::reverse(vec));
    }
    return stringify(vec);
}

std::string nsw::vectorToBitString(const std::span<const std::uint8_t> vec, bool littleEndian) {
    const auto stringify = [] (auto&& data) {
      return fmt::format("{:08b}", fmt::join(data, ""));
    };
    if (littleEndian) {
      return stringify(std::views::reverse(vec));
    }
    return stringify(vec);
}

std::string nsw::bitstringToHexString(const std::string& bitstr) {
    return vectorToHexString(stringToByteVector(bitstr));
}

boost::property_tree::ptree nsw::buildPtreeFromVector(const std::span<const unsigned int> vec) {
    boost::property_tree::ptree temp;

    // This is the only way to create an array in a ptree
    for (const auto& value : vec) {
        boost::property_tree::ptree child;
        child.put("", value);
        temp.push_back(std::make_pair("", child));
    }
    return temp;
}

/// Strips string "_READONLY" from end of string, used for i2c addresses
std::string nsw::stripReadonly(std::string str) {
    constexpr static std::string_view STR_TO_STRIP = "_READONLY";
    auto pos = str.find(STR_TO_STRIP);
    if (pos != std::string::npos) {
        str.erase(pos, STR_TO_STRIP.length());
    }
    return str;
}

std::set<std::string> nsw::matchRegexpInPtree(const std::string& regexp, const boost::property_tree::ptree& pt,
    const std::string& current_node) {
    std::set<std::string> names;
    const std::regex re(regexp);

    const std::string base_node = current_node.empty() ? "" : fmt::format("{}.", current_node);

    for (const auto& v : pt) {
        const auto node = v.first;
        const auto pt2 = v.second;
        const auto match = std::regex_match(node, re);
        if (match) {
           names.emplace(base_node + node);
        }
        if (!pt2.empty()) {  // pt2 is a ptree, not a single value
            const auto temp = matchRegexpInPtree(regexp, pt2, base_node + node);
            for (const auto& el : temp) {
              names.emplace(el);
            }
        }
    }
    return names;
}

std::string nsw::guessSector(const std::string_view str) {
  constexpr static std::array<std::string_view, nsw::NUM_WHEELS> sides = {"A", "C"};
  constexpr static std::array<std::string_view, nsw::NUM_SECTORS> sects = {
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
  };
  for (const auto& side : sides) {
    for (const auto& sector : sects) {
      const auto name = fmt::format("{}{}", side, sector);
      const auto p1_name = fmt::format("E{}-S{}", side, sector);
      if (str.find(name) != std::string::npos or str.find(p1_name) != std::string::npos) {
        return name;
      }
    }
  }
  return "";
}


std::string nsw::dumpTree(const boost::property_tree::ptree& pt)
{
    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, pt);
    return ss.str();
}


std::string nsw::getPrintableGbtxConfig(std::vector<uint8_t> data){
    // Return nicely formatted GBTx configuration string from configuration vector
    if (data.size()!=436){
        ERS_DEBUG(1,"Asked to print GBTx configuration with size!=436. This strongly indicates a mistake.");
    }

    std::stringstream ss;
    ss<<"\nreg |";
    for (std::size_t i=0; i<data.size(); i++){
        if (i%16==0) {
            ss<<std::dec<<'\n'<<i;
            if (i<10) ss<<" ";
            if (i<100) ss<<" ";
            ss<<" | ";
        }
        if (data.at(i)<std::uint8_t{0x10}) ss<<"0";
        ss << std::hex << static_cast<int>(data.at(i)) << std::dec << " ";
    }
    ss<<'\n';
    return ss.str();
}

std::vector<std::string> nsw::tokenizeString(const std::string& s, const std::string_view delimiter) {
  std::vector<std::string> outVec;
  std::size_t last = 0;
  std::size_t next = 0;
  while ((next = s.find(delimiter, last)) != std::string::npos) {
    outVec.push_back(s.substr(last, next-last));
    last = next + 1;
  }
  outVec.push_back(s.substr(last));
  return outVec;
}


__uint128_t nsw::constructUint128t(const std::uint64_t msbs, const std::uint64_t lsbs) {
  __uint128_t ret{};
  ret = (__uint128_t{msbs} << nsw::NUM_BITS_IN_WORD64) + lsbs;
  return ret;
}


bool nsw::isLargeSector(const std::string_view sector_name) {
  //
  // Large sectors are odd-numbered. Small are even.
  // string("A13") -> static_cast<bool>(13 % 2)
  //
  if (sector_name.size() != 3) {
    const std::string msg =
      fmt::format("Sector name should be 3 characters. e.g. A13 or C02. You gave: {}", sector_name);
    nsw::BadSectorName issue(ERS_HERE, msg);
    ers::error(issue);
    throw issue;
  }
  const auto sector_str = std::string{sector_name.substr(1)};
  const auto sector_int = std::stoi(sector_str);
  return sector_int % 2 == 1;
}

bool nsw::isSmallSector(const std::string_view sector_name) {
  return !(nsw::isLargeSector(sector_name));
}

void nsw::snooze(const std::chrono::duration<float> dur) {
  std::this_thread::sleep_for(dur);
}

std::string nsw::getenv(const std::string& env_var) {
  const auto* ret = std::getenv(env_var.c_str());
  if (ret == nullptr) {
    return "";
  }
  return ret;
}

bool nsw::contains(const std::string_view& s1, const std::string_view& s2) {
  return s1.find(s2) != std::string::npos;
}

std::unordered_set<std::string> nsw::getPathsFromPtree(const boost::property_tree::ptree& tree)
{
  std::unordered_set<std::string> result{};
  // Lambda which takes itself as first argument to call itself recursively. Hides implementation
  // details. (Would be a public function in Utility which is just a helper)
  const auto walkTreeImpl = [](const auto& func,
                                const boost::property_tree::ptree& ptree,
                                std::unordered_set<std::string>& key_set,
                                const std::string& parent_key) {
    if (ptree.empty()) {
      key_set.insert(parent_key);
      return;
    }
    for (const auto& it : ptree) {
      std::string key = parent_key;
      // add a separator between key segments
      if (!key.empty()) {
        key += '.';
      }
      key += it.first;
      func(func, it.second, key_set, key);
    }
  };
  walkTreeImpl(walkTreeImpl, tree, result, "");
  return result;
}

namespace boost::property_tree {

  boost::optional<
    translator_between<std::string, __uint128_t>::type::external_type>
  translator_between<std::string, __uint128_t>::type::get_value(
    const internal_type& str) {
    __uint128_t res = 0;

    for (const char c : str) {
      if (std::isdigit(c) == 0) {
        throw std::runtime_error(std::string("Non-numeric character: ") + c);
      }
      res *= 10;
      res += static_cast<__uint128_t>(c - '0');
    }

    return res;
  }

  boost::optional<
    translator_between<std::string, __uint128_t>::type::internal_type>
  translator_between<std::string, __uint128_t>::type::put_value(
    const external_type& obj) {
    std::stringstream ss;

    return [&obj]() {
      // Recursive lambda
      const auto toString_impl = [](const auto&          func,
                                    const external_type& x) -> std::string {
        // If the value fits into a 64 bit integer use standard to_string conversion
        // Otherwise convert it bit by bit until the value fits into a 64 bit integer
        if (x > std::numeric_limits<std::uint64_t>::max()) {
          __uint128_t   leading  = x / 10;
          std::uint64_t trailing = static_cast<std::uint64_t>(x % 10);
          return func(func, leading) + std::to_string(trailing);
        }
        return std::to_string(static_cast<std::uint64_t>(x));
      };
      return toString_impl(toString_impl, obj);
    }();
  }
}  // namespace boost::property_tree

std::ostream& operator<<(std::ostream& out, const __uint128_t x) {
  if (x > std::numeric_limits<std::uint64_t>::max()) {
    __uint128_t   leading  = x / 10;
    auto trailing = static_cast<std::uint64_t>(x % 10);
    return out << leading << trailing;
  }
  return out << static_cast<std::uint64_t>(x);
}
