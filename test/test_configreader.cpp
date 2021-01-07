#define BOOST_TEST_MODULE ConfigReader_tests
#define BOOST_TEST_DYN_LINK

#include "boost/test/unit_test.hpp"

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/FEBConfig.h"

BOOST_AUTO_TEST_CASE(Constructor_PrefixNotInBeginningOfPath_ThrowsConfigIssue) {
    // Path prefix found, but in wrong string position
    BOOST_CHECK_THROW(nsw::ConfigReader("AAAAxml://BBBB"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("AAAAjson://BBBB"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("AAAAoksconfig:BBBB"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("AAAAoracle:BBBB"), nsw::ConfigIssue);
}

BOOST_AUTO_TEST_CASE(Constructor_PrefixNotInPath_ThrowsConfigIssue) {
    // Path prefix not present
    BOOST_CHECK_THROW(nsw::ConfigReader("thisisaninvalidpath"), nsw::ConfigIssue);
}

/* WILLFAIL
BOOST_AUTO_TEST_CASE(Constructor_EmptyPathExceptForPrefix_ThrowsConfigIssue) {
    // Path prefix found, but path itself is empty
    BOOST_CHECK_THROW(nsw::ConfigReader("xml://"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("json://"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("oksconfig:"), nsw::ConfigIssue);
    BOOST_CHECK_THROW(nsw::ConfigReader("oracle:"), nsw::ConfigIssue);
}
*/

BOOST_AUTO_TEST_CASE(Constructor_EmptyPath_ThrowConfigIssue) {
    BOOST_CHECK_THROW(nsw::ConfigReader(""), nsw::ConfigIssue);
}
