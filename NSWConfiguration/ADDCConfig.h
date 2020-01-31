/// ADDC Configuration class

#ifndef NSWCONFIGURATION_ADDC_H_
#define NSWCONFIGURATION_ADDC_H_

#include <vector>
#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/SCAConfig.h"
#include "NSWConfiguration/ARTConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class ADDCConfig: public FEConfig {
 private:
    std::vector<ARTConfig> m_arts;
    std::vector<uint8_t> m_ARTCoreregisters;
    std::vector<uint8_t> m_ARTregisters;
    std::vector<uint8_t> m_ARTregistervalues;
    std::vector<uint8_t> m_GBTx_eport_registers;
    std::vector<uint8_t> m_GBTx_ConfigurationData;

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit ADDCConfig(ptree config);
    ~ADDCConfig() {}

    void dump();

    ARTConfig & getART(size_t i) {return m_arts[i];}
    const std::vector<ARTConfig> & getARTs() const {return m_arts;}

    std::vector<uint8_t> ARTCoreregisters()       const {return m_ARTCoreregisters;}
    std::vector<uint8_t> ARTregisters()           const {return m_ARTregisters;}
    std::vector<uint8_t> ARTregistervalues()      const {return m_ARTregistervalues;}
    std::vector<uint8_t> GBTx_eport_registers()   const {return m_GBTx_eport_registers;}
    std::vector<uint8_t> GBTx_ConfigurationData() const {return m_GBTx_ConfigurationData;}

};

}  // namespace nsw

#endif  // NSWCONFIGURATION_ADDC_H_
