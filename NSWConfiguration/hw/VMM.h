#ifndef NSWCONFIGURATION_HW_VMM_H
#define NSWCONFIGURATION_HW_VMM_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/OpcManager.h"

namespace nsw::hw {
  /**
   * \brief Class representing a VMM
   *
   * Provides methods to read/write individual VMM registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Documentation: http://cern.ch/go/9FCJ
   */
  class VMM
  {
  public:
    /**
     * \brief Constructor from a \ref FEBConfig object and a VMM ID
     */
    VMM(nsw::OpcManager& manager, const FEBConfig& config, std::size_t numVmm);

    /**
     * \brief Read the full VMM address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint8_t>> readConfiguration() const;

    /**
     * \brief Write the full VMM configuration
     *
     * \param resetVmm Reset the VMM
     */
    void writeConfiguration(bool resetVmm = false) const;

    /**
     * \brief Write a provided full VMM configuration
     *
     * \param config Config to be sent
     * \param resetVmm Reset the VMM
     */
    void writeConfiguration(const VMMConfig& config, bool resetVmm = false) const;

    /**
     * \brief Sampling the selected monitoring output of the VMM by the PDO channel
     *
     * \param nSamples Number of samples
     * \return std::vector<std::uint16_t> Samples
     */
    std::vector<std::uint16_t> samplePdoMonitoringOutput(std::size_t nSamples) const;

    /**
     * \brief Sampling the selected monitoring output of the VMM by the PDO channel
     *
     * \param config modified configuration to be send
     * \param nSamples Number of samples
     * \return std::vector<std::uint16_t> Samples
     */
    std::vector<std::uint16_t> samplePdoMonitoringOutput(VMMConfig config,
                                                         std::size_t nSamples) const;

    /**
     * \brief Get the \ref VMMConfig object associated with this VMM object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] VMMConfig& getConfig() { return m_config; }
    [[nodiscard]] const VMMConfig& getConfig() const { return m_config; }  //!< overload

  private:

    /**
     * \brief Set VMMConfigurationStatusInfo FreeVariable parameter
     *        used by SCA DCS for VMM boards (polyneikis).
     *
     * \param opcConnection OPC client to perform the transaction
     * \param config VMM Configuration to be set on the chip
     */
    void setVmmConfigurationStatusInfoDcs(const OpcClientPtr& opcConnection,
                                          const nsw::VMMConfig& config) const;

    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
    VMMConfig m_config;           //!< VMMConfig object associated with this VMM
    std::string m_opcserverIp;    //!< address and port of Opc Server
    std::string m_scaAddress;     //!< SCA address of FE item in Opc address space
    std::string m_rocAnalogName;  //!< Disable data acquisition
    std::size_t m_vmmId{};        //!< Board position of VMM
  };
}  // namespace nsw::hw

#endif
