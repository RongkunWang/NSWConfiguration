#ifndef NSWCONFIGURATION_SCAGEOIDENTIFIER_H
#define NSWCONFIGURATION_SCAGEOIDENTIFIER_H

#include "NSWConfiguration/Constants.h"
#include <cstddef>
#include <string>
#include <fmt/core.h>

#include <ers/ers.h>

ERS_DECLARE_ISSUE(nsw, NSWSCAGeoIdentifierIssue, message, ((std::string)message))
ERS_DECLARE_ISSUE(nsw,
                  NSWNoGeoId,
                  fmt::format("Returning value 'unknown/doesnotexist' since node ID is not a geo ID: {}", scaAddress),
                  ((std::string)scaAddress)
                  )

namespace nsw {
  /**
   * \brief Information about the location of a device
   */
  class SCAGeoIdentifier
  {
  public:
    /**
     * \brief Construct a new SCAGeoIdentifier object
     *
     * \param scaAddress OPC node ID of the device (e.g. sTGC-A/V0/SCA/Strip/S9/L7/R0)
     */
    explicit SCAGeoIdentifier(std::string scaAddress);

    /**
     * \brief Is an MMG device
     *
     * \return true is MMG
     * \return false is STG
     */
    [[nodiscard]] bool isMM() const;

    /**
     * \brief Is an MMG device
     *
     * \return true is STG
     * \return false is MMG
     */
    [[nodiscard]] bool isSTGC() const;

    /**
     * \brief Get the detector technology
     *
     * \return nsw::geoid::Detector MMG or STG
     */
    [[nodiscard]] nsw::geoid::Detector detector() const
    {
      warnNoGeoId();
      return m_detector;
    };

    /**
     * \brief Get the wheel
     *
     * \return nsw::geoid::Wheel A or C
     */
    [[nodiscard]] nsw::geoid::Wheel wheel() const
    {
      warnNoGeoId();
      return m_wheel;
    };

    /**
     * \brief Get the device type
     *
     * \return std::string device type
     */
    [[nodiscard]] std::string resourceType() const
    {
      warnNoGeoId();
      return m_resourceType;
    };

    /**
     * \brief Get the sector
     *
     * \return std::uint8_t Sector
     */
    [[nodiscard]] std::uint8_t sector() const
    {
      warnNoGeoId();
      return m_sector;
    };

    /**
     * \brief Get the layer (nsw::geoid::DoesNotExist if no layer)
     *
     * \return std::uint8_t Layer
     */
    [[nodiscard]] std::uint8_t layer() const
    {
      warnNoGeoId();
      return m_layer;
    };

    /**
     * \brief Get the radius (nsw::geoid::DoesNotExist if no radius)
     *
     * \return std::uint8_t Radius
     */
    [[nodiscard]] std::uint8_t radius() const
    {
      warnNoGeoId();
      return m_radius;
    };

  private:
    std::string m_scaAddress{};
    std::vector<std::string> m_elements{};
    bool m_noGeoId{false};
    nsw::geoid::Detector m_detector{};
    nsw::geoid::Wheel m_wheel{};
    std::string m_resourceType;
    std::uint8_t m_sector{};
    std::uint8_t m_layer{};
    std::uint8_t m_radius{};

    /**
     * \brief Get a value from the node ID
     *
     * \param prefix Prefix of the value
     * \param position Position in the geo ID
     * \param maxValue Maximum allowed value
     * \param required Throw if no success
     * \return std::uint8_t value
     */
    [[nodiscard]] std::uint8_t getValue(char prefix,
                                        std::size_t position,
                                        std::size_t maxValue,
                                        bool required) const;

    /**
     * \brief Get the detector technology from geo ID
     *
     * \return nsw::geoid::Detector Detector technology
     */
    [[nodiscard]] nsw::geoid::Detector extractDetector() const;

    /**
     * \brief Get the wheel from geo ID
     *
     * \return nsw::geoid::Detector Wheel
     */
    [[nodiscard]] nsw::geoid::Wheel extractWheel() const;

    /**
     * \brief Is the layer required for this device
     *
     * \return true is required to be found
     * \return false is not expected to be found
     */
    [[nodiscard]] bool layerRequired() const;

    /**
     * \brief Is the radius required for this device
     *
     * \return true is required to be found
     * \return false is not expected to be found
     */
    [[nodiscard]] bool radiusRequired() const;

    /**
     * \brief Warn in case user wants to retrieve information if no geo ID was provided
     */
    void warnNoGeoId() const;

    /**
     * \brief Is Device a FEB
     *
     * \return true is FEB
     * \return false is no FEB
     */
    [[nodiscard]] bool isFeb() const;

    static constexpr std::string_view DELIMITER{"/"};
    static constexpr std::string_view DETECTOR_NAME_MM{"MM-"};
    static constexpr std::string_view DETECTOR_NAME_STGC{"sTGC-"};
    static constexpr std::string_view WHEEL_NAME_A{"-A"};
    static constexpr std::string_view WHEEL_NAME_C{"-C"};
    static constexpr std::size_t POSITION_SECTOR{4};
    static constexpr std::size_t POSITION_LAYER{5};
    static constexpr std::size_t POSITION_RADIUS{6};
    static constexpr char PREFIX_SECTOR{'S'};
    static constexpr char PREFIX_LAYER{'L'};
    static constexpr char PREFIX_RADIUS{'R'};
  };
}  // namespace nsw

#endif
