#ifndef NSWCONFIGURAITON_HW_L1DDC_H
#define NSWCONFIGURAITON_HW_L1DDC_H

#include <chrono>
#include <type_traits>

#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/hw/GBTxI2c.h"
#include "NSWConfiguration/hw/GBTxIC.h"
#include "NSWConfiguration/hw/OpcManager.h"

namespace nsw::hw {
  /**
   * @brief Class representing an L1DDC
   *
   * Provides methods to read and write the L1DDC configuration.
   *
   * Documentation: TODO
   */
  class L1DDC
  {
  public:
    /**
     * @brief Constructor
     *
     * @param manager Reference to OPC manager
     * @param config L1DDC config object
     */
    L1DDC(OpcManager& manager, const L1DDCConfig& config);

    /**
     * @brief Configure all GBTx's which are supposed to be configured
     */
    void writeConfiguration();

    /**
     * \brief Get the \ref GBTxConfig object associated with this L1DDC object
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] L1DDCConfig& getConfig() { return m_config; }
    [[nodiscard]] const L1DDCConfig& getConfig() const { return m_config; } //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref GBTx0
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] GBTxIC& getGbtx0() { return m_gbtxIc; }
    [[nodiscard]] const GBTxIC& getGbtx0() const { return m_gbtxIc; }  //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref GBTx1
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] GBTxI2c& getGbtx1() { return m_gbtxI2c.at(0); }
    [[nodiscard]] const GBTxI2c& getGbtx1() const { return m_gbtxI2c.at(0); }  //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref GBTx2
     *
     * Both const and non-const overloads are provided
     */
    // clang-format off
    [[nodiscard]] GBTxI2c& getGbtx2() { return m_gbtxI2c.at(1); }
    [[nodiscard]] const GBTxI2c& getGbtx2() const { return m_gbtxI2c.at(1); }  //!< \overload
    // clang-format on

  private:
    /**
     * @brief Configure all GBTx's which are supposed to be configured
     */
    template<typename GBTx>
    void writeConfiguration(GBTx& gbtx) requires(std::is_same_v<GBTx, GBTxIC> or
                                                 std::is_same_v<GBTx, GBTxI2c>)
    {
      for (std::size_t nTries = 0; nTries < MAX_ATTEMPTS; ++nTries) {
        gbtx.writeConfiguration();
        static_cast<void>(gbtx.readConfiguration());
        if (not gbtx.hasConfigurationErrors()) {
          return;
        }
        // nTries--; // FIXME infinite loop time!
        ERS_LOG(fmt::format("Retrying configuration of GBTx {}. Attempt: {}/{}", gbtx.getScaAddress(), nTries, MAX_ATTEMPTS));
      }

      nsw::NSWGBTxIssue issue(
        ERS_HERE, fmt::format("Unable configure GBTx {} within {} attempts", gbtx.getScaAddress(), MAX_ATTEMPTS));
      ers::error(issue);
    }

    GBTxIC m_gbtxIc;
    std::array<GBTxI2c, 2> m_gbtxI2c;
    L1DDCConfig m_config;
  };
}  // namespace nsw::hw

#endif