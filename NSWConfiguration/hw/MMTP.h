#ifndef NSWCONFIGURATION_HW_MMTP_H
#define NSWCONFIGURATION_HW_MMTP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <ers/Issue.h>

#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"

ERS_DECLARE_ISSUE(nsw,
                  MMTPFiberAlignIssue,
                  message,
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  MMTPReadWriteIssue,
                  message,
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  MMTPReadbackMismatch,
                  message,
                  ((std::string)message)
                  )

namespace nsw::hw {
  /**
   * \brief Class representing a MM Trigger Processor
   *
   * Provides methods to read/write individual MMTP registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   */
  class MMTP : public ScaAddressBase, public OpcConnectionBase
  {
  public:
    /**
     * \brief Constrctor from a \ref ptree object
     */
    MMTP(OpcManager& manager, const boost::property_tree::ptree& config);

    /**
     * \brief Name of MM TP object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Read the full MMTP address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::uint32_t, std::uint32_t> readConfiguration() const;

    /**
     * \brief Write the full MMTP configuration
     */
    void writeConfiguration(bool doAlignArtGbtx = true) const;

    /**
     * \brief Write a value to a MMTP register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister(std::uint32_t regAddress,
                       std::uint32_t value) const;

    /**
     * \brief Read an individual MMTP register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::uint32_t regAddress) const;

    /**
     * \brief Write a value to a MMTP register address, and read it back
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(std::uint32_t regAddress,
                                  std::uint32_t value) const;

    /**
     * \brief toggle idle state to high for all trigger electronics
     */
    void toggleIdleStateHigh() const;

    /**
     * \brief Get the ptree object associated with this MMTP object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]]
    boost::property_tree::ptree& getConfig() { return m_config; }
    [[nodiscard]]
    const boost::property_tree::ptree& getConfig() const { return m_config; }  //!< \overload

    /**
     * \brief Read the ART ASIC "alignment" register
     *
     * \param n_reads Number of times to read the register
     */
    std::vector<std::uint32_t> readAlignment(const size_t n_reads) const;

    /**
     * \brief set the horx environment monitoring register
     * \param tx/rx
     * \param microPod(1-3)
     * \param temp for temperature, 
     * \param loss for loss of the fiber
     * \param fiber(0-11)
     */
    void setHorxEnvMonAddr(bool tx, std::uint8_t microPod, bool temp, bool loss, std::uint8_t fiber) const;


    /**
     * \brief Enable the channel rate based on the register
     * \param enable enable (true) or disable (false) the channel rate elink
     */
    void enableChannelRates(const bool enable) const { writeRegister(nsw::mmtp::REG_CHAN_RATE_ENABLE, static_cast<uint32_t>(enable)); }

  private:

    /**
     * \brief Do "alignment" of ART ASIC input data capture at the TP,
     *        where QPLL resets are requested until the data captured from all fibers
     *        has no errors
     */
    void alignArtGbtx() const;


    /**
     * \brief Get the "L1AOpeningOffset" provided by the user configuration
     */
    std::uint32_t L1AOpeningOffset() const { return m_config.get<std::uint32_t>("L1AOpeningOffset"); }

    /**
     * \brief Get the "L1ARequestOffset" provided by the user configuration
     */
    std::uint32_t L1ARequestOffset() const { return m_config.get<std::uint32_t>("L1ARequestOffset"); }

    /**
     * \brief Get the "L1AClosingOffset" provided by the user configuration
     */
    std::uint32_t L1AClosingOffset() const { return m_config.get<std::uint32_t>("L1AClosingOffset"); }

    /**
     * \brief Get the "L1AClosingOffset" provided by the user configuration
     */
    std::uint32_t L1ATimeoutWindow() const { return m_config.get<std::uint32_t>("L1ATimeoutWindow"); }

    /**
     * \brief Get the "FiberBCOffset" provided by the user configuration
     */
    std::uint32_t FiberBCOffset() const { return m_config.get<std::uint32_t>("FiberBCOffset"); }

    /**
     * \brief Get the "SelfTriggerDelay" provided by the user configuration
     */
    std::uint32_t SelfTriggerDelay() const { return m_config.get<std::uint32_t>("SelfTriggerDelay"); }

    /**
     * \brief Get the "VmmMaskHotThresh" provided by the user configuration
     */
    std::uint32_t VmmMaskHotThresh() const { return m_config.get("VmmMaskHotThresh", std::uint32_t{0xA}); }

    /**
     * \brief Get the "VmmMaskHotThreshHyst" provided by the user configuration
     */
    std::uint32_t VmmMaskHotThreshHyst() const { return m_config.get("VmmMaskHotThreshHyst", std::uint32_t{0x3}); }

    /**
     * \brief Get the "VmmMaskDrainPeriod" provided by the user configuration
     */
    std::uint32_t VmmMaskDrainPeriod() const { return m_config.get("VmmMaskDrainPeriod", std::uint32_t{0x9c40}); }

    /**
     * \brief Get the "L1ALatencyScanStart" provided by the user configuration
     */
    std::uint32_t L1ALatencyScanStart() const { return m_config.get("L1ALatencyScanStart", std::uint32_t{0}); }

    /**
     * \brief Get the "L1ALatencyScanEnd" provided by the user configuration
     */
    std::uint32_t L1ALatencyScanEnd() const { return m_config.get("L1ALatencyScanEnd", std::uint32_t{50}); }

    /**
     * \brief Get the "HorxEnvMonAddr" provided by the user configuration
     */
    std::uint32_t HorxEnvMonAddr() const { return m_config.get("HorxEnvMonAddr", std::uint32_t{0}); }

    /**
     * \brief Get the "gloSyncIdleState" provided by the user configuration; write to 0 will not overwrite an idle state of 1 because idle state from 1-->0 is triggered by OCR
     */
    std::uint32_t gloSyncIdleState() const { return m_config.get("gloSyncIdleState", std::uint32_t{0}); }

    /**
     * \brief Get the "gloSyncBcidOffset" provided by the user configuration
     */
    std::uint32_t gloSyncBcidOffset() const { return m_config.get("gloSyncBcidOffset", std::uint32_t{0}); }

    /**
     * \brief Get the "fiberRemapSel" provided by the user configuration; 
     * 0: default
     * 1: additional swap fiber and switch to use star-1, C06, C1516
     * 2: star-2 and star-3 swap(outer two out of the three). C08
     * For latest, see https://espace.cern.ch/ATLAS-NSW-ELX/_layouts/15/WopiFrame.aspx?sourcedoc=/ATLAS-NSW-ELX/Shared%20Documents/NSW%20Trigger%20Processor/NSWTP_Connections.pptx
     */
    std::uint32_t fiberRemapSel() const { return m_config.get("fiberRemapSel", std::uint32_t{0}); }

    /**
     * \brief Get the "SkipRegisters" provided by the user configuration
     */
    std::set<std::uint8_t> SkipRegisters() const;


    /**
     * \brief Get the "SkipFibers" provided by the user configuration
     */
    std::set<std::uint8_t> SkipFibers() const;

    boost::property_tree::ptree m_config; //!< config object associated with this MMTP
    std::string m_busAddress;             //!< Address of I2C bus
    std::string m_name;                   //!< Name composed of OPC and SCA addresses

  };
} // namespace nsw::hw

#endif
