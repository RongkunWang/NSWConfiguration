#include <utility>

#include "NSWConfiguration/ROCCodec.h"
#include "NSWConfiguration/I2cFECodec.h"
#include "NSWConfiguration/Utility.h"

nsw::ROCCodec::ROCCodec() {
    setRegisterMapping();
    calculateTotalSizes();
}

// This function has to be specific for each FE
void nsw::ROCCodec::setRegisterMapping() {
    i2c::RegisterSizeVector reg_size;

    reg_size = {{"l1_first", 1}, {"even_parity", 1}, {"roc_id", 6}};
    m_addr_reg["0"] = std::move(reg_size);

    reg_size = {{"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}};
    m_addr_reg["1"] = std::move(reg_size);
}
