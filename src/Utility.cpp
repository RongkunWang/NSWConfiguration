#include "NSWConfiguration/Utility.h"

#include <exception>
#include <cmath>
#include <regex>
#include <thread>

#include <ers/ers.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;

// template<size_t N1, size_t N2>
// std::bitset<N1 + N2> concatenate(std::bitset<N1> b1, std::bitset<N2> b2);

std::uint32_t nsw::overwriteBits(const std::uint32_t original,
                                 const std::uint32_t value,
                                 const std::uint32_t position,
                                 const std::uint32_t nbits) {
  constexpr std::uint32_t one{1};
  if (value >= (one << nbits)) {
    const auto err = "Value exceeds number of bits";
    throw std::runtime_error(err);
  }
  if (position >= sizeof(original) * CHAR_BIT) {
    const auto err = "Position exceeds original";
    throw std::runtime_error(err);
  }
  if (nbits >= sizeof(original) * CHAR_BIT) {
    const auto err = "nbits exceeds original";
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

std::vector<uint8_t> nsw::intToByteVector(uint32_t value, size_t nbytes, bool littleEndian) {
    std::vector<uint8_t> byteVector(nbytes);
    for (size_t i = 0; i < nbytes; i++)
        byteVector.at(i) = static_cast<std::uint8_t>(value >> (i * NUM_BITS_IN_BYTE));
    if (!littleEndian)
        std::reverse(byteVector.begin(), byteVector.end());
    return byteVector;
}

std::vector<uint8_t> nsw::intToByteVector(uint8_t value, size_t nbytes, bool littleEndian) {
  return nsw::intToByteVector(static_cast<uint32_t>(value), nbytes, littleEndian);
}

unsigned nsw::reversedBits(unsigned value, size_t nbits) {
  return static_cast<unsigned>(std::stoul(reversedBitString(value, nbits), nullptr, nsw::BASE_BIN));
}

std::string nsw::reversedBitString(unsigned value, size_t nbits) {
    std::bitset<MAX_REGISTER_SIZE> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    std::reverse(str.begin(), str.end());
    return str;
}

std::string nsw::bitString(unsigned value, size_t nbits) {
    std::bitset<MAX_REGISTER_SIZE> b(value);
    auto str = b.to_string();
    str = str.substr(str.size()-nbits, str.size());
    return str;
}

std::string nsw::getElementType(const std::string& element_name) {
    for (auto name : nsw::ELEMENT_NAMES) {
        if (element_name.find(name) != std::string::npos) {
            ERS_DEBUG(2,"Found instance of " << name << " configuration: " << element_name);
            return name;
        }
    }
    auto err = "Unknown front end element type: " + element_name;
    throw std::runtime_error(err);
}

void nsw::checkOverflow(size_t register_size, unsigned value, const std::string& register_name) {
    if (std::pow(2, register_size) <= value) {
        std::string err = "Overflow, register: " + register_name + ", size: "
                           + std::to_string(register_size) + ", max value: "
                           + std::to_string(std::pow(2, register_size)-1)
                           + ", actual value: " + std::to_string(value);
        nsw::RegisterOverflow issue(ERS_HERE, err.c_str());
        ers::warning(issue);
        throw issue;
    }
}

std::vector<uint8_t> nsw::stringToByteVector(const std::string& bitstr) {
    std::vector<uint8_t> vec;
    std::string substr;
    uint8_t byte;
    // Go 8 bit at a time and convert it to hex
    for (size_t pos = 0; pos < bitstr.length(); pos=pos+NUM_BITS_IN_BYTE) {
        substr = bitstr.substr(pos, NUM_BITS_IN_BYTE);
        ERS_DEBUG(6, std::string("substr: ") << substr);
        byte = static_cast<uint8_t> (std::stoi(substr, nullptr, 2));
        vec.push_back(byte);
        ERS_DEBUG(6, std::hex << "0x" << unsigned(byte));
    }
    ERS_DEBUG(6, "Vector size: " << std::dec << vec.size());
    return vec;
}

std::vector<uint8_t> nsw::hexStringToByteVector(const std::string& hexstr, std::size_t length, bool littleEndian) {
    std::vector<uint8_t> vec;
    vec.reserve(length);
    std::string substr;
    // Go 8 bit at a time and convert it to hex
    for (size_t pos = 0; pos < hexstr.length(); pos=pos+2) {
        substr = hexstr.substr(pos, 2);
        ERS_DEBUG(6, std::string("substr: ") << substr);
        auto byte = static_cast<uint8_t>(std::strtoul(substr.c_str(), 0, 16));
        vec.push_back(byte);
        ERS_DEBUG(6, std::hex << "0x" << unsigned(byte));
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

uint32_t nsw::byteVectorToWord32(std::vector<uint8_t> vec, bool littleEndian) {
  if (vec.size() != nsw::NUM_BYTES_IN_WORD32) {
    throw std::runtime_error(
      "Cannot convert std::vector<uint8_t> with " + std::to_string(vec.size()) + " elements to uint32_t"
    );
  }
  if (!littleEndian) {
    std::reverse(vec.begin(), vec.end());
  }
  return *reinterpret_cast<uint32_t*>(vec.data());
}
std::string nsw::vectorToHexString(std::vector<uint8_t> vec, bool littleEndian) {
    std::stringstream hexstream;
    hexstream << std::hex << std::setfill('0');
    if (littleEndian) std::reverse(vec.begin(), vec.end());
    // Go 8 bit at a time and convert it to hex
    for (auto byte : vec) {
        hexstream << std::setw(2) << static_cast<uint32_t>(byte);
    }
    return hexstream.str();
}

std::string nsw::vectorToBitString(std::vector<uint8_t> vec, bool littleEndian) {
    std::string bitstring;
    if (littleEndian) std::reverse(vec.begin(), vec.end());
    // Go 8 bit at a time and convert it to binary
    for (auto byte : vec) {
        std::bitset<NUM_BITS_IN_BYTE> bs(byte);
        bitstring = bitstring +  bs.to_string();
    }
    return bitstring;
}

std::string nsw::bitstringToHexString(const std::string& bitstr) {
    std::stringstream hexstream;
    hexstream << std::hex << std::setfill('0');
    // Go 8 bit at a time and convert it to hex
    for (size_t pos = 0; pos < bitstr.length(); pos=pos+NUM_BITS_IN_BYTE) {
        auto substr = bitstr.substr(pos, 8);
        auto byte = static_cast<uint32_t> (std::stoi(substr, nullptr, 2));
        hexstream << std::setw(2) << byte;
    }
    return hexstream.str();
}

std::string nsw::buildBitstream(const std::vector<std::pair<std::string, size_t>>& name_sizes, const ptree& config) {
    // This function does something similar to nsw::I2cMasterCodec::buildConfig, but it's more generic
    std::string tempstr;
    for (auto ns : name_sizes) {
        auto name = ns.first;
        auto size = ns.second;

        unsigned value;
        // Fill not used bits with 0
        if (name == "NOT_USED") {
            value = 0;
        } else {
            try {
                value = config.get<unsigned>(name);
            } catch (const boost::property_tree::ptree_bad_path& e) {
                ERS_LOG("Problem: " << e.what());
                throw;
            }
            nsw::checkOverflow(size, value, name);
        }
        ERS_DEBUG(5, " -- " << name << " -> " << value);

        // TODO(cyildiz): Large enough to take any register
        std::bitset<MAX_REGISTER_SIZE> bs(value);
        auto stringbs = bs.to_string();
        stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
        ERS_DEBUG(6, " --- substr:" << stringbs);
        tempstr += stringbs;
    }
    ERS_DEBUG(4, " - bitstream : " << tempstr);
    return tempstr;
}

/// Strips string "_READONLY" from end of string, used for i2c addresses
std::string nsw::stripReadonly(std::string str) {
    std::string str_to_strip = "_READONLY";
    auto pos = str.find(str_to_strip);
    if (pos != std::string::npos) {
        str.erase(pos, str_to_strip.length());
    }
    return str;
}

std::set<std::string> nsw::matchRegexpInPtree(const std::string& regexp, const ptree& pt,
    const std::string& current_node) {
    std::set<std::string> names;
    std::regex re(regexp);

    std::string base_node = current_node == "" ? "" : current_node + ".";

    BOOST_FOREACH(const ptree::value_type &v, pt) {
        std::string node = v.first;
        ptree pt2 = v.second;
        bool match = std::regex_match(node, re);
        if (match) {
           names.emplace(base_node + node);
        }
        if (!pt2.empty()) {  // pt2 is a ptree, not a single value
            auto temp = matchRegexpInPtree(regexp, pt2, base_node + node);
            for (auto el : temp) {
              names.emplace(el);
            }
        }
    }
    return names;
}

std::string nsw::guessSector(const std::string& str) {
  const std::array<std::string, nsw::NUM_WHEELS> sides = {"A", "C"};
  const std::array<std::string, nsw::NUM_SECTORS> sects = {"01", "02", "03", "04",
                                                           "05", "06", "07", "08",
                                                           "09", "10", "11", "12",
                                                           "13", "14", "15", "16",
                                                           };
  for (const auto& side : sides) {
    for (const auto& sector : sects) {
      const auto name = side + sector;
      if (str.find(name) != std::string::npos) {
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
    for (int i=0; i<data.size(); i++){
        if (i%16==0) {
            ss<<std::dec<<'\n'<<i;
            if (i<10) ss<<" ";
            if (i<100) ss<<" ";
            ss<<" | ";
        }
        if (data.at(i)<0x10) ss<<"0";
        ss << std::hex << static_cast<int>(data.at(i)) << std::dec << " ";
    }
    ss<<'\n';
    return ss.str();
}

std::vector<std::string> nsw::tokenizeString(const std::string& s, const std::string& delimiter) {
  std::vector<std::string> outVec;
  size_t last = 0;
  size_t next = 0;
  while ((next = s.find(delimiter, last)) != std::string::npos) {
    outVec.push_back(s.substr(last, next-last));
    last = next + 1;
  }
  outVec.push_back(s.substr(last));
  return outVec;
}


bool nsw::isLargeSector(const std::string& sector_name) {
  //
  // Large sectors are odd-numbered. Small are even.
  // string("A13") -> static_cast<bool>(13 % 2)
  //
  if (sector_name.size() != 3) {
    const std::string msg0 = "Sector name should be 3 characters. ";
    const std::string msg1 = "e.g. A13 or C02. ";
    const std::string msg2 = "You gave: " + sector_name;
    const std::string msg = msg0 + msg1 + msg2;
    nsw::BadSectorName issue(ERS_HERE, msg.c_str());
    ers::error(issue);
    throw issue;
  }
  auto sector_str = sector_name.substr(1);
  auto sector_int = std::stoi(sector_str);
  return static_cast<bool>(sector_int % 2);
}

bool nsw::isSmallSector(const std::string& sector_name) {
  return !(nsw::isLargeSector(sector_name));
}

void nsw::snooze(const std::chrono::duration<float> dur) {
  std::this_thread::sleep_for(dur);
}

std::string nsw::getenv(const std::string& env_var) {
  const auto ret = std::getenv(env_var.c_str());
  if (ret == nullptr) {
    return "";
  }
  return std::string(ret);
}

std::unordered_set<std::string> nsw::getPathsFromPtree(const boost::property_tree::ptree& tree)
{
  std::unordered_set<std::string> result{};
  // Lambda which takes itself as first argument to call itself recursively. Hides implementation
  // details. (Would be a public function in Utility which is just a helper)
  const auto walkTreeImpl = [](const auto& func,
                                const boost::property_tree::ptree& tree,
                                std::unordered_set<std::string>& key_set,
                                const std::string& parent_key) {
    if (tree.empty()) {
      key_set.insert(parent_key);
      return;
    }
    for (const auto& it : tree) {
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
      res += (c - '0');
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
          std::uint64_t trailing = x % 10;
          return func(func, leading) + std::to_string(trailing);
        }
        return std::to_string(static_cast<std::uint64_t>(x));
      };
      return toString_impl(toString_impl, obj);
    }();
  }

ptree operator+(const ptree& lhs, const ptree& rhs)
{
    // TODO: Fix merging
    ptree result(lhs);
    result.insert(result.end(), rhs.begin(), rhs.end());
    return result;
}

}  // namespace boost::property_tree

std::ostream& operator<<(std::ostream& out, const __uint128_t x) {
  if (x > std::numeric_limits<std::uint64_t>::max()) {
    __uint128_t   leading  = x / 10;
    std::uint64_t trailing = x % 10;
    return out << leading << trailing;
  }
  return out << static_cast<std::uint64_t>(x);
}
