// This file defines register name/size mapping for each I2CMaster
// The mappings are of type AddressRegisterMap, and should be declared
// static const, and passed as reference to I2CFEConfig instances

// Each I2c mapping is of type AddressRegisterMap, a map such with:
// key: Address of I2c main registers
// value: RegisterSizeVector: a vector of pairs as: {subregisterName, subregister size in its}

// ReadOnly I2C addresses: If an address includes READONLY in it's name, it's ignored in configuration
// and is only kept in the map to convert bits back into values

#ifndef NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
#define NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "NSWConfiguration/Types.h"

static const char ROC_ANALOG_NAME[] = "rocPllCoreAnalog";
static const char ROC_DIGITAL_NAME[] = "rocCoreDigital";

static const char TDS_NAME[] = "tds";

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
        {"enable160MHzOnBCR" , 4},
        {"enable160MHzOn40MHz", 4}
    } },
    { "reg106ePllTdc", {
        {"tx_enable_bcr", 4},
        {"tx_csel_bcr", 4}
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

    { "reg032vmmCapture0Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg033vmmCapture1Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg034vmmCapture2Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg035vmmCapture3Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg036vmmCapture4Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg037vmmCapture5Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg038vmmCapture6Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg039vmmCapture7Status_READONLY", { {"NOT_USED", 3}, {"fifo_full_err", 1}, {"coherency_err", 1},
                                            {"dec_err", 1}, {"misaligned_err", 1}, {"aligned", 1} } },
    { "reg040sRoc0Status_READONLY", { {"NOT_USED", 5}, {"ttc_fifo_full", 1}, {"enc_err", 1}, {"event_full", 1} } },
    { "reg041sRoc1Status_READONLY", { {"NOT_USED", 5}, {"ttc_fifo_full", 1}, {"enc_err", 1}, {"event_full", 1} } },
    { "reg042sRoc2Status_READONLY", { {"NOT_USED", 5}, {"ttc_fifo_full", 1}, {"enc_err", 1}, {"event_full", 1} } },
    { "reg043sRoc3Status_READONLY", { {"NOT_USED", 5}, {"ttc_fifo_full", 1}, {"enc_err", 1}, {"event_full", 1} } },
    { "reg044seu_READONLY", { {"NOT_USED", 7}, {"seu", 1} } },
    { "reg045parityCounterVmm0_READONLY", { {"parity_counter", 8} } },
    { "reg046parityCounterVmm1_READONLY", { {"parity_counter", 8} } },
    { "reg047parityCounterVmm2_READONLY", { {"parity_counter", 8} } },
    { "reg048parityCounterVmm3_READONLY", { {"parity_counter", 8} } },
    { "reg049parityCounterVmm4_READONLY", { {"parity_counter", 8} } },
    { "reg050parityCounterVmm5_READONLY", { {"parity_counter", 8} } },
    { "reg051parityCounterVmm6_READONLY", { {"parity_counter", 8} } },
    { "reg052parityCounterVmm7_READONLY", { {"parity_counter", 8} } },
    { "reg053seuCounter_READONLY", { {"seu_counter", 8} } },

    { "reg063timeoutStatus", { {"vmm7", 1}, {"vmm6", 1}, {"vmm5", 1}, {"vmm4", 1},
                         {"vmm3", 1}, {"vmm2", 1}, {"vmm1", 1}, {"vmm0", 1} } }
};

static const i2c::AddressRegisterMap TDS_REGISTERS = {
    { "register0", {{"BCID_Offset", 12}, {"BCID_Rollover_Value", 12},
                    {"CKBC_Clock_Phase", 4}, {"Strip_Match_Window", 4}}},

     {"register1", {{"Ck160_1_Phase", 5}, {"Ck160_0_Phase", 5}, {"SER_PLL_I", 4}, {"SER_PLL_R", 2}}},

     {"register2", {{"Chan127", 1}, {"Chan126", 1}, {"Chan125", 1}, {"Chan124", 1},
                 {"Chan123", 1}, {"Chan122", 1}, {"Chan121", 1}, {"Chan120", 1},
                 {"Chan119", 1}, {"Chan118", 1}, {"Chan117", 1}, {"Chan116", 1},
                 {"Chan115", 1}, {"Chan114", 1}, {"Chan113", 1}, {"Chan112", 1},
                 {"Chan111", 1}, {"Chan110", 1}, {"Chan109", 1}, {"Chan108", 1},
                 {"Chan107", 1}, {"Chan106", 1}, {"Chan105", 1}, {"Chan104", 1},
                 {"Chan103", 1}, {"Chan102", 1}, {"Chan101", 1}, {"Chan100", 1},
                 {"Chan099", 1}, {"Chan098", 1}, {"Chan097", 1}, {"Chan096", 1},
                 {"Chan095", 1}, {"Chan094", 1}, {"Chan093", 1}, {"Chan092", 1},
                 {"Chan091", 1}, {"Chan090", 1}, {"Chan089", 1}, {"Chan088", 1},
                 {"Chan087", 1}, {"Chan086", 1}, {"Chan085", 1}, {"Chan084", 1},
                 {"Chan083", 1}, {"Chan082", 1}, {"Chan081", 1}, {"Chan080", 1},
                 {"Chan079", 1}, {"Chan078", 1}, {"Chan077", 1}, {"Chan076", 1},
                 {"Chan075", 1}, {"Chan074", 1}, {"Chan073", 1}, {"Chan072", 1},
                 {"Chan071", 1}, {"Chan070", 1}, {"Chan069", 1}, {"Chan068", 1},
                 {"Chan067", 1}, {"Chan066", 1}, {"Chan065", 1}, {"Chan064", 1},
                 {"Chan063", 1}, {"Chan062", 1}, {"Chan061", 1}, {"Chan060", 1},
                 {"Chan059", 1}, {"Chan058", 1}, {"Chan057", 1}, {"Chan056", 1},
                 {"Chan055", 1}, {"Chan054", 1}, {"Chan053", 1}, {"Chan052", 1},
                 {"Chan051", 1}, {"Chan050", 1}, {"Chan049", 1}, {"Chan048", 1},
                 {"Chan047", 1}, {"Chan046", 1}, {"Chan045", 1}, {"Chan044", 1},
                 {"Chan043", 1}, {"Chan042", 1}, {"Chan041", 1}, {"Chan040", 1},
                 {"Chan039", 1}, {"Chan038", 1}, {"Chan037", 1}, {"Chan036", 1},
                 {"Chan035", 1}, {"Chan034", 1}, {"Chan033", 1}, {"Chan032", 1},
                 {"Chan031", 1}, {"Chan030", 1}, {"Chan029", 1}, {"Chan028", 1},
                 {"Chan027", 1}, {"Chan026", 1}, {"Chan025", 1}, {"Chan024", 1},
                 {"Chan023", 1}, {"Chan022", 1}, {"Chan021", 1}, {"Chan020", 1},
                 {"Chan019", 1}, {"Chan018", 1}, {"Chan017", 1}, {"Chan016", 1},
                 {"Chan015", 1}, {"Chan014", 1}, {"Chan013", 1}, {"Chan012", 1},
                 {"Chan011", 1}, {"Chan010", 1}, {"Chan009", 1}, {"Chan008", 1},
                 {"Chan007", 1}, {"Chan006", 1}, {"Chan005", 1}, {"Chan004", 1},
                 {"Chan003", 1}, {"Chan002", 1}, {"Chan001", 1}, {"Chan000", 1}}},

     {"register3", { {"NOT_USED", 1}, {"trig_lut7", 15}, {"NOT_USED", 1}, {"trig_lut6", 15},
                     {"NOT_USED", 1}, {"trig_lut5", 15}, {"NOT_USED", 1}, {"trig_lut4", 15},
                     {"NOT_USED", 1}, {"trig_lut3", 15}, {"NOT_USED", 1}, {"trig_lut2", 15},
                     {"NOT_USED", 1}, {"trig_lut1", 15}, {"NOT_USED", 1}, {"trig_lut0", 15}}},

     {"register4", { {"NOT_USED", 1}, {"trig_lutf", 15}, {"NOT_USED", 1}, {"trig_lute", 15},
                     {"NOT_USED", 1}, {"trig_lutd", 15}, {"NOT_USED", 1}, {"trig_lutc", 15},
                     {"NOT_USED", 1}, {"trig_lutb", 15}, {"NOT_USED", 1}, {"trig_luta", 15},
                     {"NOT_USED", 1}, {"trig_lut9", 15}, {"NOT_USED", 1}, {"trig_lut8", 15}}},

     {"register5", {{"NOT_USED", 3}, {"Chan000", 5}, {"NOT_USED", 3}, {"Chan001", 5},
                    {"NOT_USED", 3}, {"Chan002", 5}, {"NOT_USED", 3}, {"Chan003", 5},
                    {"NOT_USED", 3}, {"Chan004", 5}, {"NOT_USED", 3}, {"Chan005", 5},
                    {"NOT_USED", 3}, {"Chan006", 5}, {"NOT_USED", 3}, {"Chan007", 5},
                    {"NOT_USED", 3}, {"Chan008", 5}, {"NOT_USED", 3}, {"Chan009", 5},
                    {"NOT_USED", 3}, {"Chan010", 5}, {"NOT_USED", 3}, {"Chan011", 5},
                    {"NOT_USED", 3}, {"Chan012", 5}, {"NOT_USED", 3}, {"Chan013", 5},
                    {"NOT_USED", 3}, {"Chan014", 5}, {"NOT_USED", 3}, {"Chan015", 5}}},

     {"register6", {{"NOT_USED", 3}, {"Chan016", 5}, {"NOT_USED", 3}, {"Chan017", 5},
                    {"NOT_USED", 3}, {"Chan018", 5}, {"NOT_USED", 3}, {"Chan019", 5},
                    {"NOT_USED", 3}, {"Chan020", 5}, {"NOT_USED", 3}, {"Chan021", 5},
                    {"NOT_USED", 3}, {"Chan022", 5}, {"NOT_USED", 3}, {"Chan023", 5},
                    {"NOT_USED", 3}, {"Chan024", 5}, {"NOT_USED", 3}, {"Chan025", 5},
                    {"NOT_USED", 3}, {"Chan026", 5}, {"NOT_USED", 3}, {"Chan027", 5},
                    {"NOT_USED", 3}, {"Chan028", 5}, {"NOT_USED", 3}, {"Chan029", 5},
                    {"NOT_USED", 3}, {"Chan030", 5}, {"NOT_USED", 3}, {"Chan031", 5}}},

     {"register7", {{"NOT_USED", 3}, {"Chan032", 5}, {"NOT_USED", 3}, {"Chan033", 5},
                    {"NOT_USED", 3}, {"Chan034", 5}, {"NOT_USED", 3}, {"Chan035", 5},
                    {"NOT_USED", 3}, {"Chan036", 5}, {"NOT_USED", 3}, {"Chan037", 5},
                    {"NOT_USED", 3}, {"Chan038", 5}, {"NOT_USED", 3}, {"Chan039", 5},
                    {"NOT_USED", 3}, {"Chan040", 5}, {"NOT_USED", 3}, {"Chan041", 5},
                    {"NOT_USED", 3}, {"Chan042", 5}, {"NOT_USED", 3}, {"Chan043", 5},
                    {"NOT_USED", 3}, {"Chan044", 5}, {"NOT_USED", 3}, {"Chan045", 5},
                    {"NOT_USED", 3}, {"Chan046", 5}, {"NOT_USED", 3}, {"Chan047", 5}}},

     {"register8", {{"NOT_USED", 3}, {"Chan048", 5}, {"NOT_USED", 3}, {"Chan049", 5},
                    {"NOT_USED", 3}, {"Chan050", 5}, {"NOT_USED", 3}, {"Chan051", 5},
                    {"NOT_USED", 3}, {"Chan052", 5}, {"NOT_USED", 3}, {"Chan053", 5},
                    {"NOT_USED", 3}, {"Chan054", 5}, {"NOT_USED", 3}, {"Chan055", 5},
                    {"NOT_USED", 3}, {"Chan056", 5}, {"NOT_USED", 3}, {"Chan057", 5},
                    {"NOT_USED", 3}, {"Chan058", 5}, {"NOT_USED", 3}, {"Chan059", 5},
                    {"NOT_USED", 3}, {"Chan060", 5}, {"NOT_USED", 3}, {"Chan061", 5},
                    {"NOT_USED", 3}, {"Chan062", 5}, {"NOT_USED", 3}, {"Chan063", 5}}},

     {"register9", {{"NOT_USED", 3}, {"Chan064", 5}, {"NOT_USED", 3}, {"Chan065", 5},
                    {"NOT_USED", 3}, {"Chan066", 5}, {"NOT_USED", 3}, {"Chan067", 5},
                    {"NOT_USED", 3}, {"Chan068", 5}, {"NOT_USED", 3}, {"Chan069", 5},
                    {"NOT_USED", 3}, {"Chan070", 5}, {"NOT_USED", 3}, {"Chan071", 5},
                    {"NOT_USED", 3}, {"Chan072", 5}, {"NOT_USED", 3}, {"Chan073", 5},
                    {"NOT_USED", 3}, {"Chan074", 5}, {"NOT_USED", 3}, {"Chan075", 5},
                    {"NOT_USED", 3}, {"Chan076", 5}, {"NOT_USED", 3}, {"Chan077", 5},
                    {"NOT_USED", 3}, {"Chan078", 5}, {"NOT_USED", 3}, {"Chan079", 5}}},

     {"register10", {{"NOT_USED", 3}, {"Chan080", 5}, {"NOT_USED", 3}, {"Chan081", 5},
                     {"NOT_USED", 3}, {"Chan082", 5}, {"NOT_USED", 3}, {"Chan083", 5},
                     {"NOT_USED", 3}, {"Chan084", 5}, {"NOT_USED", 3}, {"Chan085", 5},
                     {"NOT_USED", 3}, {"Chan086", 5}, {"NOT_USED", 3}, {"Chan087", 5},
                     {"NOT_USED", 3}, {"Chan088", 5}, {"NOT_USED", 3}, {"Chan089", 5},
                     {"NOT_USED", 3}, {"Chan090", 5}, {"NOT_USED", 3}, {"Chan091", 5},
                     {"NOT_USED", 3}, {"Chan092", 5}, {"NOT_USED", 3}, {"Chan093", 5},
                     {"NOT_USED", 3}, {"Chan094", 5}, {"NOT_USED", 3}, {"Chan095", 5}}},

     {"register11", {{"NOT_USED", 3}, {"Chan096", 5}, {"NOT_USED", 3}, {"Chan097", 5},
                     {"NOT_USED", 3}, {"Chan098", 5}, {"NOT_USED", 3}, {"Chan099", 5},
                     {"NOT_USED", 3}, {"Chan100", 5}, {"NOT_USED", 3}, {"Chan101", 5},
                     {"NOT_USED", 3}, {"Chan102", 5}, {"NOT_USED", 3}, {"Chan103", 5}}},

     {"register12", {{"timer", 8}, {"bypass", 4}, {"prompt_circuit", 4}, {"NOT_USED", 3},
                     {"bypass_trigger", 1}, {"bypass_scrambler", 1}, {"test_frame2Router_enable", 1},
                     {"stripTDS_globaltest", 1}, {"PRBS_e", 1}, {"resets", 8}}},

     {"register13_READONLY", {
         {"CRC[1295:1215]", 8}, {"CRC[1214:1134]", 8}, {"CRC[1133:1053]", 8}, {"CRC[1052:972]", 8},
         {"CRC[971:891]", 8}, {"CRC[890:810]", 8}, {"CRC[809:729]", 8}, {"CRC[728:648]", 8},
         {"CRC[647:567]", 8}, {"CRC[566:486]", 8}, {"CRC[485:405]", 8}, {"CRC[404:324]", 8},
         {"CRC[323:243]", 8}, {"CRC[242:162]", 8}, {"CRC[161:81]", 8}, {"CRC[80:0]", 8}}},

     {"register14_READONLY", {{"NOT_USED", 1}, {"Monitor_Strip64", 19 }, {"NOT_USED", 1}, {"Monitor_Strip0", 19},
                              {"NOT_USED", 6}, {"SER_lock", 1}, {"Pll_lock", 1}}},

     {"register15_READONLY", {{"NOT_USED", 4}, {"strip_trigger_bcid", 12},
                              {"NOT_USED", 3}, {"strip_trigger_band_phid", 13}}}
};

#endif  // NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
