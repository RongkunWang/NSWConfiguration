/// Test suite for testing ConfigReaderApi methods

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <set>

#define BOOST_TEST_MODULE translationtest
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/I2cRegisterMappings.h"


ptree createDummyAnalog()
{
    ptree tree;
    const std::string parentName{"ePllVmm0"};
    tree.put(parentName + ".ePllPhase40MHz_0", 120);
    tree.put(parentName + ".ePllPhase160MHz_0", 23);
    tree.put(parentName + ".ePllInstantLock", 1);
    tree.put(parentName + ".ePllReset", 1);
    tree.put(parentName + ".bypassPLL", 1);
    tree.put(parentName + ".ePllLockEn", 0);
    tree.put(parentName + ".ePllReferenceFrequency", 2);
    tree.put(parentName + ".ePllCap", 3);
    tree.put(parentName + ".ePllEnablePhase", 240);
    return tree;
}


ptree createDummyDigital()
{
    ptree tree;
    tree.put("rocId.l1_first", 1);
    tree.put("rocId.even_parity", 0);
    tree.put("rocId.roc_id", 37);
    tree.put("FIXME.busyOnLimit", 1590);
    return tree;
}


nsw::FEBConfig getReferenceConfig()
{
    const std::string file_path = "test_jsonapi.json";
    nsw::ConfigReader reader("json://" + file_path);
    const std::string name = "MMFE8-0001";
    return nsw::FEBConfig(reader.readConfig(name));
}



BOOST_AUTO_TEST_CASE(getRegisterBasedConfigAnalog) {
    const auto valueTree = createDummyAnalog();
    const auto converter = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converter.getRegisterBasedConfig();

    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase40MHz_0"), registerTree.get<int>("reg064ePllVmm0.ePllPhase40MHz_0"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase160MHz_0") / 16, registerTree.get<int>("reg064ePllVmm0.ePllPhase160MHz_0[4]"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase160MHz_0") % 16, registerTree.get<int>("reg068ePllVmm0.ePllPhase160MHz_0[3:0]"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllInstantLock"), registerTree.get<int>("reg070ePllVmm0.ePllInstantLock"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllReset"), registerTree.get<int>("reg070ePllVmm0.ePllReset"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.bypassPLL"), registerTree.get<int>("reg070ePllVmm0.bypassPLL"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllLockEn"), registerTree.get<int>("reg070ePllVmm0.ePllLockEn"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllReferenceFrequency"), registerTree.get<int>("reg070ePllVmm0.ePllReferenceFrequency"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllCap"), registerTree.get<int>("reg070ePllVmm0.ePllCap"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllEnablePhase"), registerTree.get<int>("reg072ePllVmm0.ePllEnablePhase"));
}


BOOST_AUTO_TEST_CASE(getRegisterBasedConfigNoSubregisterNoReadbackAnalog) {
    const auto valueTree = createDummyAnalog();
    const auto converter = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converter.getRegisterBasedConfigWithoutSubregisters(getReferenceConfig().getRocAnalog());

    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase40MHz_0"), registerTree.get<int>("reg064ePllVmm0") & 0b0111'1111);
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase160MHz_0") / 16, registerTree.get<int>("reg064ePllVmm0") & 0b1000'0000);
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase160MHz_0") % 16, registerTree.get<int>("reg068ePllVmm0") & 0b0000'1111);
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllInstantLock") +
                      valueTree.get<int>("ePllVmm0.ePllReset") +
                      valueTree.get<int>("ePllVmm0.bypassPLL") +
                      valueTree.get<int>("ePllVmm0.ePllLockEn") +
                      valueTree.get<int>("ePllVmm0.ePllReferenceFrequency") +
                      valueTree.get<int>("ePllVmm0.ePllCap"), registerTree.get<int>("reg070ePllVmm0"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllEnablePhase"), registerTree.get<int>("reg072ePllVmm0"));
}


BOOST_AUTO_TEST_CASE(translationNewToOldToNewAnalog) {
    const auto valueTree = createDummyAnalog();
    const auto converterValueToRegister = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converterValueToRegister.getRegisterBasedConfig();
    const auto converterRegisterToValue = ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::REGISTER_BASED);
    const auto valueTreeNew = converterRegisterToValue.getValueBasedConfig();

    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase40MHz_0"), valueTreeNew.get<int>("ePllVmm0.ePllPhase40MHz_0"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllPhase160MHz_0"), valueTreeNew.get<int>("ePllVmm0.ePllPhase160MHz_0"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllInstantLock"), valueTreeNew.get<int>("ePllVmm0.ePllInstantLock"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllReset"), valueTreeNew.get<int>("ePllVmm0.ePllReset"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.bypassPLL"), valueTreeNew.get<int>("ePllVmm0.bypassPLL"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllLockEn"), valueTreeNew.get<int>("ePllVmm0.ePllLockEn"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllReferenceFrequency"), valueTreeNew.get<int>("ePllVmm0.ePllReferenceFrequency"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllCap"), valueTreeNew.get<int>("ePllVmm0.ePllCap"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("ePllVmm0.ePllEnablePhase"), valueTreeNew.get<int>("ePllVmm0.ePllEnablePhase"));
}


BOOST_AUTO_TEST_CASE(getRegisterBasedConfigDigital) {
    const auto valueTree = createDummyDigital();
    const auto converter = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converter.getRegisterBasedConfig();

    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.l1_first"), registerTree.get<int>("reg000rocId.l1_first"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.even_parity"), registerTree.get<int>("reg000rocId.even_parity"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.roc_id"), registerTree.get<int>("reg000rocId.roc_id"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("FIXME.busyOnLimit") / 256, registerTree.get<int>("reg021busyOnLimit0.busy_on_limit[10:8]"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("FIXME.busyOnLimit") % 256, registerTree.get<int>("reg022busyOnLimit1.busy_on_limit[7:0]"));
}


BOOST_AUTO_TEST_CASE(getRegisterBasedConfigNoSubregisterNoReadbackDigital) {
    const auto valueTree = createDummyDigital();
    const auto converter = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converter.getRegisterBasedConfigWithoutSubregisters(getReferenceConfig().getRocDigital());

    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.l1_first"), registerTree.get<int>("reg000rocId") & 0b1000'0000);
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.even_parity"), registerTree.get<int>("reg000rocId") & 0b0100'0000);
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.roc_id"), registerTree.get<int>("reg000rocId") & 0b0011'1111);
    BOOST_CHECK_EQUAL(valueTree.get<int>("FIXME.busyOnLimit") / 256, registerTree.get<int>("reg021busyOnLimit0") & 0b0000'0111);
    BOOST_CHECK_EQUAL(valueTree.get<int>("FIXME.busyOnLimit") % 256, registerTree.get<int>("reg022busyOnLimit1"));
}


BOOST_AUTO_TEST_CASE(translationNewToOldToNewDigital) {
    const auto valueTree = createDummyDigital();
    const auto converterValueToRegister = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converterValueToRegister.getRegisterBasedConfig();
    const auto converterRegisterToValue = ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::REGISTER_BASED);
    const auto valueTreeNew = converterRegisterToValue.getValueBasedConfig();

    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.l1_first"), valueTreeNew.get<int>("rocId.l1_first"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.even_parity"), valueTreeNew.get<int>("rocId.even_parity"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("rocId.roc_id"), valueTreeNew.get<int>("rocId.roc_id"));
    BOOST_CHECK_EQUAL(valueTree.get<int>("FIXME.busyOnLimit"), valueTreeNew.get<int>("FIXME.busyOnLimit"));
}


BOOST_AUTO_TEST_CASE(translationFailuresAnalog) {
    const auto valueTree = createDummyAnalog();
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
    
    const auto converterValueToRegister = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converterValueToRegister.getRegisterBasedConfig();
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
}


BOOST_AUTO_TEST_CASE(translationFailuresDgital) {
    const auto valueTree = createDummyDigital();
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
    
    const auto converterValueToRegister = ConfigConverter(valueTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED);
    const auto registerTree = converterValueToRegister.getRegisterBasedConfig();
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::VALUE_BASED), std::exception);
    BOOST_CHECK_THROW(ConfigConverter(registerTree, ConfigConverter::RegisterAddressSpace::ROC_ANALOG, ConfigConverter::ConfigType::REGISTER_BASED), std::exception);
}
