/// Test suite for testing VMMCodec class

#include <iostream>

#define BOOST_TEST_MODULE my test module
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

void check_something() {
    int a = 15;
    int b = 15;
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(dummy_test) {
    check_something();
}

