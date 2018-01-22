#include <memory>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ROCConfig.h"

using boost::property_tree::ptree;

nsw::ROCCodec::ROCCodec() {
    setRegisterMapping();
    calculateTotalSizes();
}

// This function has to be specific for each FE
void nsw::ROCCodec::setRegisterMapping() {
    i2c::RegisterSizeVector reg_size;

    // reg_size = {{"l1_first", 1}, {"even_parity", 1}, {"roc_id", 6}};
    // m_addr_reg["0"] = std::move(reg_size);

    // reg_size = {{"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}};
    // m_addr_reg["1"] = std::move(reg_size);

     reg_size = {{"ePllInstantLock", 1}, {"ePllReset", 1}, {"bypassPLL", 1},
                 {"ePllLockEn", 1}, {"ePllCap", 2}, {"ePllReferenceFrequency", 2}};
     m_addr_reg["ePllVMM0reg70"] = reg_size;
     m_addr_reg["ePllVMM1reg86"] = reg_size;
     m_addr_reg["ePllTDCreg102"] = reg_size;
     m_addr_reg["Register112"] = reg_size;

     reg_size = {{"VMM_ENA_INV", 8}};
     m_addr_reg["VMM_ENA_INVreg122"] = std::move(reg_size);
}

nsw::ROCConfig::ROCConfig(ptree config): I2cFEConfig(config) {
    codec = std::make_unique<ROCCodec>();
    m_address_bitstream = codec->buildConfig(config);
}
