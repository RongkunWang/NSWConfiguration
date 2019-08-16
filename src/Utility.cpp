#include <string>
#include <exception>
#include <cmath>
#include <vector>
#include <utility>
#include <regex>
#include <set>

#include "boost/foreach.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/Utility.h"

// template<size_t N1, size_t N2>
// std::bitset<N1 + N2> concatenate(std::bitset<N1> b1, std::bitset<N2> b2);


std::vector<unsigned char> nsw::intToByteVector(int value, size_t nbytes, bool littleEndian) {
    std::vector<unsigned char> byteVector(nbytes);
    for (int i = 0; i < nbytes; i++)
        byteVector.at(i) = (value >> (i * 8));
    if (!littleEndian)
        std::reverse(byteVector.begin(), byteVector.end());
    return byteVector;
}


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
    return str;
}

std::string nsw::getElementType(std::string element_name) {
    for (auto name : std::vector<std::string>({"VMM", "TDS", "ROC", "MMFE8", "PFEB", "SFEB", "TP", "ADDC"})) {
        if (element_name.find(name) != std::string::npos) {
            ERS_DEBUG(2, "Found instance of " << name << " configuration: " << element_name);
            return name;
        }
    }
    auto err = "Unknown front end element type: " + element_name;
    throw std::runtime_error(err);
}

void nsw::checkOverflow(size_t register_size, unsigned value, std::string register_name) {
    if (std::pow(2, register_size) <= value) {
        std::string err = "Overflow, register: " + register_name + ", size: "
                           + std::to_string(register_size) + ", max value: "
                           + std::to_string(std::pow(2, register_size)-1)
                           + ", actual value: " + std::to_string(value);
        throw std::runtime_error(err);  // TODO(cyildiz): convert to ers
    }
}

std::vector<uint8_t> nsw::stringToByteVector(std::string bitstr) {
    std::vector<uint8_t> vec;
    std::string substr;
    uint8_t byte;
    // Go 8 bit at a time and convert it to hex
    for (size_t pos; pos < bitstr.length(); pos=pos+8) {
        substr = bitstr.substr(pos, 8);
        ERS_DEBUG(6, std::string("substr: ") << substr);
        byte = static_cast<uint8_t> (std::stoi(substr, nullptr, 2));
        vec.push_back(byte);
        ERS_DEBUG(6, std::hex << "0x" << unsigned(byte));
    }
    ERS_DEBUG(6, "Vector size: " << std::dec << vec.size());
    return vec;
}

std::vector<uint8_t> nsw::hexStringToByteVector(std::string hexstr, int length = 4, bool littleEndian = true) {
    std::vector<uint8_t> vec(length);
    std::string substr;
    uint8_t byte;
    // Go 8 bit at a time and convert it to hex
    for (size_t pos; pos < hexstr.length(); pos=pos+2) {
        substr = hexstr.substr(pos, 2);
        ERS_DEBUG(6, std::string("substr: ") << substr);
        byte = static_cast<uint8_t> (std::strtoul(substr.c_str(), 0, 16));
        if(littleEndian) vec.insert(vec.begin(),byte);
        else vec.push_back(byte);
        ERS_DEBUG(6, std::hex << "0x" << unsigned(byte));
    }
    std::vector<uint8_t> vecFront(vec.begin(),vec.begin()+length);
    ERS_DEBUG(6, "Vector size: " << std::dec << vecFront.size());
    return vecFront;
}

std::string nsw::vectorToHexString(std::vector<uint8_t> vec) {
    std::stringstream hexstream;
    hexstream << std::hex << std::setfill('0');
    // Go 8 bit at a time and convert it to hex
    for (auto byte : vec) {
        hexstream << std::setw(2) << static_cast<uint32_t>(byte);
    }
    return hexstream.str();
}

std::string nsw::vectorToBitString(std::vector<uint8_t> vec) {
    std::string bitstring;
    // Go 8 bit at a time and convert it to binary
    for (auto byte : vec) {
        std::bitset<8> bs(byte);
        bitstring = bitstring +  bs.to_string();
    }
    return bitstring;
}

std::string nsw::bitstringToHexString(std::string bitstr) {
    std::string substr;
    uint32_t byte;
    std::string hexstr;
    std::stringstream hexstream;
    hexstream << std::hex << std::setfill('0');
    // Go 8 bit at a time and convert it to hex
    for (size_t pos; pos < bitstr.length(); pos=pos+8) {
        substr = bitstr.substr(pos, 8);
        byte = static_cast<uint32_t> (std::stoi(substr, nullptr, 2));
        hexstream << std::setw(2) << byte;
    }
    return hexstream.str();
}

std::string nsw::buildBitstream(const std::vector<std::pair<std::string, size_t>>& name_sizes, const ptree& config) {
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
                std::string temp = e.what();
                // nsw::MissingI2cRegister issue(ERS_HERE, temp.c_str());
                // ers::error(issue);
                // throw issue;
                // TODO(cyildiz): Throw an exception that should be propagated by caller
                std::cout << "Problem: " << temp << std::endl;
            }
            nsw::checkOverflow(size, value, name);
        }
        ERS_DEBUG(5, " -- " << name << " -> " << value);

        // TODO(cyildiz): Large enough to take any register
        std::bitset<32> bs(value);
        auto stringbs = bs.to_string();
        stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
        ERS_DEBUG(6, " --- substr:" << stringbs);
        tempstr += stringbs;
    }
    ERS_DEBUG(4, " - bitstream : " << tempstr);
    return tempstr;
}

ptree nsw::buildPtreeFromVector(std::vector<unsigned> vec) {
    ptree temp;

    // This is the only way to create an array in a ptree
    for (auto value : vec) {
        ptree child;
        child.put("", value);
        temp.push_back(std::make_pair("", child));
    }
    return temp;
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

std::set<std::string> nsw::matchRegexpInPtree(std::string regexp, ptree pt, std::string current_node) {
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
