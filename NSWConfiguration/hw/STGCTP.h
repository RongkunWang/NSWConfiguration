#ifndef NSWCONFIGURATION_HW_STGCTP_H
#define NSWCONFIGURATION_HW_STGCTP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <set>

#include <boost/property_tree/ptree.hpp>

#include <ers/Issue.h>

#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

ERS_DECLARE_ISSUE(nsw,
                  STGCTPIssue,
                  message,
                  ((std::string)message)
                  )

namespace nsw::hw {
  /**
   * \brief Class representing a STGC Trigger Processor
   *
   * Provides methods to read/write individual STGC TP registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   */
  class STGCTP : public ScaAddressBase, public OpcConnectionBase
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
    void writeRegister(std::uint32_t regAddress,
                       std::uint32_t value) const;

    /**
     * \brief Read an individual STGCTP register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::uint32_t regAddress) const;

    /**
     * \brief Write a value to a STGCTP register address, and read it back
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(std::uint32_t regAddress,
                                  std::uint32_t value) const;

    /**
     * \brief Get the "SkipRegisters" provided by the user configuration
     */
    std::set<std::uint8_t> SkipRegisters() const;

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

    /**
     * \brief Read the reset command from the ptree config
     */
    [[nodiscard]]
    bool getDoReset() const;

  private:
    boost::property_tree::ptree m_config; //!< ptree object associated with this STGCTP
    std::string m_scaAddressFPGA;         //!< SCA address of STGCTP FPGA line, namely I2C_0, bus0
    std::string m_name;                   //!< Name composed of OPC and SCA addresses
    std::set<std::uint8_t> m_skippedReg;  //!< Set of registers which should be skipped

  };
}  // namespace nsw::hw

#endif
