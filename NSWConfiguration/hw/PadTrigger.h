#ifndef NSWCONFIGURATION_HW_PADTRIGGER_H
#define NSWCONFIGURATION_HW_PADTRIGGER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/PadTriggerSCAConfig.h"

namespace nsw::hw {
  /**
   * \brief Class representing a Pad Trigger
   *
   * Provides methods to read/write individual PadTrigger registers,
   * as well as to write a complete configuration and read back all
   * the registers contained in the configuration.
   *
   * Register mapping: TODO
   * Documentation: TODO
   */
  class PadTrigger
  {
  public:
    /**
     * \brief Constrctor from a \ref PadTriggerConfig object
     */
    explicit PadTrigger(const PadTriggerSCAConfig& config);

    /**
     * \brief Read the full PadTrigger address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration() const;

    /**
     * \brief Write the full PadTrigger configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Write a value to a TP register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister([[maybe_unused]] std::uint8_t regAddress,
                       [[maybe_unused]] std::uint32_t value) const;

    /**
     * \brief Read an individual TP register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]] std::vector<std::uint8_t> readRegister(
      [[maybe_unused]] std::uint8_t regAddress) const;

    /**
     * \brief Get the \ref PadTriggerConfig object associated with this PadTrigger object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] PadTriggerSCAConfig& getConfig() { return m_config; }
    [[nodiscard]] const PadTriggerSCAConfig& getConfig() const { return m_config; }  //!< overload

  private:
    PadTriggerSCAConfig m_config;  //!< PadTriggerConfig object associated with this PadTrigger
    std::string m_opcserverIp;     //!< Address and port of OPC Server
    std::string m_scaAddress;      //!< SCA address of PadTrigger item in the OPC address space
  };
}  // namespace nsw::hw

#endif
