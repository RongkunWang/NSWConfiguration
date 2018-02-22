// Test I2cFEConfig class and surrounding facilities.

#include <utility>
#include <string>
#include <sstream>
#include <memory>
#include <iostream>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"


#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/Types.h"

using boost::property_tree::ptree;

static const i2c::AddressRegisterMap CUSTOM_REGISTER_SIZE_1 = {
    { "i2caddress0", { {"reg0", 6}, {"reg1", 7}, {"reg2", 1}, {"reg3", 1}, {"reg4", 1} } },  // total: 16
    { "i2caddress1", { {"reg0", 15}, {"reg1", 15}, {"reg2", 8}, {"reg3", 2}, {"reg4", 8} } },  // total: 48
    { "i2caddress2", { {"reg0", 1}, {"reg1", 1}, {"reg2", 1}, {"reg3", 29 }} },  // total: 32
    { "i2caddress3", { {"reg0", 8} } }  // total: 8
};

#define BOOST_TEST_MODULE my test module
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

BOOST_AUTO_TEST_CASE(zero_test) {
    std::stringstream json;
    json << "{ \"i2caddress0\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress1\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0 },";
    json << "\"i2caddress3\": { \"reg0\":0 } }\n";

    ptree config;
    boost::property_tree::read_json(json, config);

    nsw::I2cMasterConfig master(config, "master_address", CUSTOM_REGISTER_SIZE_1);
    auto bs_map = master.getBitstreamMap();
    BOOST_TEST(bs_map["i2caddress0"] == "0000000000000000");
    BOOST_TEST(bs_map["i2caddress1"] == "000000000000000000000000000000000000000000000000");
    BOOST_TEST(bs_map["i2caddress2"] == "00000000000000000000000000000000");
    BOOST_TEST(bs_map["i2caddress3"] == "00000000");
}

BOOST_AUTO_TEST_CASE(nonzero_test) {
    std::stringstream json;
    json << "{ \"i2caddress0\": { \"reg0\":1, \"reg1\":1, \"reg2\":1, \"reg3\":1, \"reg4\":1 },";
    json << "\"i2caddress1\": { \"reg0\":15, \"reg1\":16, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0 },";
    json << "\"i2caddress3\": { \"reg0\":7 } }\n";

    ptree config;
    boost::property_tree::read_json(json, config);

    nsw::I2cMasterConfig master(config, "master_address", CUSTOM_REGISTER_SIZE_1);
    auto bs_map = master.getBitstreamMap();
    BOOST_TEST(bs_map["i2caddress0"] == "0000010000001111");
    BOOST_TEST(bs_map["i2caddress1"] == "000000000001111000000000010000000000000000000000");
    BOOST_TEST(bs_map["i2caddress2"] == "00000000000000000000000000000000");
    BOOST_TEST(bs_map["i2caddress3"] == "00000111");
}

BOOST_AUTO_TEST_CASE(get_value_test) {
    std::stringstream json;
    json << "{ \"i2caddress0\": { \"reg0\":1, \"reg1\":1, \"reg2\":1, \"reg3\":1, \"reg4\":1 },";
    json << "\"i2caddress1\": { \"reg0\":15, \"reg1\":16, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\": 5368709 },";
    json << "\"i2caddress3\": { \"reg0\":7 } }\n";

    ptree config;
    boost::property_tree::read_json(json, config);

    nsw::I2cMasterConfig master(config, "master_address", CUSTOM_REGISTER_SIZE_1);
    BOOST_TEST(master.getRegisterValue("i2caddress0", "reg0") == 1);
    BOOST_TEST(master.getRegisterValue("i2caddress1", "reg1") == 16);
    BOOST_TEST(master.getRegisterValue("i2caddress2", "reg3") == 5368709);
    BOOST_TEST(master.getRegisterValue("i2caddress3", "reg0") == 7);
}

BOOST_AUTO_TEST_CASE(set_value_test) {
    std::stringstream json;
    json << "{ \"i2caddress0\": { \"reg0\":1, \"reg1\":1, \"reg2\":1, \"reg3\":1, \"reg4\":1 },";
    json << "\"i2caddress1\": { \"reg0\":15, \"reg1\":16, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\": 5368709 },";
    json << "\"i2caddress3\": { \"reg0\":7 } }\n";

    ptree config;
    boost::property_tree::read_json(json, config);

    nsw::I2cMasterConfig master(config, "master_address", CUSTOM_REGISTER_SIZE_1);

    master.setRegisterValue("i2caddress0", "reg0", 11);
    master.setRegisterValue("i2caddress1", "reg1", 33);
    master.setRegisterValue("i2caddress2", "reg2", 1);
    master.setRegisterValue("i2caddress2", "reg3", 55555);
    master.setRegisterValue("i2caddress3", "reg0", 243);

    BOOST_TEST(master.getRegisterValue("i2caddress0", "reg0") == 11);
    BOOST_TEST(master.getRegisterValue("i2caddress1", "reg1") == 33);
    BOOST_TEST(master.getRegisterValue("i2caddress2", "reg2") == 1);
    BOOST_TEST(master.getRegisterValue("i2caddress2", "reg3") == 55555);
    BOOST_TEST(master.getRegisterValue("i2caddress3", "reg0") == 243);
}

BOOST_AUTO_TEST_CASE(overflow_test) {
    std::stringstream json;
    json << "{ \"i2caddress0\": { \"reg0\":1, \"reg1\":1, \"reg2\":1, \"reg3\":1, \"reg4\":2 },";
    json << "\"i2caddress1\": { \"reg0\":15, \"reg1\":16, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0 },";
    json << "\"i2caddress3\": { \"reg0\":7 } }\n";

    ptree config;
    boost::property_tree::read_json(json, config);

    BOOST_CHECK_THROW(nsw::I2cMasterConfig(config, "master_address", CUSTOM_REGISTER_SIZE_1), std::runtime_error);

    std::stringstream json2;
    json2 << "{ \"i2caddress0\": { \"reg0\":1, \"reg1\":1, \"reg2\":1, \"reg3\":1, \"reg4\":1 },";
    json2 << "\"i2caddress1\": { \"reg0\":15, \"reg1\":16, \"reg2\":0, \"reg3\":0, \"reg4\":0 },";
    json2 << "\"i2caddress2\": { \"reg0\":0, \"reg1\":0, \"reg2\":0, \"reg3\":0 },";
    json2 << "\"i2caddress3\": { \"reg0\":7 } }\n";

    ptree config2;
    boost::property_tree::read_json(json2, config2);

    nsw::I2cMasterConfig master(config2, "master_address", CUSTOM_REGISTER_SIZE_1);
    BOOST_CHECK_THROW(master.setRegisterValue("i2caddress0", "reg4", 2), std::runtime_error);
}