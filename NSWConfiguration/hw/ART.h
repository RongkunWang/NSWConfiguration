#ifndef NSWCONFIGURATION_HW_ART_H
#define NSWCONFIGURATION_HW_ART_H

#include <functional>

#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/OpcManager.h"

namespace nsw::hw {
  /**
   * \brief Class representing an ART
   *
   * Provides methods to read/write individual ART registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Register mapping: http://cern.ch/go/8jKB
   * Documentation: TODO
   */
  class ART
  {
  public:
    /**
     * \brief Constructor from an \ref ARTConfig object
     * 
     * @param manager Link to the OPC Manager
     * @param config Configuration of the ADDC
     * @param numArt Number of the ART on the ADDC
     */
    ART(OpcManager& manager, const nsw::ADDCConfig& config, std::size_t numArt);

    /**
     * \brief Read the full ART address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration() const;

    /**
     * \brief Write the full ART configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Write a value to a ART register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister([[maybe_unused]] std::uint8_t regAddress,
                       [[maybe_unused]] std::uint32_t value) const;

    /**
     * \brief Read an individual ART register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]] std::vector<std::uint8_t> readRegister(
      [[maybe_unused]] std::uint8_t regAddress) const;

    /**
     * \brief Get the Opc server IP
     *
     * \return std::string Opc server IP
     */
    [[nodiscard]] std::string getOpcServerIp() const { return m_opcserverIp; }

    /**
     * \brief Get the \ref ARTConfig object associated with this ART object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] ARTConfig& getConfig() { return m_config; }
    [[nodiscard]] const ARTConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    mutable std::reference_wrapper<OpcManager> m_opcManager;  //!< Pointer to OpcManager
    ARTConfig m_config;         //!< ARTConfig object associated with this ART
    std::string m_opcserverIp;  //!< address and port of Opc Server
    std::string m_scaAddress;   //!< SCA address of ART item in Opc address space
    std::size_t m_numArt;       //!< Number of the ART
  };
}  // namespace nsw::hw

#endif
