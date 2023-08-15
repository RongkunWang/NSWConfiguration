#ifndef NSWCONFIGURAITON_HW_GBTXIC_H
#define NSWCONFIGURAITON_HW_GBTXIC_H

#include <ic-handler/IChandler.h>

#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/hw/ConfigurationTracker.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

namespace nsw::hw {
  class GBTxIC : public ScaAddressBase
  {
  public:
    /**
     * \brief Constructor from a \ref L1DDCConfig object
     *
     * \param config L1DDC config object
     * \param gbtxId ID of GBTx
     */
    explicit GBTxIC(const L1DDCConfig& config);

    /**
     * \brief Read the full GBTx address space
     *
     * \return std::vector<std::uint8_t> Vector of registers
     */
    [[nodiscard]] std::vector<std::uint8_t> readConfiguration() const;

    /**
     * \brief Write the full GBTx configuration
     */
    void writeConfiguration() const;

    /**
     * @brief Write the full GBTx configuration
     *
     * @param config Configuration to be written
     */
    void writeConfiguration(const GBTxConfig& config) const;

    /**
     * @brief Train the GBTx
     *
     * Set the train registers on and off after a delay
     *
     * @param trainEc Train EC link
     * @param sleepTime Delay until nominal configuration is written again
     */
    void train(bool trainEc, const std::chrono::microseconds& sleepTime) const;

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
    ic::fct::IChandler m_icConnection;
    GBTxConfig m_config;
    std::string m_name;
    mutable internal::ConfigurationTrackerMap<internal::DeviceType::GBTx> m_tracker;
    constexpr static std::size_t GBTX_IC_ID{0};
  };
}  // namespace nsw::hw

#endif