/// Test suite for testing ConfigReaderApi methods

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <set>

#define BOOST_TEST_MODULE ConfigReader_tests
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "NSWConfiguration/ConfigReader.h"

std::set<std::string> vector2set(std::vector<std::string> vec) {
  return std::set<std::string>(vec.begin(), vec.end());
}

BOOST_AUTO_TEST_CASE(getAllElementNames_test) {
    std::string file_path = "test_configreader.json";

    nsw::ConfigReader cfg("json://" + file_path);

    auto elements = cfg.getAllElementNames();

    std::set<std::string> s2 = {"MMFE8-0001", "PFEB-0001", "SFEB-0001",
                          "A01.SFEB-0001", "A01.SFEB-0002",
                          "A02.Layer0.MMFE8-0001",
                          "A02.Layer1.MMFE8-0001", "A02.Layer1.PFEB-0001"};

    BOOST_TEST(elements == s2);
}


BOOST_AUTO_TEST_CASE(getElementNames_test) {
    std::string file_path = "test_configreader.json";

    nsw::ConfigReader cfg("json://" + file_path);

    {
      auto elements = cfg.getElementNames(".*MMFE8.*");

      std::set<std::string> s2 = {"MMFE8-0001",
                            "A02.Layer0.MMFE8-0001",
                            "A02.Layer1.MMFE8-0001"};
      BOOST_TEST(elements == s2);
    }

    {
      auto elements = cfg.getElementNames("A02\.Layer0.*");

      std::set<std::string> s2 = { "A02.Layer0.MMFE8-0001"};
      BOOST_TEST(elements == s2);
    }

    {
      auto elements = cfg.getElementNames("A0.*\.Layer1.*");

      std::set<std::string> s2 = { "A02.Layer1.MMFE8-0001", "A02.Layer1.PFEB-0001"};
      BOOST_TEST(elements == s2);
    }
}
