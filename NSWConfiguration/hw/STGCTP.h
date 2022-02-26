#ifndef NSWCONFIGURATION_HW_STGCTP_H
#define NSWCONFIGURATION_HW_STGCTP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <ers/Issue.h>

#include "NSWConfiguration/hw/OpcManager.h"

ERS_DECLARE_ISSUE(nsw,
                  STGCTPReadbackMismatch,
                  message,
                  ((const char *)message)
                  )

namespace nsw::hw {
  /**
   * \brief Class representing a STGC Trigger Processor
   *
   * Provides methods to read/write individual STGC TP registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   */
  class STGCTP
  {
  public:
    /**
     * \brief Constrctor from a \ref STGCTPConfig object
     */
    STGCTP(OpcManager& manager, const boost::property_tree::ptree& config);

    /**
     * \brief Name of STGC TP object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Read the full STGCTP address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::uint32_t, std::uint32_t> readConfiguration() const;

    /**
     * \brief Write the full STGCTP configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Write a value to a STGCTP register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister(const std::uint32_t regAddress,
                       const std::uint32_t value) const;

    /**
     * \brief Read an individual STGCTP register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readRegister(const std::uint32_t regAddress) const;

    /**
     * \brief Write a value to a STGCTP register address, and read it back
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(const std::uint32_t regAddress,
                                  const std::uint32_t value) const;

    /**
     * \brief Get the Opc server IP
     *
     * \return std::string Opc server IP
     */
    [[nodiscard]] std::string getOpcServerIp() const { return m_opcserverIp; }

    /**
     * \brief Get the ptree object associated with this STGCTP object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]]
    boost::property_tree::ptree& getConfig() { return m_config; }
    [[nodiscard]]
    const boost::property_tree::ptree& getConfig() const { return m_config; }  //!< \overload

    /**
     * \brief Read the sector from the ptree config
     */
    [[nodiscard]]
    std::uint32_t getSector() const;

  private:
    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
    boost::property_tree::ptree m_config; //!< ptree object associated with this STGCTP
    std::string m_opcserverIp;            //!< address and port of Opc Server
    std::string m_scaAddress;             //!< SCA address of STGCTP item in Opc address space
    std::string m_name;                   //!< Name composed of OPC and SCA addresses
  };
}  // namespace nsw::hw

#endif
