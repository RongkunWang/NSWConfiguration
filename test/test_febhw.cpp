#define BOOST_TEST_MODULE FebHw
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/DeviceManager.h"

BOOST_AUTO_TEST_CASE(VmmGetVmmId_ReturnsCorrectValue) {
  const auto mmfeName = "MMFE8-0001";
  nsw::hw::DeviceManager deviceManager;
  nsw::ConfigReader reader("json://test_jsonapi.json");
  for (const auto& name : reader.getAllElementNames()) {
    if (name != mmfeName) {
      continue;
    }
    deviceManager.add(reader.readConfig(name));
  }
  BOOST_TEST(deviceManager.getFebs().size() == 1);
  for (const auto& feb: deviceManager.getFebs()) {
    for (std::size_t it = 0; it < feb.getNumVmms(); ++it) {
      BOOST_TEST(feb.getVmm(it).getVmmId() == it);
    }
  }
}

