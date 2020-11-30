/// Test suite for testing VMMCodec and VMMConfig classes
/// This tests require test_vmm.json file, so if you change the file, you also need to change the tests

#include <iostream>
#include <string>
#include <algorithm>

#define BOOST_TEST_MODULE VMMConfig_tests
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/VMMCodec.h"

BOOST_AUTO_TEST_CASE(getter_test) {
    std::string file_path = "test_vmm.json";
    ptree config;
    boost::property_tree::read_json(file_path, config);

    ptree config0 = config.get_child("vmm0");
    // write_json(std::cout, config0);

    nsw::VMMConfig vmm0(config0);

    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 400);

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sc", 0) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sc", 1) == 0);

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 0) == 1);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 1) == 1);

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 0) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 1) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 2) == 3);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 3) == 0);

    auto chmap = vmm0.getChannelRegisterAllChannels("channel_sz10b");
    BOOST_TEST(chmap[1] == 0);
    BOOST_TEST(chmap[2] == 3);
    BOOST_TEST(chmap[3] == 0);

    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b_WRONG", 3), nsw::NoSuchVmmRegister);
    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b", 64), nsw::VmmChannelOutOfRange);
}

BOOST_AUTO_TEST_CASE(setter_test) {
    std::string file_path = "test_vmm.json";
    ptree config;
    boost::property_tree::read_json(file_path, config);

    ptree config0 = config.get_child("vmm0");
    // write_json(std::cout, config0);

    nsw::VMMConfig vmm0(config0);

    vmm0.setGlobalRegister("sdp_dac", 10);
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 10);

    vmm0.setGlobalRegister("sdp_dac", 111);
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 111);

    BOOST_CHECK_THROW(vmm0.setGlobalRegister("sp_non_existant", 1), nsw::NoSuchVmmRegister);

    vmm0.setChannelRegisterAllChannels("channel_sd", 1);  // Set sd for all channels to 1
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sd", 0) == 1);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sd", 1) == 1);

    vmm0.setChannelRegisterOneChannel("channel_st", 0, 53);  // Set st for channel 53 to 0
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 53) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 52) == 1);

    auto bytes_before = vmm0.getByteVector();

    vmm0.setChannelRegisterOneChannel("channel_sz10b", 30, 5);  // Set channel 5 to 254
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 1) == 0);  // Check other elements didn't change
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 2) == 3);  // Check other elements didn't change
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 5) == 30);

    auto bytes_after = vmm0.getByteVector();

    // Check content of 2 vector not equal
    BOOST_TEST(!std::equal(bytes_before.begin(), bytes_before.end(), bytes_after.begin()));

    // Restore the initial value
    vmm0.setChannelRegisterOneChannel("channel_sz10b", 0, 5);  // Set channel 5 to 0

    auto bytes_restored = vmm0.getByteVector();

    BOOST_TEST(std::equal(bytes_before.begin(), bytes_before.end(), bytes_restored.begin()));

    vmm0.setChannelRegisterAllChannels("channel_sz10b", 30);

    bytes_after = vmm0.getByteVector();

    BOOST_TEST(!std::equal(bytes_before.begin(), bytes_before.end(), bytes_after.begin()));

    vmm0.setChannelRegisterAllChannels("channel_sz10b", 0);
    vmm0.setChannelRegisterOneChannel("channel_sz10b", 3, 2);

    bytes_restored = vmm0.getByteVector();

    BOOST_TEST(std::equal(bytes_before.begin(), bytes_before.end(), bytes_restored.begin()));
}

