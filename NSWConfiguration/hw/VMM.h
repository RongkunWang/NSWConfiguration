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
     * \brief Get the \ref VMMConfig object associated with this VMM object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] VMMConfig& getConfig() { return m_config; }
    [[nodiscard]] const VMMConfig& getConfig() const { return m_config; }  //!< overload

  private:
    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
    VMMConfig m_config;           //!< VMMConfig object associated with this VMM
    std::string m_opcserverIp;    //!< address and port of Opc Server
    std::string m_scaAddress;     //!< SCA address of FE item in Opc address space
    std::string m_rocAnalogName;  //!< Disable data acquisition
  };
}  // namespace nsw::hw

#endif
