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
    { "ePllVmm0Reg64", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "ePllVmm0Reg65", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "ePllVmm0Reg66", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "ePllVmm0Reg67", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "ePllVmm0Reg68", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "ePllVmm0Reg69", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "ePllVmm0Reg70", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "ePllVmm0Reg71", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "ePllVmm0Reg72", {
        {"ePllEnablePhase", 8}
    } },
    { "ePllVmm0Reg73", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "ePllVmm0Reg74", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "ePllVmm0Reg75", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm0Reg76", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm0Reg77", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm0Reg78", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm0Reg79", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "ePllVmm1Reg80", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "ePllVmm1Reg81", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "ePllVmm1Reg82", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "ePllVmm1Reg83", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "ePllVmm1Reg84", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "ePllVmm1Reg85", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "ePllVmm1Reg86", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "ePllVmm1Reg87", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "ePllVmm1Reg88", {
        {"ePllEnablePhase", 8}
    } },
    { "ePllVmm1Reg89", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "ePllVmm1Reg90", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "ePllVmm1Reg91", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm1Reg92", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm1Reg93", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm1Reg94", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllVmm1Reg95", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "ePllTdcReg96", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "ePllTdcReg97", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "ePllTdcReg98", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "ePllTdcReg99", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "ePllTdcReg100", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "ePllTdcReg101", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "ePllTdcReg102", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "ePllTdcReg103", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "ePllTdcReg104", {
        {"ePllEnablePhase", 8}
    } },
    { "ePllTdcReg105", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "ePllTdcReg106", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "ePllTdcReg107", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllTdcReg108", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllTdcReg109", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllTdcReg110", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "ePllTdcReg111", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "register112", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "register113", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "register114", {
        {"ePllEnablePhase", 8}
    } },
    { "register115", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "register116", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "register117", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "register118", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "register119", {
        {"tp_bypass_global", 1},
        {"tp_phase_global", 3},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "register120", {
        {"TDS_BCR_INV", 4},
        {"LockOutInv", 1},
        {"testOutEn", 1},
        {"testOutMux", 2}
    } },
    { "vmmBcrInv", {
        {"vmmBcrInv", 8}
    } },
    { "vmmEnaInv", {
        {"vmmEnaInv", 8}
    } },
    { "vmaL0Inv", {
        {"vmmL0Inv", 8}
    } },
    { "vmaTpInv", {
        {"vmmTpInv", 8}
    } },
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

