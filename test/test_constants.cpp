#define BOOST_TEST_MODULE Constants
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/Constants.h"

BOOST_AUTO_TEST_CASE(Constants_CorrectNumberOfPfebVmms) {
  BOOST_TEST(nsw::NUM_PAD_VMM_PER_PFEB == 2);
}
