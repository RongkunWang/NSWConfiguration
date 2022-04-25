#ifndef NSWCONFIGURATION_HW_TPCARRIER_H
#define NSWCONFIGURATION_HW_TPCARRIER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/TPCarrierConfig.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

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
  class TPCarrier : public ScaAddressBase, OpcConnectionBase
  {
  public:
    /**
     * \brief Constrctor from a \ref TPCarrierConfig object
     */
    TPCarrier(nsw::OpcManager& manager, const TPCarrierConfig& config);

    /**
     * \brief Name of TP Carrier object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Read the full TPCarrier address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::uint32_t, std::uint32_t> readConfiguration() const;

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
    void writeRegister(std::uint32_t regAddress,
                       std::uint32_t value) const;

    /**
     * \brief Read an individual TPCarrier register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::uint32_t regAddress) const;

    /**
     * \brief Write a value to a TPCarrier register address, and read it back
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(const std::uint32_t regAddress,
                                  const std::uint32_t value) const;

    /**
     * \brief Get the \ref TPCarrierConfig object associated with this TPCarrier object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]]
    TPCarrierConfig& getConfig() { return m_config; }
    [[nodiscard]]
    const TPCarrierConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    TPCarrierConfig m_config;   //!< TPCarrierConfig object associated with this TPCarrier
    std::string m_busAddress;   //!< Address of the I2C bus of this SCAX
    std::string m_name;         //!< Name of this hardware object
  };
}  // namespace nsw::hw

#endif
