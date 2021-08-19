#ifndef NSWCONFIGURATION_HW_TPCARRIER_H
#define NSWCONFIGURATION_HW_TPCARRIER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/TPCarrierConfig.h"

namespace nsw::hw {
  /**
   * \brief Class representing a Trigger Processor Carrier board
   *
   * Provides methods to read/write individual TP Carrier registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Register mapping: TODO
   * Documentation: TODO
   */
  class TPCarrier
  {
  public:
    /**
     * \brief Constrctor from a \ref TPCarrierConfig object
     */
    explicit TPCarrier(const TPCarrierConfig& config);

    /**
     * \brief Read the full TPCarrier address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration() const;

    /**
     * \brief Write the full TPCarrier configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Write a value to a TPCarrier register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister([[maybe_unused]] std::uint8_t regAddress,
                       [[maybe_unused]] std::uint32_t value) const;

    /**
     * \brief Read an individual TPCarrier register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]] std::vector<std::uint8_t> readRegister(
      [[maybe_unused]] std::uint8_t regAddress) const;

    /**
     * \brief Get the \ref TPCarrierConfig object associated with this TPCarrier object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] TPCarrierConfig& getConfig() { return m_config; }
    [[nodiscard]] const TPCarrierConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    TPCarrierConfig m_config;   //!< TPCarrierConfig object associated with this TPCarrier
    std::string m_opcserverIp;  //!< address and port of Opc Server
    std::string m_scaAddress;   //!< SCA address of TPCarrier item in Opc address space
  };
}  // namespace nsw::hw

#endif
