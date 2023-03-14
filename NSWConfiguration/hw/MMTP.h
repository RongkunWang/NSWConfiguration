#ifndef NSWCONFIGURATION_HW_MMTP_H
#define NSWCONFIGURATION_HW_MMTP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include <boost/property_tree/ptree.hpp>

#include <ers/Issue.h>

#include "NSWConfiguration/hw/ScaAddressBase.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"

ERS_DECLARE_ISSUE(nsw,
                  MMTPVersionMissing,
                  message,
                  ((std::string)message)
                  )

namespace nsw::hw {
  class MMTP_v1;
  class MMTP_v2;

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
    std::string getName() const;

    /**
     * \brief Read the full MMTP address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::string, std::uint32_t> readConfiguration() const;

    /**
     * \brief Write the full MMTP configuration
     */
    void writeConfiguration(bool doAlignArtGbtx = true) const;


    /**
     * \brief Write an individual MMTP register by its name
     *
     * \param regAddress is the address the register, containing I2C bus and secondary name
     * \param value is the value to be written
     */
    void writeRegister(std::string_view regAddress,
                       const std::uint32_t value) const;

    /**
     * \brief Read an individual MMTP register by its name
     *
     * \param regAddress is the address the register, 
     *      containing I2C bus and secondary name
     */
    [[nodiscard]]
    std::uint32_t readRegister(std::string_view regAddress) const;

    /**
     * \brief Write a value to a MMTP register address, and read it back
     *
     * \param regAddress is the address the register, 
     *      containing I2C bus and secondary name
     * \param value is the value to be written
     */
    void writeAndReadbackRegister(std::string_view regAddress,
                                  const std::uint32_t value) const;

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
    boost::property_tree::ptree& getConfig();
    [[nodiscard]]
    const boost::property_tree::ptree& getConfig() const;  //!< \overload

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
    void enableChannelRates(const bool enable) const;

  private:

    // set default version
    int m_version = 0;
    std::shared_ptr<nsw::hw::MMTP_v1> m_impl1;
    std::shared_ptr<nsw::hw::MMTP_v2> m_impl2;
  };
} // namespace nsw::hw

#endif
