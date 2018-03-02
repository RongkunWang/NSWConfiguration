// This file defines register name/size mapping for each I2CMaster
// The mappings are of type AddressRegisterMap, and should be declared
// static const, and passed as reference to I2CFEConfig instances

// Each I2c mapping is of type AddressRegisterMap, a map such with:
// key: Address of I2c main registers
// value: RegisterSizeVector: a vector of pairs as: {subregisterName, subregister size in its}

#ifndef NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
#define NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "NSWConfiguration/Types.h"

static const char ROC_ANALOG_NAME[] = "rocPllCoreAnalog";
static const char ROC_DIGITAL_NAME[] = "rocCoreDigital";

static const i2c::AddressRegisterMap ROC_ANALOG_REGISTERS = {
    { "reg064ePllVmm0", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg065ePllVmm0", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "reg066ePllVmm0", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "reg067ePllVmm0", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "reg068ePllVmm0", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "reg069ePllVmm0", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "reg070ePllVmm0", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "reg071ePllVmm0", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "reg072ePllVmm0", {
        {"ePllEnablePhase", 8}
    } },
    { "reg073ePllVmm0", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "reg074ePllVmm0", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "reg075ePllVmm0", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg076ePllVmm0", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg077ePllVmm0", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg078ePllVmm0", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg079ePllVmm0", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "reg080ePllVmm1", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg081ePllVmm1", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "reg082ePllVmm1", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "reg083ePllVmm1", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "reg084ePllVmm1", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "reg085ePllVmm1", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "reg086ePllVmm1", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "reg087ePllVmm1", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "reg088ePllVmm1", {
        {"ePllEnablePhase", 8}
    } },
    { "reg089ePllVmm1", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "reg090ePllVmm1", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "reg091ePllVmm1", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg092ePllVmm1", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg093ePllVmm1", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg094ePllVmm1", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg095ePllVmm1", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "reg096ePllTdc", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg097ePllTdc", {
        {"ePllPhase160MHz_1[4]", 1},
        {"ePllPhase40MHz_1", 7}
    } },
    { "reg098ePllTdc", {
        {"ePllPhase160MHz_2[4]", 1},
        {"ePllPhase40MHz_2", 7}
    } },
    { "reg099ePllTdc", {
        {"ePllPhase160MHz_3[4]", 1},
        {"ePllPhase40MHz_3", 7}
    } },
    { "reg100ePllTdc", {
        {"ePllPhase160MHz_1[3:0]", 4},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "reg101ePllTdc", {
        {"ePllPhase160MHz_3[3:0]", 4},
        {"ePllPhase160MHz_2[3:0]", 4}
    } },
    { "reg102ePllTdc", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "reg103ePllTdc", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "reg104ePllTdc", {
        {"ePllEnablePhase", 8}
    } },
    { "reg105ePllTdc", {
        {"tp_bypass_1", 1},
        {"tp_phase_1", 3},
        {"tp_bypass_0", 1},
        {"tp_phase_0", 3}
    } },
    { "reg106ePllTdc", {
        {"tp_bypass_3", 1},
        {"tp_phase_3", 3},
        {"tp_bypass_2", 1},
        {"tp_phase_2", 3}
    } },
    { "reg107ePllTdc", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg108ePllTdc", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg109ePllTdc", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg110ePllTdc", {
        {"ctrl_05delay_0", 1},
        {"ctrl_delay_0", 3},
        {"ctrl_bypass_0", 1},
        {"ctrl_phase_0", 3}
    } },
    { "reg111ePllTdc", {
        {"tx_enable", 4},
        {"tx_csel", 4}
    } },
    { "reg112", {
        {"ePllInstantLock", 1},
        {"ePllReset", 1},
        {"bypassPLL", 1},
        {"ePllLockEn", 1},
        {"ePllReferenceFrequency", 2},
        {"ePllCap", 2}
    } },
    { "reg113", {
        {"ePllRes", 4},
        {"ePllIcp", 4}
    } },
    { "reg114", {
        {"ePllEnablePhase", 8}
    } },
    { "reg115", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg116", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg117", {
        {"ePllPhase160MHz_0[4]", 1},
        {"ePllPhase40MHz_0", 7}
    } },
    { "reg118", {
        {"ePllPhase160MHz_0[0:3]", 4},
        {"ePllPhase160MHz_1[0:3]", 4}
    } },
    { "reg119", {
        {"tp_bypass_global", 1},
        {"tp_phase_global", 3},
        {"ePllPhase160MHz_0[3:0]", 4}
    } },
    { "reg120", {
        {"TDS_BCR_INV", 4},
        {"LockOutInv", 1},
        {"testOutEn", 1},
        {"testOutMux", 2}
    } },
    { "reg121vmmBcrInv", {
        {"vmmBcrInv", 8}
    } },
    { "reg122vmmEnaInv", {
        {"vmmEnaInv", 8}
    } },
    { "reg123vmmL0Inv", {
        {"vmmL0Inv", 8}
    } },
    { "reg124vmmTpInv", {
        {"vmmTpInv", 8}
    } },
};

static const i2c::AddressRegisterMap ROC_DIGITAL_REGISTERS = {
    { "reg000rocId", {{"l1_first", 1}, {"even_parity", 1}, {"roc_id", 6}}},
    { "reg001elinkSpeed", { {"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}}},
    { "reg002sRoc0VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "reg003sRoc1VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "reg004sRoc2VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "reg005sRoc3VmmConnections", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                               {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },
    { "reg006eopAndNullEventEnable", { {"sroc3_eop_enable", 1}, {"sroc1_eop_enable", 1},
                                 {"sroc1_eop_enable", 1}, {"sroc0_eop_enable", 1},
                                 {"sroc3_nullevt_enable", 1}, {"sroc2_nullevt_enable", 1},
                                 {"sroc1_nullevt_enable", 1}, {"sroc0_nullevt_enable", 1} } },

    { "reg007sRocEnable", {{"bypass", 1}, {"timeoutEnable", 1}, {"TTCStartBits", 2},
                         {"enableSROC3", 1}, {"enableSROC2", 1}, {"enableSROC1", 1}, {"enableSROC0", 1}}},

    { "reg008vmmEnable", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                     {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },

    { "reg009timeout", { {"timeout", 8} } },
    { "reg010bcOffset0_txcSel", { {"tx_csel", 4}, {"bc_offset11_8", 4} } },
    { "reg011bcOffset1", { {"bc_offset7_0", 8} } },
    { "reg012bcRollover0", { {"NOT_USED", 4}, {"bc_rollover11_8", 4} } },
    { "reg013bcRollover1", { {"bc_rollover7_0", 8} } },

    { "reg014eportEnable", { {"sroc3", 2}, {"sroc2", 2}, {"sroc1", 2}, {"sroc0", 2}}},

    { "reg019fakeVmmFailure", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                          {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } },

    { "reg020busyAndTdcEnable", { {"tdc_enable_sroc3", 1}, {"tdc_enable_sroc2", 1},
                            {"tdc_enable_sroc1", 1}, {"tdc_enable_sroc0", 1},
                            {"busy_enable_sroc3", 1}, {"busy_enable_sroc2", 1},
                            {"busy_enable_sroc1", 1}, {"busy_enable_sroc0", 1} } },


    { "reg021busyOnLimit0", { {"NOT_USED", 5}, {"busy_on_limit10_7", 3} } },
    { "reg022busyOnLimit1", { {"busy_on_limit8_0", 8} } },

    { "reg023busyOffLimit0", { {"NOT_USED", 5}, {"busy_off_limit10_7", 3} } },
    { "reg024busyOffLimit1", { {"busy_off_limit8_0", 8} } },

    { "reg031l1EventsWithoutComma", { {"l1_events_no_comma", 8} } },

    { "reg063timeoutStatus", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                         {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } }
};

#endif  // NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

