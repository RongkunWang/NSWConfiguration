#include <memory>
#include <iostream>
#include <utility>
#include <map>
#include <string>
#include <vector>


#ifndef NSWCONFIGURATION_I2CMASTERCONFIG_H_
#define NSWCONFIGURATION_I2CMASTERCONFIG_H_

#include "boost/property_tree/ptree.hpp"

#include "ers/ers.h"

#include "NSWConfiguration/Types.h"

using boost::property_tree::ptree;

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
    ~I2cMasterCodec() {}

    // Method that creates bitstreams from config tree.
    i2c::AddressBitstreamMap buildConfig(ptree config);

    /// Map of i2c addresses, to a map of registers to positions in the bitstream
    i2c::AddressRegisterSizeMap m_addr_reg_pos;

    /// Map of i2c addresses, to a map of registers to register sizes
    i2c::AddressRegisterSizeMap m_addr_reg_size;

    /// Return total size in bits, of registers in an i2c address
    size_t getTotalSize(std::string address) {return m_addr_size[address];}

    /// Return addresses of slaves the I2c master
    std::vector<std::string> getAddresses();

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
    ptree m_config;
    std::string m_name;  // Name of I2cMaster, used in Opc Address
    I2cMasterCodec m_codec;
    i2c::AddressBitstreamMap m_address_bitstream;  /// Map of I2c addresses(string) and bitstreams(string)

 public:
    explicit I2cMasterConfig(ptree config, std::string name, const i2c::AddressRegisterMap & reg):
        m_config(config), m_name(name), m_codec(reg) {
            buildConfig(config);
        }

    ~I2cMasterConfig() {}

    void buildConfig(ptree config) {
      m_address_bitstream = m_codec.buildConfig(config);
    }

    std::string getName() const { return m_name;}

    const ptree & getConfig() const {return m_config;}

    void setName(std::string name) { m_name = name;}

    i2c::AddressBitstreamMap getBitstreamMap() const { return m_address_bitstream;}

    /// Set value of register by changing the corresponding bits in in m_address_bitstream
    void setRegisterValue(std::string address, std::string register_name, uint32_t value);

    /// Get value of register from m_address_bitstream
    uint32_t getRegisterValue(std::string address, std::string register_name);

    /// Decode vector of bytes into register values for a certain address
    void decodeVector(std::string address, std::vector<uint8_t> vec);

    /// Return addresses of slaves the I2c master
    std::vector<std::string> getAddresses() { return m_codec.getAddresses();}

    /// Return total size of registers in an i2c address
    size_t getTotalSize(std::string address) { return m_codec.getTotalSize(address);}

    // Following may be needed if codec is declared as unique_ptr
    // I2cMasterConfig(const I2cMasterConfig&) = delete;

    void dump();
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_I2CMASTERCONFIG_H_
