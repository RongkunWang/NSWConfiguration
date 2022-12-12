#define BOOST_TEST_MODULE FebHw
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/DeviceManager.h"

BOOST_AUTO_TEST_CASE(VmmGetVmmId_ReturnsCorrectValue) {
  const auto mmfeName  = "MMFE8-0001";
  const auto pfebName  = "PFEB-0001";
  const auto sfeb8Name = "SFEB-0001";
  const auto sfeb6Name = "SFEB6-0001";
  nsw::hw::DeviceManager deviceManager;
  nsw::ConfigReader reader("json://test_jsonapi.json");
  for (const auto& name : reader.getAllElementNames()) {
    if (name != mmfeName and
        name != pfebName and
        name != sfeb8Name and
        name != sfeb6Name) {
      continue;
    }
    deviceManager.add(reader.readConfig(name));
  }
  BOOST_TEST(deviceManager.getFebs().size() == 4);
  for (const auto& feb: deviceManager.getFebs()) {
    const std::size_t offset = (feb.isSFEB6()) ? nsw::SFEB6_FIRST_VMM : 0;
    for (std::size_t it = offset; it < offset + feb.getNumVmms(); ++it) {
      BOOST_TEST(feb.getVmm(it).getVmmId() == it);
    }
  }
}

