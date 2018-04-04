/// Test suite for testing VMMCodec and VMMConfig classes
/// This tests require test_vmm.json file, so if you change the file, you also need to change the tests

#include <iostream>
#include <string>

#define BOOST_TEST_MODULE my test module
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
}

