// This file defines register name/size mapping for each I2CMaster
// The mappings are of type AddressRegisterMap, and should be declared
// static const, and passed as reference to I2CFEConfig instances

#ifndef NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
#define NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "NSWConfiguration/Types.h"

static const char ROC_ANALOG_NAME[] = "mmfe8RocPllCoreAnalog";
static const char ROC_DIGITAL_NAME[] = "mmfe8RocCoreDigital";

static const i2c::AddressRegisterMap ROC_ANALOG_REGISTERS = {
    { "ePllVMM0reg70", {{"ePllInstantLock", 1}, {"ePllReset", 1}, {"bypassPLL", 1},
                    {"ePllLockEn", 1}, {"ePllCap", 2}, {"ePllReferenceFrequency", 2}}},
    { "ePllVMM1reg86", {{"ePllInstantLock", 1}, {"ePllReset", 1}, {"bypassPLL", 1},
                    {"ePllLockEn", 1}, {"ePllCap", 2}, {"ePllReferenceFrequency", 2}}},
    { "ePllTDCreg102", {{"ePllInstantLock", 1}, {"ePllReset", 1}, {"bypassPLL", 1},
                    {"ePllLockEn", 1}, {"ePllCap", 2}, {"ePllReferenceFrequency", 2}}},
    { "register112", {{"ePllInstantLock", 1}, {"ePllReset", 1}, {"bypassPLL", 1},
                    {"ePllLockEn", 1}, {"ePllCap", 2}, {"ePllReferenceFrequency", 2}}},
    { "VMM_ENA_INVreg122", {{"VMM_ENA_INV", 8}}}
};

static const i2c::AddressRegisterMap ROC_DIGITAL_REGISTERS = {
    { "rocId", {{"l1_first", 1}, {"even_parity", 1}, {"roc_id", 6}}},
    { "elinkSpeed", { {"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}}},
    { "sroc0VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "sroc1VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "sroc2VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "sroc3VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "eopAndNullEventEnable", { {"sroc3_eop_enable", 1}, {"sroc1_eop_enable", 1},
                                 {"sroc1_eop_enable", 1}, {"sroc0_eop_enable", 1},
                                 {"sroc3_nullevt_enable", 1}, {"sroc2_nullevt_enable", 1},
                                 {"sroc1_nullevt_enable", 1}, {"sroc0_nullevt_enable", 1} } },

    { "srocEnable", {{"bypass", 1}, {"timeoutEnable", 1}, {"TTCStartBits", 2},
                         {"enableSROC3", 1}, {"enableSROC2", 1}, {"enableSROC1", 1}, {"enableSROC0", 1}}},

    { "vmmEnable", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                     {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },

    { "timeout", { {"timeout", 8} } },
    { "bcOffset0_txcSel", { {"tx_csel", 4}, {"bc_offset11_8", 4} } },
    { "bcOffset1", { {"bc_offset7_0", 8} } },
    { "bcRollover0", { {"NOT_USED", 4}, {"bc_rollover11_8", 4} } },
    { "bcRollover1", { {"bc_rollover7_0", 8} } },

    { "eportEnable", { {"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}}},

    { "fakeVmmFailure", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                          {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },

    { "busyAndTdcEnable", { {"tdc_enable_sroc3", 1}, {"tdc_enable_sroc2", 1},
                            {"tdc_enable_sroc1", 1}, {"tdc_enable_sroc0", 1},
                            {"busy_enable_sroc3", 1}, {"busy_enable_sroc2", 1},
                            {"busy_enable_sroc1", 1}, {"busy_enable_sroc0", 1} } },


    { "busyOnLimit0", { {"NOT_USED", 5}, {"busy_on_limit10_7", 3} } },
    { "busyOnLimit1", { {"busy_on_limit8_0", 8} } },

    { "busyOffLimit0", { {"NOT_USED", 5}, {"busy_off_limit10_7", 3} } },
    { "busyOffLimit1", { {"busy_off_limit8_0", 8} } },

    { "l1EventsWithoutComma", { {"l1_events_no_comma", 8} } },

    { "timeoutStatus", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                         {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } }
};

#endif  // NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

