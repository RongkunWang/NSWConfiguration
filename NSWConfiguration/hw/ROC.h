#ifndef NSWCONFIGURATION_HW_ROC_H
#define NSWCONFIGURATION_HW_ROC_H

#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include <stdexcept>

namespace nsw::hw {
  /**
   * \brief Class representing a ROC
   *
   * Provides methods to read/write individual ROC registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Register mapping: http://cern.ch/go/9Q9t
   * ROC digital documentation: http://cern.ch/go/HX7Z
   */
  class ROC
  {
  public:
    /**
     * \brief Constructor from a \ref FEBConfig object
     */
    explicit ROC(const nsw::FEBConfig& config);

    /**
     * \brief Read the full ROC address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::uint8_t> readConfiguration() const;

    /**
     * \brief Write the full ROC configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Read a ROC register
     *
     * \param regAddress is the address of the register
     * \return std::uint8_t is the value of the register
     */
    [[nodiscard]] std::uint8_t readRegister(std::uint8_t regAddress) const;

    /**
     * \brief Write a value to a ROC register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     * \throws std::out_of_range \ref regAddress out of range
     */
    void writeRegister(std::uint8_t regAddress, std::uint8_t value) const;

    /**
     * \brief Write a value to a ROC register address
     *
     * \param regName is the name of the register
     * \param value is the value to be written
     * \throws std::logic_error \ref regName is invalid
     */
    void writeRegister(const std::string& regName, std::uint8_t value) const;

    /**
     * \brief Disable all VMMs in the VMM enable register
     */
    void disableVmmCaptureInputs() const;

    /**
     * \brief Set the VMM enable register to the value in the passed config
     */
    void enableVmmCaptureInputs() const;

    /**
     * \brief Read the VMM capture status registers
     *
     * \param vmmIndex Index of the VMM (0-7)
     * \return std::uint8_t value of the status register
     * \throws std::out_of_range \ref vmmIndex out of range
     */
    [[nodiscard]] std::uint8_t readVmmCaptureStatus(std::uint8_t vmmIndex) const;

    /**
     * \brief Read the VMM parity counter
     *
     * \param vmmIndex Index of the VMM (0-7)
     * \return std::uint8_t value of the status register
     * \throws std::out_of_range \ref vmmIndex out of range
     */
    [[nodiscard]] std::uint8_t readVmmParityCounter(std::uint8_t vmmIndex) const;

    /**
     * \brief Read the sROC status registers
     *
     * \param srocIndex Index of the sROC (0-3)
     * \return std::uint8_t value of the status register
     * \throws std::out_of_range \ref srocIndex out of range
     */
    [[nodiscard]] std::uint8_t readSrocStatus(std::uint8_t srocIndex) const;

    /**
     * \brief Get the \ref I2cMasterConfig object associated with this ROC object
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] I2cMasterConfig& getConfigAnalog() { return m_rocAnalog; }
    [[nodiscard]] const I2cMasterConfig& getConfigAnalog() const { return m_rocAnalog; } //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref I2cMasterConfig object associated with this ROC object
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] I2cMasterConfig& getConfigDigital() { return m_rocDigital; }
    [[nodiscard]] const I2cMasterConfig& getConfigDigital() const { return m_rocDigital; } //!< \overload
    // clang-format on

  private:
    /**
     * \brief Read a status register
     *
     * Performs to reads to read an up-to-date value (read resets register value).
     *
     * \param regAddress Address of the register
     * \return std::uint8_t Value of the status register
     */
    [[nodiscard]] std::uint8_t readStatusRegister(std::uint8_t regAddress) const;

    /**
     * \brief Reset for ePLL core. Only configuration registers are reset
     *
     * \param opcConnection OPC client
     * \param state true = set reset, false = release reset
     */
    void setSResetN(const OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Reset for all PLLs
     *
     * \param opcConnection OPC client
     * \param state true = set reset, false = release reset
     */
    void setPllResetN(const OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Asynchronous reset for the ROC core
     *
     * \param opcConnection OPC client
     * \param state true = set reset, false = release reset
     */
    void setCoreResetN(const OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Set a given reset to a given state
     *
     * \param opcConnection OPC client
     * \param resetName GPIO name of the reset
     * \param state true = set reset, false = release reset
     */
    void setReset(const OpcClientPtr& opcConnection,
                  const std::string& resetName,
                  bool state) const;

    I2cMasterConfig m_rocAnalog;   //!< associated I2cMasterConfig for the analog part of this ROC
    I2cMasterConfig m_rocDigital;  //!< associated I2cMasterConfig for the digital part of this ROC
    std::string m_opcserverIp;     //!< address and port of Opc Server
    std::string m_scaAddress;      //!< SCA address of FE item in Opc address space
    constexpr static std::array<std::uint8_t, 22>
      UNUSED_REGISTERS{15, 16, 17, 18, 25, 26, 27, 28, 29, 30, 54, 55, 56, 57, 58, 59, 60, 61, 62, 125, 126, 127};  //!< Unused ROC registers

    struct UnusedRegisterException : public std::logic_error {
      explicit UnusedRegisterException(const std::string& message) : logic_error(message) {}
    };
  };
}  // namespace nsw::hw

#endif
