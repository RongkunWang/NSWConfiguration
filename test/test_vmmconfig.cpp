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

BOOST_AUTO_TEST_CASE(GetChannelRegisterOneChannel_CorrectChannelAndRegister_ReturnsChannelRegisterValue) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sc", 0) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sc", 1) == 0);

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 0) == 1);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_st", 1) == 1);

    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 0) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 1) == 0);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 2) == 3);
    BOOST_TEST(vmm0.getChannelRegisterOneChannel("channel_sz10b", 3) == 0);

    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b_WRONG", 3), nsw::NoSuchVmmRegister);
    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b", 64), nsw::VmmChannelOutOfRange);
}

BOOST_AUTO_TEST_CASE(GetChannelRegisterOneChannel_WrongRegisterName_ThrowsNoSuchRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b_WRONG", 3), nsw::NoSuchVmmRegister);
}

BOOST_AUTO_TEST_CASE(GetChannelRegisterOneChannel_InvalidChannel_ThrowsVmmChannelOutOfRange) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.getChannelRegisterOneChannel("channel_sz10b", 64), nsw::VmmChannelOutOfRange);
}

BOOST_AUTO_TEST_CASE(GetGlobalRegister_CorrectRegisterName_ReturnsGlobalRegisterValue) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 400);
}

BOOST_AUTO_TEST_CASE(GetGlobalRegister_WrongRegisterName_ThrowsNoSuchVmmRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.getGlobalRegister("notarealregister"), nsw::NoSuchVmmRegister);
}

BOOST_AUTO_TEST_CASE(GetChannelRegisterAllChannels_CorrectChannelRegisterName_ReturnsChannelRegisterValueArray) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    std::vector<uint32_t> expected_chmap = { 0, 0, 3, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0 };
    BOOST_TEST(vmm0.getChannelRegisterAllChannels("channel_sz10b") == expected_chmap);
}

BOOST_AUTO_TEST_CASE(GetChannelRegisterAllChannels_WrongChannelRegisterName_ThrowsNoSuchVmmRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.getChannelRegisterAllChannels("notarealregister"), nsw::NoSuchVmmRegister);
}

BOOST_AUTO_TEST_CASE(SetGlobalRegister_CorrectRegisterName_SetsRegisterValue) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));

    vmm0.setGlobalRegister("sdp_dac", 10);
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 10);

    vmm0.setGlobalRegister("sdp_dac", 111);
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 111);
}

BOOST_AUTO_TEST_CASE(SetGlobalRegister_WrongRegisterName_ThrowsNoSuchVmmRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.setGlobalRegister("sp_non_existant", 1), nsw::NoSuchVmmRegister);
}

BOOST_AUTO_TEST_CASE(SetChannelRegisterAllChannels_CorrectRegisterName_SetsAllChannelsValues) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setChannelRegisterAllChannels("channel_sd", 1);    // Set sd for all channels to 1
    std::vector<uint32_t> expected_chmap(64, 1);            // vector of 64 ones
    BOOST_TEST(vmm0.getChannelRegisterAllChannels("channel_sd") == expected_chmap);
}

/* WILLFAIL
BOOST_AUTO_TEST_CASE(SetChannelRegisterAllChannels_WrongRegisterName_ThrowsNoSuchVmmChannel) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.setChannelRegisterAllChannels("notarealregister", 1), nsw::NoSuchVmmRegister);
}
*/

BOOST_AUTO_TEST_CASE(SetChannelRegisterOneChannel_CorrectRegisterAndChannel_SetsChannelsRegisterValue) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));

    std::vector<uint32_t> expected_chmap(64, 1);
    expected_chmap[53] = 0;

    vmm0.setChannelRegisterOneChannel("channel_st", 0, 53);  // Set st for channel 53 to 0
    BOOST_TEST(vmm0.getChannelRegisterAllChannels("channel_st") == expected_chmap);
}

BOOST_AUTO_TEST_CASE(SetChannelRegisterOneChannel_WrongRegisterName_ThrowsNoSuchVmmRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.setChannelRegisterOneChannel("notarealregister", 0, 53), nsw::NoSuchVmmRegister);
}

BOOST_AUTO_TEST_CASE(SetChannelRegisterOneChannel_InvalidChannel_ThrowsVmmChannelOutOfRange) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    BOOST_CHECK_THROW(vmm0.setChannelRegisterOneChannel("channel_sd", 0, 64), nsw::VmmChannelOutOfRange);
    BOOST_CHECK_THROW(vmm0.setChannelRegisterOneChannel("channel_sd", 0, -1), nsw::VmmChannelOutOfRange);
}

BOOST_AUTO_TEST_CASE(SetTestPulseDAC__SetsSdpDacGlobalRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setTestPulseDAC(512);
    BOOST_TEST(vmm0.getGlobalRegister("sdp_dac") == 512);
}

BOOST_AUTO_TEST_CASE(SetGlobalThreshold__SetsSdtDacGlobalRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setGlobalThreshold(128);
    BOOST_TEST(vmm0.getGlobalRegister("sdt_dac") == 128);
}

// TODO: Should probably have tests in the function to validate input!
BOOST_AUTO_TEST_CASE(SetMonitorOutput__SetsScmxAndSmGlobalRegisters) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setMonitorOutput(4, 0);
    BOOST_TEST(vmm0.getGlobalRegister("scmx") == 0);
    BOOST_TEST(vmm0.getGlobalRegister("sm") == 4);
}

BOOST_AUTO_TEST_CASE(SetChannelMOMode__SetsSmxChannelRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setChannelMOMode(32, 1);

    std::vector<uint32_t> expected_chmap(64, 0);
    expected_chmap[32] = 1;

    BOOST_TEST(vmm0.getChannelRegisterAllChannels("channel_smx") == expected_chmap);
}

BOOST_AUTO_TEST_CASE(SetChannelTrimmer__SetsSdChannelRegister) {
    ptree config;
    boost::property_tree::read_json("test_vmm.json", config);
    nsw::VMMConfig vmm0(config.get_child("vmm0"));
    vmm0.setChannelTrimmer(32, 30);

    std::vector<uint32_t> expected_chmap(64, 0);
    expected_chmap[32] = 30;

    BOOST_TEST(vmm0.getChannelRegisterAllChannels("channel_sd") == expected_chmap);
}