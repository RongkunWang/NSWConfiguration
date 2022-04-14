#define BOOST_TEST_MODULE SCAGeoIdentifier
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/SCAGeoIdentifier.h"

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesMMFE8) {
    const auto info = nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/L0/R14");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::MM);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::A);
    BOOST_CHECK_EQUAL(info.sector(), 9);
    BOOST_CHECK_EQUAL(info.layer(), 0);
    BOOST_CHECK_EQUAL(info.radius(), 14);
    BOOST_CHECK_EQUAL(info.resourceType(), "MMFE8");
    BOOST_CHECK_EQUAL(info.isMM(), true);
    BOOST_CHECK_EQUAL(info.isSTGC(), false);
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesPFEB) {
    const auto info = nsw::SCAGeoIdentifier("sTGC-C/V0/SCA/Pad/S3/L2/R1");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::STGC);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::C);
    BOOST_CHECK_EQUAL(info.sector(), 3);
    BOOST_CHECK_EQUAL(info.layer(), 2);
    BOOST_CHECK_EQUAL(info.radius(), 1);
    BOOST_CHECK_EQUAL(info.resourceType(), "PFEB");
    BOOST_CHECK_EQUAL(info.isMM(), false);
    BOOST_CHECK_EQUAL(info.isSTGC(), true);
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesSFEB) {
    const auto info = nsw::SCAGeoIdentifier("sTGC-A/V0/SCA/Strip/S3/L5/R1");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::STGC);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::A);
    BOOST_CHECK_EQUAL(info.sector(), 3);
    BOOST_CHECK_EQUAL(info.layer(), 5);
    BOOST_CHECK_EQUAL(info.radius(), 1);
    BOOST_CHECK_EQUAL(info.resourceType(), "SFEB");
    BOOST_CHECK_EQUAL(info.isMM(), false);
    BOOST_CHECK_EQUAL(info.isSTGC(), true);
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesL1DDCMMG) {
    const auto info = nsw::SCAGeoIdentifier("MM-A/V0/SCA/L1DDC/S9/L0/O");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::MM);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::A);
    BOOST_CHECK_EQUAL(info.sector(), 9);
    BOOST_CHECK_EQUAL(info.layer(), 0);
    BOOST_CHECK_EQUAL(info.radius(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.resourceType(), "L1DDC");
    BOOST_CHECK_EQUAL(info.isMM(), true);
    BOOST_CHECK_EQUAL(info.isSTGC(), false);
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesPadTrigger) {
    const auto info = nsw::SCAGeoIdentifier("sTGC-A/V0/SCA/PadTrig/S3/P");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::STGC);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::A);
    BOOST_CHECK_EQUAL(info.sector(), 3);
    BOOST_CHECK_EQUAL(info.layer(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.radius(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.resourceType(), "PadTrig");
    BOOST_CHECK_EQUAL(info.isMM(), false);
    BOOST_CHECK_EQUAL(info.isSTGC(), true);
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ReturnsCorrectPropertiesNoGeoId) {
    const auto info = nsw::SCAGeoIdentifier("MMFE8_L1P1_IPL");

    BOOST_CHECK(info.detector() == nsw::geoid::Detector::UNKNOWN);
    BOOST_CHECK(info.wheel() == nsw::geoid::Wheel::UNKNOWN);
    BOOST_CHECK_EQUAL(info.sector(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.layer(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.radius(), nsw::geoid::DoesNotExist);
    BOOST_CHECK_EQUAL(info.resourceType(), "MMFE8");
    BOOST_CHECK_EQUAL(info.isMM(), false);
    BOOST_CHECK_EQUAL(info.isSTGC(), false);
}

bool checkException(const nsw::NSWSCAGeoIdentifierIssue& ex, const std::string& pred)
{
  return ex.what() == pred;
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsEmptyNodeId) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier(""),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Empty OPC node ID"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsInvalidDetector) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("sTG-A/V0/SCA/PadTrig/S3/P"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Cannot get detector from sTG-A"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsInvalidWheel) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("sTGC-B/V0/SCA/PadTrig/S3/P"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Cannot get wheel from sTGC-B"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsInvalidPosition) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Did not find entry number 5 in MM-A/V0/SCA/Strip/S9"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/L5"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Did not find entry number 6 in MM-A/V0/SCA/Strip/S9/L5"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsInvalidPrefix) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/X9/L5/R2"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Element number 4 (X9) does not start with S as requested"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/X5/R2"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Element number 5 (X5) does not start with L as requested"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/L5/X2"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "Element number 6 (X2) does not start with R as requested"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsInvalidNumber) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("sTGC-A/V0/SCA/PadTrig/SNotANumber"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "NotANumber is not a valid number (from sTGC-A/V0/SCA/PadTrig/SNotANumber)"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsNumberOutOfRange) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("sTGC-A/V0/SCA/PadTrig/S2154635121352123"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "2154635121352123 is too large for 8 bit int (from S2154635121352123)"); });
}

BOOST_AUTO_TEST_CASE(SCAGeoIdentifier_ThrowsNumberToLarge) {
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S16/L5/R2"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "16 is too large given the maximum of 15 (from S16)"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/L8/R2"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "8 is too large given the maximum of 7 (from L8)"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("MM-A/V0/SCA/Strip/S9/L5/R16"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "16 is too large given the maximum of 15 (from R16)"); });
  BOOST_CHECK_EXCEPTION(nsw::SCAGeoIdentifier("sTGC-A/V0/SCA/Strip/S9/L5/R3"),
                        nsw::NSWSCAGeoIdentifierIssue,
                        [](const auto& exception) { return checkException(exception, "3 is too large given the maximum of 2 (from R3)"); });
}
