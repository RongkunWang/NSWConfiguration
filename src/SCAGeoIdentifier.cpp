#include "NSWConfiguration/SCAGeoIdentifier.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string>

#include <ers/ers.h>
#include <string_view>

#include "NSWConfiguration/Utility.h"

nsw::SCAGeoIdentifier::SCAGeoIdentifier(std::string scaAddress) :
  m_scaAddress{std::move(scaAddress)},
  m_elements{nsw::tokenizeString(m_scaAddress, DELIMITER)},
  m_noGeoId{m_scaAddress.find(DELIMITER) == std::string::npos},
  m_detector{extractDetector()},
  m_wheel{extractWheel()},
  m_resourceType{getElementType(m_scaAddress)},
  m_sector{getValue(PREFIX_SECTOR, POSITION_SECTOR, nsw::NUM_SECTORS, true)},
  m_layer{getValue(PREFIX_LAYER, POSITION_LAYER, nsw::NUM_LAYERS_PER_TECH, layerRequired())},
  m_radius{
    getValue(PREFIX_RADIUS,
             POSITION_RADIUS,
             (m_detector == nsw::geoid::Detector::MM) ? nsw::NUM_RADII_MM : nsw::NUM_RADII_STGC,
             radiusRequired())}
{}

bool nsw::SCAGeoIdentifier::isMM() const
{
  warnNoGeoId();
  return m_detector == nsw::geoid::Detector::MM;
}

bool nsw::SCAGeoIdentifier::isSTGC() const
{
  warnNoGeoId();
  return m_detector == nsw::geoid::Detector::STGC;
}

nsw::geoid::Detector nsw::SCAGeoIdentifier::extractDetector() const
{
  if (m_elements.empty() or m_scaAddress.empty()) {
    throw nsw::NSWSCAGeoIdentifierIssue(ERS_HERE, "Empty OPC node ID");
  }
  if (m_noGeoId) {
    return nsw::geoid::Detector::UNKNOWN;
  }
  if (nsw::contains(m_elements.at(0), DETECTOR_NAME_MM)) {
    return nsw::geoid::Detector::MM;
  }
  if (nsw::contains(m_elements.at(0), DETECTOR_NAME_STGC)) {
    return nsw::geoid::Detector::STGC;
  }
  throw nsw::NSWSCAGeoIdentifierIssue(ERS_HERE,
                                      fmt::format("Cannot get detector from {}", m_elements.at(0)));
}

nsw::geoid::Wheel nsw::SCAGeoIdentifier::extractWheel() const
{
  if (m_elements.empty()) {
    throw nsw::NSWSCAGeoIdentifierIssue(ERS_HERE, "Empty OPC node ID");
  }
  if (m_noGeoId) {
    return nsw::geoid::Wheel::UNKNOWN;
  }
  if (nsw::contains(m_elements.at(0), WHEEL_NAME_A)) {
    return nsw::geoid::Wheel::A;
  }
  if (nsw::contains(m_elements.at(0), WHEEL_NAME_C)) {
    return nsw::geoid::Wheel::C;
  }
  throw nsw::NSWSCAGeoIdentifierIssue(ERS_HERE,
                                      fmt::format("Cannot get wheel from {}", m_elements.at(0)));
}

std::uint8_t nsw::SCAGeoIdentifier::getValue(const char prefix,
                                             const std::size_t position,
                                             const std::size_t maxValue,
                                             bool required) const
{
  if (m_noGeoId) {
    return nsw::geoid::DoesNotExist;
  }
  const auto throwOrReturnNotExist = [&required](const std::string& message) {
    if (required) {
      throw nsw::NSWSCAGeoIdentifierIssue(ERS_HERE, message);
    }
    return nsw::geoid::DoesNotExist;
  };
  if (std::size(m_elements) < position + 1) {
    return throwOrReturnNotExist(
      fmt::format("Did not find entry number {} in {}", position, m_scaAddress));
  }
  const auto& element = m_elements.at(position);
  if (not element.starts_with(prefix)) {
    return throwOrReturnNotExist(fmt::format(
      "Element number {} ({}) does not start with {} as requested", position, element, prefix));
  }
  const auto number = element.substr(1);
  const auto value = [&number, &throwOrReturnNotExist, this]() {
    try {
      return std::stoul(number);
    } catch (const std::invalid_argument& ex) {
      return static_cast<unsigned long>(throwOrReturnNotExist(
        fmt::format("{} is not a valid number (from {})", number, m_scaAddress)));
    }
  }();
  if (value > std::numeric_limits<std::uint8_t>::max()) {
    throwOrReturnNotExist(fmt::format("{} is too large for 8 bit int (from {})", number, element));
  }
  if (value >= maxValue) {
    throwOrReturnNotExist(fmt::format(
      "{} is too large given the maximum of {} (from {})", number, maxValue - 1, element));
  }
  return static_cast<std::uint8_t>(value);
}

bool nsw::SCAGeoIdentifier::layerRequired() const
{
  return isFeb() or m_resourceType == "L1DDC" or m_resourceType == "ADDC" or
         m_resourceType == "Router";
}

bool nsw::SCAGeoIdentifier::radiusRequired() const
{
  return isFeb();
}

void nsw::SCAGeoIdentifier::warnNoGeoId() const
{
  if (m_noGeoId) {
    ers::warning(NSWNoGeoId(ERS_HERE));
  }
}

bool nsw::SCAGeoIdentifier::isFeb() const
{
  return m_resourceType == "PFEB" or m_resourceType == "MMFE8" or m_resourceType == "SFEB6" or
         m_resourceType == "SFEB8" or m_resourceType == "SFEB";
}
