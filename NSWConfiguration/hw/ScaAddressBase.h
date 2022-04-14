#ifndef NSWCONFIGURATION_HW_SCAADDRESSBASE_H
#define NSWCONFIGURATION_HW_SCAADDRESSBASE_H

#include "NSWConfiguration/SCAGeoIdentifier.h"

namespace nsw::hw {
  /**
   * \brief Base class for HW interfaces which have an SCA address
   */
  class ScaAddressBase
  {
  public:
    /**
     * \brief Constructor
     *
     * \param scaAddress SCA address (OPC node ID)
     */
    explicit ScaAddressBase(std::string scaAddress);

    /**
     * \brief Get the SCA address/OPC node ID/geo ID
     *
     * \return std::string SCA address
     */
    [[nodiscard]] std::string getScaAddress() const { return m_scaAddress; }

    /**
     * \brief Get the geo ID with slashes replaced by underscores for filenames
     *
     * \return std::string \copydoc m_filenameCompatibleGeoId
     */
    [[nodiscard]] std::string getFilenameCompatibleGeoId() const
    {
      return m_filenameCompatibleGeoId;
    }

    /**
     * \brief Get the geo information object
     *
     * \return const SCAGeoIdentifier& Object holding information about the SCA address
     */
    [[nodiscard]] const SCAGeoIdentifier& getGeoInfo() const { return m_geoIdInfo; }

    /**
     * \brief Replace / with _
     *
     * \param str input string with /
     * \return std::string output string with _
     */
    static std::string replaceSlashes(const std::string& str);

  private:
    std::string m_scaAddress;                 //!< SCA address
    std::string m_filenameCompatibleGeoId{};  //!< Geo ID with underscores instead of slashes
    SCAGeoIdentifier m_geoIdInfo;             //!< Information about location etc.
  };
}  // namespace nsw::hw

#endif
