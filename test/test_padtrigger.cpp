#define BOOST_TEST_MODULE padtrigger
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/DeviceManager.h"
constexpr std::size_t ONE{1};

BOOST_AUTO_TEST_CASE(PadTrigger_checkPFEBBCIDs) {
  nsw::hw::DeviceManager deviceManager;
  nsw::ConfigReader reader("json://test_padtrigger.json");
  for (const auto& name : reader.getAllElementNames()) {
    deviceManager.add(reader.readConfig(name));
  }
  BOOST_TEST(deviceManager.getPadTriggers().size() == ONE);
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

BOOST_AUTO_TEST_CASE(PadTrigger_getViableBcids) {
  nsw::hw::DeviceManager deviceManager;
  nsw::ConfigReader reader("json://test_padtrigger.json");
  for (const auto& name : reader.getAllElementNames()) {
    deviceManager.add(reader.readConfig(name));
  }
  BOOST_TEST(deviceManager.getPadTriggers().size() == ONE);
  const auto empty = BcidVector{};
  const auto zeros = BcidVector{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const auto align = BcidVector{7,7,7,7,7,7,6,6,6,6,6,6,5,5,5,5,5,5,4,4,4,4,4,4};
  const auto split = BcidVector{6,6,6,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,3,2,2,2};
  const auto rollo = BcidVector{1,1,1,1,1,1,0,0,0,0,0,0,15,15,15,15,15,15,14,14,14,14,14,14};
  for (const auto& dev: deviceManager.getPadTriggers()) {
    BOOST_TEST((dev.getViableBcids(empty) == BcidVector{}));
    BOOST_TEST((dev.getViableBcids(zeros) == BcidVector{}));
    BOOST_TEST((dev.getViableBcids(align) == BcidVector{4, 5, 6, 7}));
    BOOST_TEST((dev.getViableBcids(split) == BcidVector{3, 4, 5}));
    BOOST_TEST((dev.getViableBcids(rollo) == BcidVector{0, 1, 14, 15}));
  }
}
