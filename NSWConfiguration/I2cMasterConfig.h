#ifndef NSWCONFIGURATION_I2CMASTERCONFIG_H_
#define NSWCONFIGURATION_I2CMASTERCONFIG_H_

#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Types.h"

#include "ers/Issue.h"

ERS_DECLARE_ISSUE(nsw,
                  WriteToReadOnlyRegister,
                  "Cannot write to readonly register: " << message,
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  NoSuchI2cAddress,
                  "No I2C element with this address: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  NoSuchI2cRegister,
                  "No I2C register with this name: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  MissingI2cAddress,
                  "I2c element missing from configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  MissingI2cRegister,
                  "I2c register missing from configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  I2cSizeMismatch,
                  "Vector size doesn't match I2c bits: " << address
                        << ", vector size: " << vecsize
                        << ", i2c address total size: " << i2csize,
                  ((std::string)address) ((size_t) vecsize) ((size_t) i2csize)
                  )

namespace nsw {

/*! \brief Generic I2C type Front End Codec
*
* The base class for Front end codec
* Simplifies converting configuration into bytestream.
*
*/
class I2cMasterCodec {
 public:
    explicit I2cMasterCodec(const i2c::AddressRegisterMap & ar_map);
    ~I2cMasterCodec() = default;

    /** \brief Method that creates bitstreams from config tree
     *  Iterates through m_addr_reg and creates a bitstream. Throws if the ptree does
     *  not contain all registers.
     *  \param config Configuration ptree
     *  \return Bitstream map register name : bitstream
     */
    i2c::AddressBitstreamMap buildConfig(const boost::property_tree::ptree& config) const;

    /** \brief Method to create a bitstream from a partial config tree
     *  Iterates through the ptree and creates a bitstream. Throws if it contains
     *  unknown registers.
     *  \param config Configuration ptree
     *  \return Bitstream map register name : bitstream
     */
    i2c::AddressBitstreamMap buildPartialConfig(const boost::property_tree::ptree& config) const;

    /// Map of i2c addresses, to a map of registers to positions in the bitstream
    i2c::AddressRegisterSizeMap m_addr_reg_pos;

    /// Map of i2c addresses, to a map of registers to register sizes
    i2c::AddressRegisterSizeMap m_addr_reg_size;

    /// Return total size in bits, of registers in an i2c address
    size_t getTotalSize(const std::string& address) const;

    /// Return addresses of slaves the I2c master
    std::vector<std::string> getAddresses() const;

 protected:
    /// Map of i2c addresses and internal mapping of registers
    const i2c::AddressRegisterMap& m_addr_reg;

    /// Map of i2c addresses and total sizes
    i2c::AddressSizeMap m_addr_size;

    // Fills m_addr_size
    void calculateSizesAndPositions();
};

/*! \brief Generic I2C type Front End Configuration
*
* The base class for Front end elements who have I2C registers.
*
*/
class I2cMasterConfig {
 protected:
    // derived classes should have their own Codec types derived from I2cMasterCodec
    boost::property_tree::ptree m_config;
    std::string m_name;  // Name of I2cMaster, used in Opc Address
    I2cMasterCodec m_codec;
    i2c::AddressBitstreamMap m_address_bitstream;  /// Map of I2c addresses(string) and bitstreams(string)

 public:
    /** \brief Constructor
     *  Constructs I2cMasterConfig object and builds the bitstream
     *  \param config Configuration ptree
     *  \param name Register address space name (e.g. rocPllCoreAnalog). See I2cRegisterMappings.h
     *  \param reg Register address map (e.g. ROC_ANALOG_REGISTERS). See I2cRegisterMappings.h
     *  \param partial Switch if the config argument is a full configuration or a partial transaction
     */
    explicit I2cMasterConfig(const boost::property_tree::ptree& config, const std::string& name, const i2c::AddressRegisterMap & reg, const bool partial=false):
        m_config(config), m_name(name), m_codec(reg), m_address_bitstream(buildConfig(config, partial)) {
        }

    std::string getName() const { return m_name;}

    const boost::property_tree::ptree & getConfig() const {return m_config;}

    void setName(const std::string& name) { m_name = name;}

    i2c::AddressBitstreamMap getBitstreamMap() const { return m_address_bitstream;}

    /// Set value of register by changing the corresponding bits in in m_address_bitstream
    void setRegisterValue(const std::string& address, const std::string& register_name, uint32_t value);

    /// Get value of register from m_address_bitstream
    uint32_t getRegisterValue(const std::string& address, const std::string& register_name) const;

    /// Decode vector of bytes into register values for a certain address
    void decodeVector(const std::string& address, const std::vector<uint8_t>& vec) const;

    /// Return addresses of slaves the I2c master
    std::vector<std::string> getAddresses() const { return m_codec.getAddresses(); }

    /// Return address positions of the I2c master
    const i2c::AddressRegisterSizeMap& getAddressPositions() const { return m_codec.m_addr_reg_pos; }

    /// Return address sizes of the I2c master
    const i2c::AddressRegisterSizeMap& getAddressSizes() const { return m_codec.m_addr_reg_size; }

    /// Return total size of registers in an i2c address
    size_t getTotalSize(const std::string& address) const { return m_codec.getTotalSize(address); }

    // Following may be needed if codec is declared as unique_ptr
    // I2cMasterConfig(const I2cMasterConfig&) = delete;

    void dump() const;

protected:
    /** \brief Build the bitstream map
     *  Calls the corresponding buildConfig function of the codec.
     *  \param config configuration ptree
     *  \param partialConfig Switch if the ptree represents a full configuration object ("old buildConfig")
     *                       or a partial transaction
     *  \return map of register name to bitstream
     */
    i2c::AddressBitstreamMap buildConfig(const boost::property_tree::ptree& config, const bool partialConfig) {
      if (not partialConfig)
      {
        return m_codec.buildConfig(config);
      }
      else
      {
        return m_codec.buildPartialConfig(config);
      }
    }
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_I2CMASTERCONFIG_H_
