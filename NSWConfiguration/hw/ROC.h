#ifndef NSWCONFIGURATION_HW_ROC_H
#define NSWCONFIGURATION_HW_ROC_H

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_set>

#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/OpcClientPtr.h"
#include "NSWConfiguration/ConfigConverter.h"
#include "NSWConfiguration/hw/Helper.h"

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
    ROC(nsw::OpcManager& manager, const nsw::FEBConfig& config);

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
     * \brief Write values to the ROC (either analog or digital)
     *
     * \param values map of name to value
     */
    void writeValues(const std::map<std::string, unsigned int>& values) const;

    /**
     * \brief Write a value to the ROC
     *
     * \param name Name of the value (value-based representation)
     * \param value The value
     */
    void writeValue(const std::string& name, unsigned int value) const;

    /**
     * \brief Read a value from the ROC
     *
     * \param name Name of the value (value-based representation)
     * \return Read value
     */
    [[nodiscard]] unsigned int readValue(const std::string& name) const;

    /**
     * \brief Read values from the ROC (either analog or digital)
     *
     * \param names range of names of values
     * \tparam Range Iterable list of strings 
     */
    template<typename Range> // add requires with c++20
    [[nodiscard]] std::map<std::string, unsigned int> readValues(const Range& names) const
    {
      const auto isAnalog = internal::ROC::namesAnalog(names);
      // Lambda to const init the register names. Pulls in isAnalog and the names and asks the config
      // converter for the registers for the given names
      const auto regNames = [isAnalog, &names] {
        std::unordered_set<std::string> result{};
        for (const auto& name : names) {
          if (isAnalog) {
            result.merge(ConfigConverter<ConfigConversionType::ROC_ANALOG>::getRegsForValue(name));
          }
          else {
            result.merge(ConfigConverter<ConfigConversionType::ROC_DIGITAL>::getRegsForValue(name));
          }
        }
        return result;
      }();

      std::map<std::string, unsigned int> registerValues{};
      std::transform(std::cbegin(regNames),
                     std::cend(regNames),
                     std::inserter(registerValues, std::begin(registerValues)),
                     [&](const auto& regName) -> std::pair<std::string, std::uint8_t> {
                       return {regName, readRegister(getRegAddress(regName, isAnalog))};
                     });

      // Lambda to const init the read-back values. Pulls in isAnalog, the read-back register values and the names.
      // Asks the config converter to do the translation. TODO: Clean up config converter call (future MR)
      const auto values = [isAnalog, &registerValues, &names]() {
        if (isAnalog) {
          const auto configConverter = ConfigConverter<ConfigConversionType::ROC_ANALOG>(
            transformMapToPtree(ConfigConverter<ConfigConversionType::ROC_ANALOG>::convertRegisterToSubRegister(
              transformMapToPtree(registerValues), names)),
            ConfigType::REGISTER_BASED);
          return configConverter.getValueBasedConfig();
        }
        const auto configConverter = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(
          transformMapToPtree(ConfigConverter<ConfigConversionType::ROC_DIGITAL>::convertRegisterToSubRegister(
            transformMapToPtree(registerValues), names)),
          ConfigType::REGISTER_BASED);
        return configConverter.getValueBasedConfig();
      }();

      return transformPtreetoMap<unsigned int>(values);
    }

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

    [[nodiscard]] std::string getName() const { return m_scaAddress; }

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
    void setSResetN(const nsw::internal::OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Reset for all PLLs
     *
     * \param opcConnection OPC client
     * \param state true = set reset, false = release reset
     */
    void setPllResetN(const nsw::internal::OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Asynchronous reset for the ROC core
     *
     * \param opcConnection OPC client
     * \param state true = set reset, false = release reset
     */
    void setCoreResetN(const nsw::internal::OpcClientPtr& opcConnection, bool state) const;

    /**
     * \brief Set a given reset to a given state
     *
     * \param opcConnection OPC client
     * \param resetName GPIO name of the reset
     * \param state true = set reset, false = release reset
     */
    void setReset(const nsw::internal::OpcClientPtr& opcConnection,
                  const std::string& resetName,
                  bool state) const;

    /**
     * \brief Get the address of a named register
     *
     * \param regName is the name of the register
     * \param isAnalog is it an analog register
     * \return std::uint8_t is the address of the register
     */
    [[nodiscard]] static std::uint8_t getRegAddress(const std::string& regName, bool isAnalog);

    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
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
