#include <memory>

#ifndef NSWCONFIGURATION_I2CFECONFIG_H_
#define NSWCONFIGURATION_I2CFECONFIG_H_

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/FEConfig.h"
#include "NSWConfiguration/I2cFECodec.h"

using boost::property_tree::ptree;

namespace nsw {

/*! \brief Generic I2C type Front End Configuration
*
* The base class for Front end elements who have I2C registers.
*
*/
class I2cFEConfig: public FEConfig {
 protected:
    std::unique_ptr<I2cFECodec> codec;
    i2c::AddressBitstreamMap m_address_bitstream;

 public:
    explicit I2cFEConfig(ptree config): FEConfig(config) {}
    ~I2cFEConfig() {}

    I2cFEConfig(const I2cFEConfig&) = delete;

    void dump();
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_I2CFECONFIG_H_
