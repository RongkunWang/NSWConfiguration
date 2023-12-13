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
    BOOST_TEST(nsw::getElementType("A.RimL1DDC.B") == "RimL1DDC");
    BOOST_TEST(nsw::getElementType("A.L1DDC.B") == "L1DDC");
    BOOST_TEST(nsw::getElementType("A.PFEB.B") == "PFEB");
    BOOST_TEST(nsw::getElementType("A.SFEB_old.B") == "SFEB_old");
    BOOST_TEST(nsw::getElementType("A.SFEB8.B") == "SFEB8");
    BOOST_TEST(nsw::getElementType("A.SFEB6.B") == "SFEB6");
    BOOST_TEST(nsw::getElementType("A.SFEB.B") == "SFEB");
    BOOST_TEST(nsw::getElementType("A.TP.B") == "TP");
    BOOST_TEST(nsw::getElementType("A.ADDC.B") == "ADDC");
    BOOST_TEST(nsw::getElementType("A.PadTrigger.B") == "PadTrigger");
    BOOST_TEST(nsw::getElementType("A.Router.B") == "Router");
}

BOOST_AUTO_TEST_CASE(GetElementType_GeoIdInput_ReturnsElementName) {
    BOOST_TEST(nsw::getElementType("MM-A/V0/SCA/Strip/S9/L5/R3") == "MMFE8");
    BOOST_TEST(nsw::getElementType("sTGC-A/V0/TTC/Pad/S9/L0/R1") == "PFEB");
    BOOST_TEST(nsw::getElementType("sTGC-A/V0/from-SCA/Rim-L1DDC/S12/P/E0") == "RimL1DDC");
    BOOST_TEST(nsw::getElementType("sTGC-A/V0/from-SCA/L1DDC/S12/P/E0") == "L1DDC");
    // BOOST_TEST(nsw::getElementType("sTGC-A/V0/L1A/Strip/S9/L0/R0") == "SFEB8");
    // BOOST_TEST(nsw::getElementType("sTGC-A/V0/L1A/Strip/S9/L0/R2") == "SFEB6");
    BOOST_TEST(nsw::getElementType("MM-A/V0/from-SCA/ADDC/S9/L0/E") == "ADDC");
    BOOST_TEST(nsw::getElementType("sTGC-A/V0/Ext/PadTrig/S9/A") == "PadTrigger");
    BOOST_TEST(nsw::getElementType("sTGC-A/V0/TTC/Router/S9/L0/A") == "Router");
    BOOST_TEST(nsw::getElementType("sTGC-C/V0/SCA/TrigProc/S1/E0") == "STGCTP");
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
    [](const nsw::RegisterOverflow& exception) -> bool {
        std::string error{ exception.what() };
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

BOOST_AUTO_TEST_CASE(ByteVectorToWord32_LittleEndian_ReturnsLittleEndianWord32) {
    std::vector<std::uint8_t> data = { 0xde, 0xad, 0xbe, 0xef };
    std::uint32_t expected = 0xefbeadde;
    BOOST_TEST(nsw::byteVectorToWord32(data, true) == expected);
}

BOOST_AUTO_TEST_CASE(ByteVectorToWord32_BigEndian_ReturnsBigEndianWord32) {
    std::vector<std::uint8_t> data = { 0xde, 0xad, 0xbe, 0xef };
    std::uint32_t expected = 0xdeadbeef;
    BOOST_TEST(nsw::byteVectorToWord32(data, false) == expected);
}

BOOST_AUTO_TEST_CASE(ByteVectorToWord32_InvalidSize_ThrowsRuntimeError) {
    std::vector<std::uint8_t> data = { 0xde, 0xad, 0xbe, 0xef, 0x00 };
    BOOST_CHECK_THROW(nsw::byteVectorToWord32(data),
        std::runtime_error);
}

BOOST_AUTO_TEST_CASE(GuessSector_ValidInput_ReturnsCorrectSector) {
    BOOST_TEST(nsw::guessSector("191A-A06-MM-Calib") == "A06");
    BOOST_TEST(nsw::guessSector("P1-C15-MM-Calib") == "C15");
    BOOST_TEST(nsw::guessSector("NSW-STG-EA-S03-CalibApplication") == "A03");
}

BOOST_AUTO_TEST_CASE(GuessSector_InvalidInput_ReturnsEmpty) {
    BOOST_TEST(nsw::guessSector("notasector") == "");
}

BOOST_AUTO_TEST_CASE(TokenizeString_CommaSeparatedList_ReturnsCorrectWords) {
    const auto expected = std::vector<std::string>{"one", "two", "three"};
    BOOST_TEST(nsw::tokenizeString("one,two,three", ",") == expected);
}

BOOST_AUTO_TEST_CASE(IsLargeSector_LargeSector_ReturnsTrue) {
    BOOST_TEST(nsw::isLargeSector("A03") == true);
}

BOOST_AUTO_TEST_CASE(IsLargeSector_SmallSector_ReturnsFalse) {
    BOOST_TEST(nsw::isLargeSector("A12") == false);
}

BOOST_AUTO_TEST_CASE(IsLargeSector_InvalidSizeInput_ThrowsBadSectorName) {
    BOOST_CHECK_THROW(nsw::isLargeSector("asdf"), nsw::BadSectorName);
}

BOOST_AUTO_TEST_CASE(IsSmallSector_SmallSector_ReturnsTrue) {
    BOOST_TEST(nsw::isSmallSector("A02") == true);
}

BOOST_AUTO_TEST_CASE(IsSmallSector_LargeSector_ReturnsFalse) {
    BOOST_TEST(nsw::isSmallSector("A11") == false);
}

BOOST_AUTO_TEST_CASE(Contains_ContainsString_ReturnsTrue) {
    BOOST_TEST(nsw::contains("mystring", "str") == true);
}

BOOST_AUTO_TEST_CASE(Contains_NotContainsString_ReturnsFalse) {
    BOOST_TEST(nsw::contains("mystring", "unicorn") == false);
}

BOOST_AUTO_TEST_CASE(GetPathsFromPtree_ReturnsCorrectPaths) {
    ptree tree;
    const auto paths = std::vector<std::string>{"test", "one.two", "this.is.a.long.path", "this.not"};
    for (const auto& path : paths) {
        tree.put(path, "");
    }
    const auto result = nsw::getPathsFromPtree(tree);
    for (const auto& path : result) {
        const auto test = std::find(std::cbegin(paths), std::cend(paths), path) != std::cend(paths);
        BOOST_TEST(test);
    }
}

BOOST_AUTO_TEST_CASE(OverwriteBits_ValidInput_ReturnsCorrectResult) {
    BOOST_TEST(nsw::overwriteBits(0x00000000, 0xFF, 16, 8) == 0x00FF0000);
    BOOST_TEST(nsw::overwriteBits(0x00000000, 0xFF, 0, 8) == 0x000000FF);
    BOOST_TEST(nsw::overwriteBits(0x12345678, 0x0F, 16, 8) == 0x120F5678);
    BOOST_TEST(nsw::overwriteBits(0b00000011, 0b11, 4, 2) == 0b00110011);
    // BOOST_TEST(nsw::overwriteBits(0x00000000, 0xFFFFFFFF, 0, 32) == 0xFFFFFFFF);  // WILL FAIL
}

bool checkExceptionMessage(const auto& exception, const std::string_view expectation) {
    return exception.what() == expectation;
}

BOOST_AUTO_TEST_CASE(OverwriteBits_MoreBitsThanValue_ThrowsRuntimeError) {
    BOOST_CHECK_EXCEPTION(
        nsw::overwriteBits(0x00000000, 0xFF, 16, 7), std::runtime_error, [](const auto& execption) {
            return checkExceptionMessage(execption, "Value exceeds number of bits");
        });
}

BOOST_AUTO_TEST_CASE(OverwriteBits_PositionOutOfRange_ThrowsRuntimeError) {
    BOOST_CHECK_EXCEPTION(
        nsw::overwriteBits(0x00000000, 0xFF, 33, 8), std::runtime_error, [](const auto& execption) {
            return checkExceptionMessage(execption, "Position exceeds original");
        });
}

/* WILL FAIL
BOOST_AUTO_TEST_CASE(OverwriteBits_MoreBitsThanOriginal_ThrowsRuntimeError) {
    BOOST_CHECK_EXCEPTION(
        nsw::overwriteBits(0x00000000, 0xFF, 16, 33), std::runtime_error, [](const auto& execption) {
            return checkExceptionMessage(execption, "nbits exceeds original");
        });
}
*/

BOOST_AUTO_TEST_CASE(IntegerToByteVector_ValidInputNoSize_ReturnsCorrectResult) {
    const auto test = [] (auto&& val, std::vector<std::uint8_t>&& expected) {
        const auto result = nsw::integerToByteVector(val);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            std::cbegin(result), std::cend(result), std::cbegin(expected), std::cend(expected));
    };
    test(std::uint64_t{0xdeadbeefdeadbeef}, std::vector<std::uint8_t>{0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef});
    test(std::uint32_t{0xdeadbeef}, std::vector<std::uint8_t>{0xde, 0xad, 0xbe, 0xef});
    test(std::uint16_t{0xbeef}, std::vector<std::uint8_t>{0xbe, 0xef});
    test(std::uint8_t{0xbe}, std::vector<std::uint8_t>{0xbe});
}

BOOST_AUTO_TEST_CASE(IntegerToByteVector_ValidInputSize_ReturnsCorrectResult) {
    const auto test = [] (auto&& val, std::size_t&& size, std::vector<std::uint8_t>&& expected) {
        const auto result = nsw::integerToByteVector(val, size);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            std::cbegin(result), std::cend(result), std::cbegin(expected), std::cend(expected));
    };
    test(std::uint32_t{0xdeadbeef}, 4, std::vector<std::uint8_t>{0xde, 0xad, 0xbe, 0xef});
    test(std::uint32_t{0xdeadbeef}, 2, std::vector<std::uint8_t>{0xbe, 0xef});
}

BOOST_AUTO_TEST_CASE(IntegerToByteVector_InvalidInputSize_ThrowsLogicError) {
    BOOST_CHECK_THROW(nsw::integerToByteVector(std::uint32_t{0xdeadbeef}, 5), std::logic_error);
}

BOOST_AUTO_TEST_CASE(DumpTree_ValidTree_CorrectString) {
    ptree tree;
    tree.put("test", "test");
    tree.put("one.two", 1.2);
    const auto result = fmt::format("{}\n{}\n{}\n{}\n{}\n{}\n",
        "{",
        "    \"test\": \"test\",",
        "    \"one\": {",
        "        \"two\": \"1.2\"",
        "    }",
        "}"
    );
    BOOST_TEST(nsw::dumpTree(tree) == result);
}

BOOST_AUTO_TEST_CASE(GetSetFromPtree_ValidInput_CorrectSet) {
    const auto expected = std::set<std::string>{"ptree", "to", "set", "without", "duplicates", "duplicates"};
    ptree tree;
    for (const auto& value : expected) {
        boost::property_tree::ptree child;
        child.put("", value);
        tree.push_back(std::make_pair("", child));
    }
    const auto result = nsw::getSetFromPtree<std::string>(tree, "");
    BOOST_CHECK_EQUAL_COLLECTIONS(std::cbegin(result), std::cend(result), std::cbegin(expected), std::cend(expected));
}

BOOST_AUTO_TEST_CASE(GetPathsFromPtree_ValidInput_CorrectSet) {
    const auto expected = std::unordered_set<std::string>{"ptree", "with", "possibly.nested", "s.t.r.u.c.t.u.r.e.s"};
    ptree tree;
    for (const auto& value : expected) {
        tree.put(value, 1);
    }
    const auto result = nsw::getPathsFromPtree(tree);
    for (const auto& path : result) {
        const auto test = std::find(std::cbegin(expected), std::cend(expected), path) != std::cend(expected);
        BOOST_TEST(test);
    }
}

BOOST_AUTO_TEST_CASE(TransformMapToPtree_ValidInput_CorrectSet) {
    const auto input = std::map<std::string, int>{{"test", 1}, {"one.two", 2}}; 
    ptree expected;
    expected.put("test", 1);
    expected.put("one.two", 2);
    const auto result = nsw::transformMapToPtree(input);
    for (const auto& [path, value] : input) {
        const auto test = expected.get<int>(path) == value;
        BOOST_TEST(test);
    }
}

BOOST_AUTO_TEST_CASE(TransformPtreetoMap_ValidInput_CorrectSet) {
    ptree input;
    input.put("test", 1);
    input.put("one.two", 2);
    const auto expected = std::map<std::string, int>{{"test", 1}, {"one.two", 2}}; 
    const auto result = nsw::transformPtreetoMap<int>(input);
    for (const auto& [path, value] : result) {
        const auto test = expected.at(path) == value;
        BOOST_TEST(test);
    }
}
