#ifndef NSWCONFIGURATION_HW_MMTP_v2_H
#define NSWCONFIGURATION_HW_MMTP_v2_H

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


namespace nsw::hw {
  /**
   * \brief Class representing a MM Trigger Processor
   *
   * Provides methods to read/write individual MMTP_v2 registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   */
  class MMTP_v2 : public ScaAddressBase, public OpcConnectionBase
  {
  public:
    /**
     * \brief default constructor
     */
    // MMTP_v2(){};
    /**
     * \brief Constrctor from a \ref ptree object
     */
    MMTP_v2(OpcManager& manager, const boost::property_tree::ptree& config);

    /**
     * \brief Name of MM TP object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Read the full MMTP_v2 address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::string, std::uint32_t> readConfiguration() const;

    /**
     * \brief Write the full MMTP_v2 configuration
     */
    void writeConfiguration(bool doAlignArtGbtx = true) const;


    /**
     * \brief Write an individual MMTP_v2 register by its name
     *
     * \param regAddress is the address the register, containing I2C bus and secondary name
     * \param value is the value to be written
     */
    void writeRegister(std::string_view regAddress,
                       const std::uint32_t value) const;

    /**
     * \brief Read an individual MMTP_v2 register by its name
     *
     * \param regAddress is the address the register, 
     *      containing I2C bus and secondary name
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::string_view regAddress) const;

    /**
     * \brief Write a value to a MMTP_v2 register address, and read it back
     *
     * \param regAddress is the address the register, 
     *      containing I2C bus and secondary name
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(std::string_view regAddress,
                                  const std::uint32_t value) const;

    std::string getBusAddress(std::uint8_t bus) const {
      return fmt::format("{0}.I2C_{1:d}.bus{1:d}", getScaAddress(), bus) ;
    }

    /**
     * \brief toggle idle state to high for all trigger electronics
     */
    void toggleIdleStateHigh() const;

    /**
     * \brief Get the ptree object associated with this MMTP_v2 object
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
     * \brief Get the "SkipRegisters" provided by the user configuration
     */
    std::set<std::uint8_t> SkipRegisters() const;


    /**
     * \brief Get the "SkipFibers" provided by the user configuration
     */
    boost::property_tree::ptree m_config; //!< ptree object associated with this MMTP_v2
    std::set<std::uint8_t> SkipFibers() const;

    std::string m_name;                   //!< Name composed of OPC and SCA addresses
    std::set<std::string_view> m_skippedReg;  //!< Set of registers which should be skipped

  };
} // namespace nsw::hw

#endif
