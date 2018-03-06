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
    ~I2cMasterCodec() { ERS_LOG("Destroying I2cMasterCodec");}

    // Method that created bitstreams from config tree.
    i2c::AddressBitstreamMap buildConfig(ptree config);

    /// Map of i2c addresses, to a map of registers to positions in the bitstream
    i2c::AddressRegisterSizeMap m_addr_reg_pos;

    /// Map of i2c addresses, to a map of registers to register sizes
    i2c::AddressRegisterSizeMap m_addr_reg_size;


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
    i2c::AddressBitstreamMap m_address_bitstream;

 public:
    explicit I2cMasterConfig(ptree config, std::string name, const i2c::AddressRegisterMap & reg):
        m_config(config), m_name(name), m_codec(reg) {
            m_address_bitstream = m_codec.buildConfig(config);
        }

    ~I2cMasterConfig() {}

    std::string getName() const { return m_name;}

    i2c::AddressBitstreamMap getBitstreamMap() const { return m_address_bitstream;}

    // Set value of register by changing the corresponding bits in in m_address_bitstream
    void setRegisterValue(std::string address, std::string register_name, uint32_t value);

    // Get value of register from m_address_bitstream
    uint32_t getRegisterValue(std::string address, std::string register_name);

    // Following may be needed if codec is declared as unique_ptr
    // I2cMasterConfig(const I2cMasterConfig&) = delete;

    void dump();
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_I2CMASTERCONFIG_H_
