#define BOOST_TEST_MODULE padtrigger
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/DeviceManager.h"

BOOST_AUTO_TEST_CASE(PadTrigger_checkPFEBBCIDs) {
  nsw::hw::DeviceManager deviceManager;
  nsw::ConfigReader reader("json://test_padtrigger.json");
  for (const auto& name : reader.getAllElementNames()) {
    deviceManager.add(reader.readConfig(name));
  }
  BOOST_TEST(deviceManager.getPadTriggers().size() == 1);
  const auto empty = BcidVector{};
  const auto zeros = BcidVector{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const auto fives = BcidVector{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
  const auto nines = BcidVector{9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9};
  const auto incre = BcidVector{0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3};
  const auto decre = BcidVector{7,7,7,7,7,7,6,6,6,6,6,6,5,5,5,5,5,5,4,4,4,4,4,4};
  for (const auto& dev: deviceManager.getPadTriggers()) {
    BOOST_TEST(dev.checkPFEBBCIDs(empty) == false);
    BOOST_TEST(dev.checkPFEBBCIDs(zeros) == false);
    BOOST_TEST(dev.checkPFEBBCIDs(fives) == false);
    BOOST_TEST(dev.checkPFEBBCIDs(nines) == false);
    BOOST_TEST(dev.checkPFEBBCIDs(incre) == false);
    BOOST_TEST(dev.checkPFEBBCIDs(decre) == true);
  }
}

