// Test functions in the Utility.h

#include <utility>
#include <string>

#include "NSWConfiguration/Utility.h"

#define BOOST_TEST_MODULE my test module
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

BOOST_AUTO_TEST_CASE(concatenate_test) {
    auto b1 = std::bitset<10>("1010101010");
    auto b2 = std::bitset<4>("1001");
    BOOST_TEST(nsw::concatenate(b1, b2) == std::bitset<14>("10101010101001"));

    auto b3 = std::bitset<14>("11111010101010");
    auto b4 = std::bitset<32>("11111111111111111111111111111111");
    auto b5 = std::bitset<3>("000");
    BOOST_TEST(nsw::concatenate(b3, b4, b5) == std::bitset<49>("1111101010101011111111111111111111111111111111000"));
}

BOOST_AUTO_TEST_CASE(reversedBitString_test) {
    auto b = std::bitset<8>("10101011");
    unsigned u = b.to_ulong();
    BOOST_TEST(nsw::reversedBitString(u, 8) == "11010101");

    BOOST_TEST(nsw::reversedBitString(3, 4) == "1100");

    BOOST_TEST(nsw::reversedBitString(3, 10) == "1100000000");
}

BOOST_AUTO_TEST_CASE(bitsetToHexString_test) {
    auto b1 = std::bitset<8>("10101011");
    BOOST_TEST(nsw::bitsetToHexString(b1) == "ab");

    auto b2 = std::bitset<16>("1010101000001111");
    BOOST_TEST(nsw::bitsetToHexString(b2) == "aa0f");

    auto b3 = std::bitset<16>("0000001000001111");
    BOOST_TEST(nsw::bitsetToHexString(b3) == "020f");

    auto b4 = std::bitset<24>("000000100000111101010101");
    BOOST_TEST(nsw::bitsetToHexString(b4) == "020f55");
}

BOOST_AUTO_TEST_CASE(stringToByteVector_test) {
    std::string s = "00000000";
    auto vec = nsw::stringToByteVector(s);
    BOOST_TEST(vec[0] == 0);
    BOOST_TEST(vec.size() = 1);
    vec.empty();

    s = "00000010";
    vec = nsw::stringToByteVector(s);
    BOOST_TEST(vec[0] == 2);
    BOOST_TEST(vec.size() = 1);

    s = "0000001010000000";
    vec = nsw::stringToByteVector(s);
    BOOST_TEST(vec[0] == 2);
    BOOST_TEST(vec[1] == 128);
    BOOST_TEST(vec.size() = 2);
}

BOOST_AUTO_TEST_CASE(vectorToHexString_test) {
    std::vector<uint8_t> vec = {0x00,0x01,0x06,0x12,0xf2,0x25,0x21};
    BOOST_TEST(nsw::vectorToHexString(vec) == "00010612f22521");
}

BOOST_AUTO_TEST_CASE(vectorToBitString_test) {
    std::vector<uint8_t> vec = {0x00,0x01,0x06,0x12,0xf2,0x25,0x21};
    BOOST_TEST(nsw::vectorToBitString(vec) == "00000000000000010000011000010010111100100010010100100001");
}

