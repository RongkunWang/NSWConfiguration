// Test functions in the Utility.h

#include <utility>
#include <string>
#include <vector>
#include <set>

#include "NSWConfiguration/Utility.h"

#define BOOST_TEST_MODULE Utility_tests
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/exceptions.hpp"
#include "boost/property_tree/json_parser.hpp"

using boost::property_tree::ptree;

BOOST_AUTO_TEST_CASE(IntToByteVector_LittleEndian_ReturnsLittleEndianByteVector) {
    const std::vector<uint8_t> expected4 = {0xef, 0xbe, 0xad, 0xde};
    BOOST_TEST(nsw::intToByteVector(0xdeadbeef, 4) == expected4);

    const std::vector<uint8_t> expected3 = {0x0d, 0xf0, 0x05};
    BOOST_TEST(nsw::intToByteVector(0xb105f00d, 3) == expected3);

    const std::vector<uint8_t> expected2 = {0xad, 0xfb};
    BOOST_TEST(nsw::intToByteVector(0xdecafbad, 2) == expected2);

    const std::vector<uint8_t> expected1 = { 0x0d };
    BOOST_TEST(nsw::intToByteVector(0xcafed00d, 1) == expected1);
}

BOOST_AUTO_TEST_CASE(IntToByteVector_BigEndian_ReturnsBigEndianByteVector) {
    const std::vector<uint8_t> expected4 = {0xde, 0xad, 0xbe, 0xef};
    BOOST_TEST(nsw::intToByteVector(0xdeadbeef, 4, false) == expected4);

    const std::vector<uint8_t> expected3 = {0x05, 0xf0, 0x0d};
    BOOST_TEST(nsw::intToByteVector(0xb105f00d, 3, false) == expected3);

    const std::vector<uint8_t> expected2 = {0xfb, 0xad};
    BOOST_TEST(nsw::intToByteVector(0xdecafbad, 2, false) == expected2);

    const std::vector<uint8_t> expected1 = { 0x0d };
    BOOST_TEST(nsw::intToByteVector(0xcafed00d, 1, false) == expected1);
}

/* WILLFAIL
   The function should probably throw in these cases, for two reasons:
    a. A large enough nbytes (say, as a result of underflow) can cause a bad_alloc exception,
       and (probably) nobody will have any idea why.
    b. An nbytes bigger than 4 (number of bytes in a uint32_t) will cause a bitwise shift by more than the amount of
       bits in the thing getting bitshifted (in this case, uint32_t == 32 bits), which is undefined behavior.
BOOST_AUTO_TEST_CASE(IntToByteVector_InvalidSize_ThrowsRuntimeError) {
    BOOST_CHECK_THROW(nsw::intToByteVector(0xaaaaaaaa, -1), std::runtime_error);
    BOOST_CHECK_THROW(nsw::intToByteVector(0xaaaaaaaa, -1, false), std::runtime_error);
    BOOST_CHECK_THROW(nsw::intToByteVector(0xaaaaaaaa, 5), std::runtime_error);
    BOOST_CHECK_THROW(nsw::intToByteVector(0xaaaaaaaa, 5, false), std::runtime_error);
}
*/

BOOST_AUTO_TEST_CASE(IntToByteVector_SizeZero_ReturnsEmptyVector) {
    BOOST_TEST(nsw::intToByteVector(0xaaaaaaaa, 0) == std::vector<uint8_t>{});
    BOOST_TEST(nsw::intToByteVector(0xaaaaaaaa, 0, false) == std::vector<uint8_t>{});
}


BOOST_AUTO_TEST_CASE(Concatenate_TwoValidInputs_ReturnsConcatenatedInputs) {
    std::string b1_string = "1010101010";
    std::string b2_string = "1001";
    std::bitset<10> b1{ b1_string };
    std::bitset<4> b2{ b2_string };

    BOOST_TEST(nsw::concatenate(b1, b2) == std::bitset<14>(b1_string + b2_string));
}

BOOST_AUTO_TEST_CASE(Concatenate_ThreeValidInputs_ReturnsConcatenatedInputs) {
    std::string b1_string = "11111010101010";
    std::string b2_string = "11111111111111111111111111111111";
    std::string b3_string = "000";

    std::bitset<14> b1{ b1_string };
    std::bitset<32> b2{ b2_string };
    std::bitset<3> b3{ b3_string };
    BOOST_TEST(nsw::concatenate(b1, b2, b3) == std::bitset<49>(b1_string + b2_string + b3_string));
}

BOOST_AUTO_TEST_CASE(Concatenate_EmptyInputs_ReturnsEmptyBitset) {
    auto b1 = std::bitset<0>{};
    auto b2 = std::bitset<0>{};
    BOOST_TEST(nsw::concatenate(b1, b2) == std::bitset<0>{});

    auto b3 = std::bitset<0>{};
    auto b4 = std::bitset<0>{};
    auto b5 = std::bitset<0>{};
    BOOST_TEST(nsw::concatenate(b3, b4, b5) == std::bitset<0>{});
}

BOOST_AUTO_TEST_CASE(ReversedBitstring_ValidInput_ReturnsReversedBitString) {
    BOOST_TEST(nsw::reversedBitString(0b10101011, 8) == "11010101");
    BOOST_TEST(nsw::reversedBitString(0x41414141, 32) == "10000010100000101000001010000010");
    BOOST_TEST(nsw::reversedBitString(3, 10) == "1100000000");
}

BOOST_AUTO_TEST_CASE(ReversedBitstring_SizeOverflow_ThrowsOutOfRange) {
    BOOST_CHECK_THROW(nsw::reversedBitString(0x41414141, 33), std::out_of_range);
    BOOST_CHECK_THROW(nsw::reversedBitString(0x41414141, -1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(Bitstring_ValidInput_ReturnsBitstring) {
    BOOST_TEST(nsw::bitString(0b10101011, 8) == "10101011");
    BOOST_TEST(nsw::bitString(0x41414141, 32) == "01000001010000010100000101000001");
    BOOST_TEST(nsw::bitString(3, 10) == "0000000011");
}

BOOST_AUTO_TEST_CASE(Bitstring_SizeOverflow_ThrowsOutOfRange) {
    BOOST_CHECK_THROW(nsw::bitString(0x41414141, 33), std::out_of_range);
    BOOST_CHECK_THROW(nsw::bitString(0x41414141, -1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(GetElementType_ElementNameInInput_ReturnsElementName) {
    BOOST_TEST(nsw::getElementType("A.MMFE8.B") == "MMFE8");
    BOOST_TEST(nsw::getElementType("A.PFEB.B") == "PFEB");
    BOOST_TEST(nsw::getElementType("A.SFEB_old.B") == "SFEB_old");
    BOOST_TEST(nsw::getElementType("A.SFEB8.B") == "SFEB8");
    BOOST_TEST(nsw::getElementType("A.SFEB6.B") == "SFEB6");
    BOOST_TEST(nsw::getElementType("A.SFEB.B") == "SFEB");
    BOOST_TEST(nsw::getElementType("A.TP.B") == "TP");
    BOOST_TEST(nsw::getElementType("A.ADDC.B") == "ADDC");
    BOOST_TEST(nsw::getElementType("A.PadTriggerSCA.B") == "PadTriggerSCA");
    BOOST_TEST(nsw::getElementType("A.Router.B") == "Router");
}

BOOST_AUTO_TEST_CASE(GetElementType_NoElementNameInInput_ThrowsRuntimeError) {
    BOOST_CHECK_THROW(nsw::getElementType("definitelynotanelementname"),
        std::runtime_error);
    // Check for false positives with lowercase names
    BOOST_CHECK_THROW(nsw::getElementType("mmfe8"), std::runtime_error);
    BOOST_CHECK_THROW(nsw::getElementType("pfeb"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(CheckOverflow_NoOverflow_DoesNotReportOverflow) {
    BOOST_CHECK_NO_THROW(nsw::checkOverflow(4, 10, "MyRegister"));
    BOOST_CHECK_NO_THROW(nsw::checkOverflow(8, 255, "MyRegister"));
    BOOST_CHECK_NO_THROW(nsw::checkOverflow(10, 1023, "MyRegister"));
    BOOST_CHECK_NO_THROW(nsw::checkOverflow(1, 1, "MyRegister"));
}

BOOST_AUTO_TEST_CASE(CheckOverflow_OverflowInInput_ThrowsRegisterOverflow) {
    BOOST_CHECK_THROW(nsw::checkOverflow(4, 16, "MyRegister"), nsw::RegisterOverflow);
    BOOST_CHECK_THROW(nsw::checkOverflow(8, 256, "MyRegister"), nsw::RegisterOverflow);
    BOOST_CHECK_THROW(nsw::checkOverflow(10, 10'000, "MyRegister"), nsw::RegisterOverflow);
}

BOOST_AUTO_TEST_CASE(CheckOverflow_OverflowInInput_ExceptionContainsRegisterName) {
    BOOST_CHECK_EXCEPTION(nsw::checkOverflow(4, 16, "SomeCoolRegisterName"), nsw::RegisterOverflow,
    [](const nsw::RegisterOverflow& ex) -> bool {
        std::string error{ ex.what() };
        return error.find("SomeCoolRegisterName") != std::string::npos;
    });
}

BOOST_AUTO_TEST_CASE(StringToByteVector_BitstringSizeMultipleOf8_ReturnsByteVector) {
    std::vector<uint8_t> expected = { 0x0 };
    BOOST_TEST(nsw::stringToByteVector("00000000") == expected);

    expected = { 0xc3 };
    BOOST_TEST(nsw::stringToByteVector("11000011") == expected);

    expected = { 0xc9, 0xc3 };
    BOOST_TEST(nsw::stringToByteVector("1100100111000011") == expected);
}

/* WILLFAIL
BOOST_AUTO_TEST_CASE(StringToByteVector_BitstringSizeNotMultipleOf8_ThrowsRuntimeError) {
    BOOST_CHECK_THROW(nsw::stringToByteVector("0101"), std::runtime_error);
    BOOST_CHECK_THROW(nsw::stringToByteVector("1111111111"), std::runtime_error);
}
*/

BOOST_AUTO_TEST_CASE(HexStringToByteVector_LittleEndian_ReturnsLittleEndianByteVector) {
    std::vector<uint8_t> expected = { 0xde, 0xad, 0xbe, 0xef };
    BOOST_TEST(nsw::hexStringToByteVector("efbeadde", 4, true) == expected);
    expected = { 0x13, 0x37 };
    BOOST_TEST(nsw::hexStringToByteVector("3713", 2, true) == expected);
}

BOOST_AUTO_TEST_CASE(HexStringToByteVector_BigEndian_ReturnsBigEndianByteVector) {
    std::vector<uint8_t> expected = { 0xde, 0xad, 0xbe, 0xef };
    BOOST_TEST(nsw::hexStringToByteVector("deadbeef", 4, false) == expected);
    expected = { 0x13, 0x37 };
    BOOST_TEST(nsw::hexStringToByteVector("1337", 2, false) == expected);
}

BOOST_AUTO_TEST_CASE(HexStringToByteVector_LittleEndianSizeMismatch_ResizesVector) {
    std::vector<uint8_t> expected = { 0xcd, 0xab, 0, 0 };
    BOOST_TEST(nsw::hexStringToByteVector("abcd", 4, true) == expected);

    expected = { 0xef, 0xbe };
    BOOST_TEST(nsw::hexStringToByteVector("deadbeef", 2, true) == expected);
}

BOOST_AUTO_TEST_CASE(HexStringToByteVector_BigEndianSizeMismatch_ResizesVector) {
    std::vector<uint8_t> expected = { 0, 0, 0xab, 0xcd };
    BOOST_TEST(nsw::hexStringToByteVector("abcd", 4, false) == expected);

    expected = { 0xde, 0xad };
    BOOST_TEST(nsw::hexStringToByteVector("deadbeef", 2, false) == expected);
}

/* WILLFAIL
BOOST_AUTO_TEST_CASE(HexStringToByteVector_SizeBiggerThanPossible_ThrowsRuntimeError) {
    BOOST_CHECK_THROW(nsw::hexStringToByteVector("1337", 3, true), std::runtime_error);
}
*/

BOOST_AUTO_TEST_CASE(VectorToBitString_BigEndian_ReturnsBigEndianBitString) {
    std::vector<uint8_t> vec = {0x00, 0x01, 0x06, 0x12, 0xf2, 0x25, 0x21};
    BOOST_TEST(nsw::vectorToBitString(vec) == "00000000000000010000011000010010111100100010010100100001");
}

BOOST_AUTO_TEST_CASE(VectorToBitString_LittleEndian_ReturnsLittleEndianBitString) {
    std::vector<uint8_t> vec = {0x00, 0x01, 0x06, 0x12, 0xf2, 0x25, 0x21};
    BOOST_TEST(nsw::vectorToBitString(vec, true) == "00100001001001011111001000010010000001100000000100000000");
}

BOOST_AUTO_TEST_CASE(VectorToHexString_BigEndian_ReturnsBigEndianHexString) {
    std::vector<uint8_t> vec = {0x00, 0x01, 0x06, 0x12, 0xf2, 0x25, 0x21};
    BOOST_TEST(nsw::vectorToHexString(vec, false) == "00010612f22521");
}

BOOST_AUTO_TEST_CASE(VectorToHexString_LittleEndian_ReturnsLittleEndianHexString) {
    std::vector<uint8_t> vec = { 0x68, 0x61, 0x63, 0x6b, 0x65, 0x72, 0x6d, 0x61, 0x6e };
    BOOST_TEST(nsw::vectorToHexString(vec, true) == "6e616d72656b636168");
}

BOOST_AUTO_TEST_CASE(BitstringToHexString_InputLengthMultipleOf8_ReturnsHexString) {
    BOOST_TEST(nsw::bitstringToHexString("01000001010000100100001101000100") == "41424344");
    BOOST_TEST(nsw::bitstringToHexString("00010011") == "13");
}

/* WILLFAIL
BOOST_AUTO_TEST_CASE(BitstringToHexString_InputLengthNotMultipleOf8_ThrowsRuntimeError) {
    BOOST_CHECK_THROW(nsw::nsw::bitstringToHexString("0101"), std::runtime_error);
    BOOST_CHECK_THROW(nsw::nsw::bitstringToHexString("0101010101"), std::runtime_error);
}
*/

BOOST_AUTO_TEST_CASE(BuildBitstream_ValidInput_ReturnsCorrectBitstream) {
    ptree pt;
    pt.put("register0.value0", 0x10);
    pt.put("register0.value1", 0x13);
    pt.put("register1", 0x41411337);

    std::vector<std::pair<std::string, size_t>> name_size_mapping = { {"NOT_USED", 8 }, {"register0.value0", 8}, {"register0.value1", 16}, {"register1", 32} };
    BOOST_TEST(nsw::buildBitstream(name_size_mapping, pt) == "0000000000010000000000000001001101000001010000010001001100110111");
}

BOOST_AUTO_TEST_CASE(BuildBitstream_OverflowInRegister_ThrowsRegisterOverflow) {
    ptree pt;
    pt.put("0v3rfl0w", 0x111);
    std::vector<std::pair<std::string, size_t>> name_size_mapping = { {"0v3rfl0w", 8} };
    BOOST_CHECK_EXCEPTION(nsw::buildBitstream(name_size_mapping, pt), nsw::RegisterOverflow,
    [](const nsw::RegisterOverflow& ex) -> bool {
        std::string error{ ex.what() };
        return error.find("0v3rfl0w") != std::string::npos;
    });
}

BOOST_AUTO_TEST_CASE(BuildBitstream_NodeNotFoundInPtree_ThrowsPtreeBadPath) {
    ptree pt;
    pt.put("some_register", 0x0);
    std::vector<std::pair<std::string, size_t>> name_size_mapping = { {"not_present", 8} };
    BOOST_CHECK_THROW(nsw::buildBitstream(name_size_mapping, pt), boost::property_tree::ptree_bad_path);
}

BOOST_AUTO_TEST_CASE(BuildPtreeFromVector_ValidInput_CreatesArrayInPtree) {
    std::vector<unsigned int> input = { 10, 94, 795, 585 };
    const ptree pt = nsw::buildPtreeFromVector(input);

    // Convert ptree back to vector
    std::vector<unsigned int> converted;
    for (const auto& pair : pt) {
        converted.push_back(pair.second.get_value<unsigned int>());
    }

    BOOST_TEST(input == converted);
}


BOOST_AUTO_TEST_CASE(BuildPtreeFromVector_EmptyInput_ReturnsEmptyPtree) {
    BOOST_TEST(nsw::buildPtreeFromVector(std::vector<unsigned int>{}).empty());
}

BOOST_AUTO_TEST_CASE(StripReadonly_EmptyString_ReturnsEmptyString) {
    BOOST_TEST(nsw::stripReadonly("") == "");
}

BOOST_AUTO_TEST_CASE(StripReadonly_ValidInput_RemovesReadonlySubstring) {
    BOOST_TEST(nsw::stripReadonly("_READONLY") == "");
    BOOST_TEST(nsw::stripReadonly("AAAA_READONLY") == "AAAA");
}

/* WILLFAIL: Function documentation states it strips _READONLY from *end* of string, not from any place it is found.
BOOST_AUTO_TEST_CASE(StripReadonly_ReadonlyNotAtEndOfString_DoesntRemoveReadonly) {
    BOOST_TEST(nsw::stripReadonly("AAAA_READONLYBBBB") == "AAAA_READONLY_BBBB");
}
*/

BOOST_AUTO_TEST_CASE(MatchRegexpInPtree_PtreeWithLevels_FindsAllMatches) {
    std::stringstream json;
    json << "{ \"level0_0\": {\"level1_0\": 0, \"level1_1\":1 },";
    json << " \"level0_1\": 0,";
    json << " \"level0_2\": {\"level1_0\": 0, \"level1_1\":1, \"level1_2\": {\"level2_0\":0 } } }";
    ptree pt;
    boost::property_tree::read_json(json, pt);

    {
        auto result = nsw::matchRegexpInPtree("level0.*", pt, "");
        std::set<std::string> matched = {"level0_0", "level0_1", "level0_2"};
        BOOST_TEST(result == matched);
    }

    {
        auto result = nsw::matchRegexpInPtree(".*level[0-9]_0", pt, "");
        std::set<std::string> matched = {"level0_0", "level0_0.level1_0",
                                         "level0_2.level1_0", "level0_2.level1_2.level2_0"};
        BOOST_TEST(result == matched);
    }
}
