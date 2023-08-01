#ifndef NSWCONFIGURAITON_HW_GBTXI2C_H
#define NSWCONFIGURAITON_HW_GBTXI2C_H

#include <chrono>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/hw/ConfigurationTracker.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

namespace nsw::hw {
  /**
   * @brief Class representing a GBTx
   *
   * Provides methods to read and write the GBTx configuration.
   *
   * Documentation: TODO
   */
  class GBTxI2c : public ScaAddressBase, public OpcConnectionBase
  {
  public:
    /**
     * @brief Constructor from a @ref L1DDCConfig object
     *
     * @param manager Reference to OPC manager
     * @param config L1DDC config object
     * @param gbtxId ID of GBTx
     */
    GBTxI2c(OpcManager& manager, const L1DDCConfig& config, std::size_t gbtxId);

    /**
     * @brief Read the full GBTx address space
     *
     * @return std::vector<std::uint8_t> Vector of registers
     */
    [[nodiscard]] std::vector<std::uint8_t> readConfiguration() const;

    /**
     * @brief Write the full GBTx configuration
     */
    void writeConfiguration() const;

    /**
     * @brief Write the full GBTx configuration
     *
     * @param config Configuration to be written
     */
    void writeConfiguration(const GBTxConfig& config) const;

    /**
     * @brief Check if the configuration had errors
     *
     * @return true Errors
     * @return false No errors
     */
    [[nodiscard]] bool hasConfigurationErrors() const;

    /**
     * \brief Get the \ref GBTxConfig object associated with this GBTx object
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] GBTxConfig& getConfig() { return m_config; }
    [[nodiscard]] const GBTxConfig& getConfig() const { return m_config; } //!< \overload
    // clang-format on

  private:
    /**
     * @brief Ensure that the HW interface is valid
     *
     * @param config L1DDC config object
     * @param gbtxId ID of GBTx
     * @return true Can be used to talk to HW
     * @return false Cannot be used
     */
    [[nodiscard]] static bool validateConfig(const L1DDCConfig& config, std::size_t gbtxId);

    GBTxConfig m_config;
    std::string m_name{};
    std::size_t m_gbtxId{};
    std::size_t m_chunkSize{};
    std::chrono::microseconds m_delay{};
    bool m_invalid{false};
    mutable internal::ConfigurationTrackerMap<internal::DeviceType::GBTx> m_tracker;
  };
}  // namespace nsw::hw

#endif