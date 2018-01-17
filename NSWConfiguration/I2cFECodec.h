#ifndef NSWCONFIGURATION_I2CFECODEC_H_
#define NSWCONFIGURATION_I2CFECODEC_H_

#include <bitset>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

namespace nsw {

namespace i2c {
    using RegisterAndSize = std::pair<std::string, size_t>;

    // Ordered map of register names and sizes in bits
    using RegisterSizeVector = std::vector<RegisterAndSize>;

    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;
    using AddressSizeMap = std::map<std::string, size_t>;

    using AddressBitstreamMap = std::map<std::string, std::string>;
}  // namespace i2c

/*! \brief Generic I2C type Front End Codec
*
* The base class for Front end codec
* Simplifies converting configuration into bytestream.
*
*/
class I2cFECodec {
 public:
    I2cFECodec() {}
    ~I2cFECodec() { std::cout << "Destroying I2cFECodec" << std::endl;}

    // Method that created bitstreams from config tree.
    i2c::AddressBitstreamMap buildConfig(ptree config);

 protected:
    /// Map of i2c addresses and internal mapping of registers
    i2c::AddressRegisterMap m_addr_reg;

    /// Map of i2c addresses and total sizes
    i2c::AddressSizeMap m_addr_size;

    // Fills m_addr_size
    void calculateTotalSizes();

    /// Function that fills m_addr_reg and should be implemented in the derived class
    virtual void setRegisterMapping() {}
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_I2CFECODEC_H_
