#ifndef NSWCONFIGURATION_HW_TDS_H
#define NSWCONFIGURATION_HW_TDS_H

#include "NSWConfiguration/ConfigTranslationMap.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/hw/OpcManager.h"

namespace nsw::hw {
  /**
   * \brief Class representing a TDS
   *
   * Provides methods to read/write individual TDS registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Register mapping: TODO
   * Documentation: http://cern.ch/go/9kgk
   */
  class TDS
  {
  public:
    /**
     * \brief Constructor from a \ref FEBConfig object and a TDS ID
     */
    TDS(nsw::OpcManager& manager, const nsw::FEBConfig& config, std::size_t numTds);

    /**
     * \brief Read the full TDS address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint8_t>> readConfiguration() const;

    /**
     * \brief Write the full TDS configuration
     *
     * \param resetTds Reset the TDS after configuring
     * \throws std::logic_error Unknown TDS name
     */
    void writeConfiguration(bool resetTds = false) const;

    /**
     * \brief Read a TDS register
     *
     * \param regAddress is the address of the register
     * \return std::uint8_t is the value of the register
     */
    [[nodiscard]] std::vector<std::uint8_t> readRegister(std::uint8_t regAddress) const;

    /**
     * \brief Write a value to a TDS register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister(std::uint8_t regAddress, __uint128_t value) const;

    /**
     * \brief Write a value to a TDS register address
     *
     * \param regName is the name of the register
     * \param value is the value to be written
     */
    void writeRegister(const std::string& regName, __uint128_t value) const;

    /**
     * \brief Write values to the TDS
     *
     * \param values map of name to value
     */
    void writeValues(const std::map<std::string, unsigned int>& values) const;

    /**
     * \brief Write a value to the TDS
     *
     * \param name Name of the value (value-based representation)
     * \param value The value
     */
    void writeValue(const std::string& name, unsigned int value) const;

    /**
     * \brief Read a value from the TDS
     *
     * \param name Name of the value (value-based representation)
     */
    [[nodiscard]] unsigned int readValue(const std::string& name) const;

    /**
     * \brief Read values from the TDS
     *
     * \param values span of names of values
     */
    [[nodiscard]] std::map<std::string, unsigned int> readValues(
      std::span<const std::string> names) const;

    /**
     * \brief Get the \ref I2cMasterConfig object associated with this TDS object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] I2cMasterConfig& getConfig() { return m_config; }
    [[nodiscard]] const I2cMasterConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
    I2cMasterConfig m_config;   //!< I2cMasterConfig object associated with this TDS
    std::string m_opcserverIp;  //!< address and port of Opc Server
    std::string m_scaAddress;   //!< SCA address of FE item in Opc address space
    bool m_isPfeb;              //!< is this TDS on a PFEB or SFEB
  };
}  // namespace nsw::hw

#endif
