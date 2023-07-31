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