#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <set>

#define BOOST_TEST_MODULE JsonApi_tests
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/ConfigReaderJsonApi.h"

BOOST_AUTO_TEST_CASE(GetAllElementNames_FullConfigTree_ReturnsAllElements) {
    const std::string file_path = "test_jsonapi.json";
    JsonApi cfg(file_path);
    auto elements_read = cfg.getAllElementNames();

    std::set<std::string> all_elements{ "MMFE8-0001", "PFEB-0001", "SFEB-0001", "SFEB6-0001",
                          "A01.SFEB-0001", "A01.SFEB-0002",
                          "A02.Layer0.MMFE8-0001",
                          "A02.Layer1.MMFE8-0001", "A02.Layer1.PFEB-0001" };
    BOOST_TEST(elements_read == all_elements);
}

BOOST_AUTO_TEST_CASE(GetElementNames_FullConfigTree_MatchesRegex) {
    const std::string file_path = "test_jsonapi.json";
    JsonApi cfg(file_path);

    {
      auto elements_read = cfg.getElementNames(".*MMFE8.*");
      std::set<std::string> mmfe8_elements{ "MMFE8-0001",
                            "A02.Layer0.MMFE8-0001",
                            "A02.Layer1.MMFE8-0001" };
      BOOST_TEST(elements_read == mmfe8_elements);
    }

    {
      auto elements_read = cfg.getElementNames(R"(A02\.Layer0\..*)");
      std::set<std::string> a02_layer0_elements{ "A02.Layer0.MMFE8-0001" };
      BOOST_TEST(elements_read == a02_layer0_elements);
    }

    {
      auto elements_read = cfg.getElementNames(R"(A02\.Layer1\..*)");
      std::set<std::string> a02_layer1_elements{ "A02.Layer1.MMFE8-0001", "A02.Layer1.PFEB-0001" };
      BOOST_TEST(elements_read == a02_layer1_elements);
    }
}
