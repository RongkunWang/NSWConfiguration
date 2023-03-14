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
     * \brief Constrctor from a \ref ptree object
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
    std::map<std::string, std::uint32_t> readConfiguration() const;

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
    void writeRegister(std::string regAddress,
                       std::uint32_t value) const;

    /**
     * \brief Read an individual STGCTP register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::string regAddress) const;

    /**
     * \brief Write a value to a STGCTP register address, and read it back
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(std::string regAddress,
                                  std::uint32_t value) const;


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
     * \brief Get DoReset from the user config
     * this is a sw register, doesn't exist on fw
     */
    [[nodiscard]]
    bool getDoReset() const { return m_config.get("DoReset", true); };


    /**
     * \brief Get L1AOpeningOffset from the user config
     */
    [[nodiscard]]
    std::uint32_t getL1AOpeningOffset() const { return m_config.get("L1AOpeningOffset", std::uint32_t{0x22}); };

    /**
     * \brief Get L1ARequestOffset from the user config
     */
    [[nodiscard]]
    std::uint32_t getL1ARequestOffset() const { return m_config.get("L1ARequestOffset", std::uint32_t{0x20}); };

    /**
     * \brief Get L1AClosingOffset from the user config
     */
    [[nodiscard]]
    std::uint32_t getL1AClosingOffset() const { return m_config.get("L1AClosingOffset", std::uint32_t{0x1e}); };

    /**
     * \brief Get L1ATimeoutWindow from the user config
     */
    [[nodiscard]]
    std::uint32_t getL1ATimeoutWindow() const { return m_config.get("L1ATimeoutWindow", std::uint32_t{0x60}); };

    /**
     * \brief Get L1APadEnable from the user config
     */
    [[nodiscard]]
    bool getL1APadEnable() const { return m_config.get("L1APadEnable", true); };

    /**
     * \brief Get L1AMergeEnable from the user config
     */
    [[nodiscard]]
    bool getL1AMergeEnable() const { return m_config.get("L1AMergeEnable", true); };

    /**
     * \brief Get GlobalSyncBcidOffset from the user config
     */
    [[nodiscard]]
    std::uint32_t getGlobalSyncBcidOffset() const { return m_config.get("GlobalSyncBcidOffset", std::uint32_t{0x0}); };

    /**
     * \brief Get Busy from the user config
     */
    [[nodiscard]]
    bool getBusy() const { return m_config.get("Busy", false); };

    /**
     * \brief Get MonitoringDisable from the user config
     */
    [[nodiscard]]
    bool getMonitoringDisable() const { return m_config.get("MonitoringDisable", false); };

    /**
     * \brief Get NSWMONLimit from the user config
     */
    [[nodiscard]]
    std::uint32_t getNSWMONLimit() const { return m_config.get("NSWMONLimit", std::uint32_t{10}); };

    /**
     * \brief Get MonitoringLimit from the user config
     */
    [[nodiscard]]
    std::uint32_t getMonitoringLimit() const { return m_config.get("MonitoringLimit", std::uint32_t{100197}); };

    /**
     * \brief Get MMNSWMONEnable from the user config
     */
    [[nodiscard]]
    bool getMMNSWMONEnable() const { return m_config.get("MMNSWMONEnable", true); };

    /**
     * \brief Get SmallSector from the user config
     */
    [[nodiscard]]
    bool getSmallSector() const { return m_config.get("SmallSector", false); };

    /**
     * \brief Get NoStretch from the user config
     */
    [[nodiscard]]
    bool getNoStretch() const { return m_config.get("NoStretch", false); };

    /**
     * \brief reset the trigger processor
     */
    void doReset() const;

  private:
    boost::property_tree::ptree m_config; //!< ptree object associated with this STGCTP
    std::string m_name;                   //!< Name composed of OPC and SCA addresses
    std::set<std::string> m_skippedReg;  //!< Set of registers which should be skipped

  };
}  // namespace nsw::hw

#endif
