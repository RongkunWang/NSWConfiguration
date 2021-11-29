#ifndef NSWCONFIGURAITON_HW_GBTX_H
#define NSWCONFIGURAITON_HW_GBTX_H

#include <chrono>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"

#include "ic-over-netio/IChandler.h"

ERS_DECLARE_ISSUE(nsw, NSWGBTxIssue, message, ((std::string)message))

namespace nsw::hw {
  /**
   * \brief Class representing a GBTx
   *
   * Provides methods to read and write the GBTx configuration and train them.
   *
   * Documentation:
   */
  class GBTx
  {
  public:
    /**
     * \brief Constructor from a \ref L1DDCConfig object
     *
     * \param config L1DDC config object
     * \param gbtxId ID of GBTx
     */
    GBTx(const L1DDCConfig& config, std::size_t gbtxId);

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
     * \brief Train the GBTx
     *
     * Set the train and reset registers and then write the normal configuration again
     */
    void train() const;

  private:
    /**
     * \brief Confirm that the written configuration matches the expectation
     *
     * \param config Config object to be confirmed
     * \return true Matches reference
     * \return false Does not match reference
     */
    bool confirmConfiguration(const GBTxConfig& config) const;

    /**
     * \brief Write the passed configuration
     *
     * \param config Config object to be written
     */
    void writeConfiguration(const GBTxConfig& config) const;

    /**
     * \brief Write configuration through IC
     *
     * \param data Data to be written
     */
    void writeIcConfiguration(const std::vector<std::uint8_t>& data) const;

    /**
     * \brief Read configuration through IC
     *
     * \return std::vector<std::uint8_t> Read data
     */
    [[nodiscard]] std::vector<std::uint8_t> readIcConfiguration() const;

    /**
     * \brief Write configuration through I2c
     *
     * \param data Data to be written
     */
    void writeI2cConfiguration(const std::vector<std::uint8_t>& data) const;

    /**
     * \brief Read configuration through I2c
     *
     * \return std::vector<std::uint8_t> Read data
     */
    [[nodiscard]] std::vector<std::uint8_t> readI2cConfiguration() const;

    GBTxConfig m_config;
    std::size_t m_gbtxId;
    std::size_t m_portIn;
    std::size_t m_portOut;
    std::size_t m_elinkId;
    std::string m_felixIp;
    std::string m_opcserverIp;
    std::string m_opcNodeId;
    std::size_t m_chunkSize;
    std::chrono::microseconds m_delay;
    std::string m_name;
  };
}  // namespace nsw::hw

#endif