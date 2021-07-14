#ifndef NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H
#define NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H

#include <map>
#include <vector>
#include <string>


/** \brief Defines translation between register and value based configuration entries
 *  Every value has a list of translation units which define how the value is mapped to
 *  registers.
 */
template<typename RegisterSize>
struct TranslationUnit
{
    std::string m_registerName{""}; ///< <register>.<subregister>

    /** Mask defining which bits of the register belong to this value. Needed for translation
     *  intoa register based map without sub-registers.
     */
    RegisterSize m_maskRegister{static_cast<RegisterSize>(-1)};

    /** Mask defining which part of a value is set by the value of the (sub)-register.
     *  If a value is distributed over multiple registers one has to define which bits
     *  correspond to the value (e.g. reg064ePllVmm0.ePllPhase160MHz_0[4] to the 4th bit).
     */
    RegisterSize m_maskValue{static_cast<unsigned int>(-1)};
};

template<typename RegisterSize>
using TranslationMap = std::map<std::string, std::vector<TranslationUnit<RegisterSize>>>;

using TranslationUnitRoc = TranslationUnit<std::uint32_t>;
using TranslationUnitTds = TranslationUnit<__uint128_t>;

using TranslationMapRoc = std::map<std::string, std::vector<TranslationUnitRoc>>;
using TranslationMapTds = std::map<std::string, std::vector<TranslationUnitTds>>;

static const TranslationMapRoc TRANSLATION_MAP_ROC_ANALOG = {
    {"ePllVmm0.ePllPhase40MHz_0", {
        TranslationUnitRoc{"reg064ePllVmm0.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_1", {
        TranslationUnitRoc{"reg065ePllVmm0.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_2", {
        TranslationUnitRoc{"reg066ePllVmm0.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_3", {
        TranslationUnitRoc{"reg067ePllVmm0.ePllPhase40MHz_3", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_0", {
        TranslationUnitRoc{"reg064ePllVmm0.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg068ePllVmm0.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_1", {
        TranslationUnitRoc{"reg065ePllVmm0.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg068ePllVmm0.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_2", {
        TranslationUnitRoc{"reg066ePllVmm0.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg069ePllVmm0.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_3", {
        TranslationUnitRoc{"reg067ePllVmm0.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg069ePllVmm0.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm0.ePllInstantLock", {
        TranslationUnitRoc{"reg070ePllVmm0.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllVmm0.ePllReset", {
        TranslationUnitRoc{"reg070ePllVmm0.ePllReset", 0b0100'0000},
    }},
    {"ePllVmm0.bypassPLL", {
        TranslationUnitRoc{"reg070ePllVmm0.bypassPLL", 0b0010'0000},
    }},
    {"ePllVmm0.ePllLockEn", {
        TranslationUnitRoc{"reg070ePllVmm0.ePllLockEn", 0b0001'0000},
    }},
    {"ePllVmm0.ePllReferenceFrequency", {
        TranslationUnitRoc{"reg070ePllVmm0.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllVmm0.ePllCap", {
        TranslationUnitRoc{"reg070ePllVmm0.ePllCap", 0b0000'0011},
    }},
    {"ePllVmm0.ePllRes", {
        TranslationUnitRoc{"reg071ePllVmm0.ePllRes", 0b1111'0000},
    }},
    {"ePllVmm0.ePllIcp", {
        TranslationUnitRoc{"reg071ePllVmm0.ePllIcp", 0b0000'1111},
    }},
    {"ePllVmm0.ePllEnablePhase", {
        TranslationUnitRoc{"reg072ePllVmm0.ePllEnablePhase"},
    }},
    {"ePllVmm0.tp_bypass_0", {
        TranslationUnitRoc{"reg073ePllVmm0.tp_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm0.tp_bypass_1", {
        TranslationUnitRoc{"reg073ePllVmm0.tp_bypass_1", 0b1000'0000},
    }},
    {"ePllVmm0.tp_bypass_2", {
        TranslationUnitRoc{"reg074ePllVmm0.tp_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm0.tp_bypass_3", {
        TranslationUnitRoc{"reg074ePllVmm0.tp_bypass_3", 0b1000'0000},
    }},
    {"ePllVmm0.tp_phase_0", {
        TranslationUnitRoc{"reg073ePllVmm0.tp_phase_0", 0b0000'0111},
    }},
    {"ePllVmm0.tp_phase_1", {
        TranslationUnitRoc{"reg073ePllVmm0.tp_phase_1", 0b0111'0000},
    }},
    {"ePllVmm0.tp_phase_2", {
        TranslationUnitRoc{"reg074ePllVmm0.tp_phase_2", 0b0000'0111},
    }},
    {"ePllVmm0.tp_phase_3", {
        TranslationUnitRoc{"reg074ePllVmm0.tp_phase_3", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_05delay_0", {
        TranslationUnitRoc{"reg075ePllVmm0.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_1", {
        TranslationUnitRoc{"reg076ePllVmm0.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_2", {
        TranslationUnitRoc{"reg077ePllVmm0.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_3", {
        TranslationUnitRoc{"reg078ePllVmm0.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_delay_0", {
        TranslationUnitRoc{"reg075ePllVmm0.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_1", {
        TranslationUnitRoc{"reg076ePllVmm0.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_2", {
        TranslationUnitRoc{"reg077ePllVmm0.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_3", {
        TranslationUnitRoc{"reg078ePllVmm0.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_bypass_0", {
        TranslationUnitRoc{"reg075ePllVmm0.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_1", {
        TranslationUnitRoc{"reg076ePllVmm0.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_2", {
        TranslationUnitRoc{"reg077ePllVmm0.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_3", {
        TranslationUnitRoc{"reg078ePllVmm0.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_phase_0", {
        TranslationUnitRoc{"reg075ePllVmm0.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_1", {
        TranslationUnitRoc{"reg076ePllVmm0.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_2", {
        TranslationUnitRoc{"reg077ePllVmm0.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_3", {
        TranslationUnitRoc{"reg078ePllVmm0.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllVmm0.tx_enable", {
        TranslationUnitRoc{"reg079ePllVmm0.tx_enable", 0b1111'0000},
    }},
    {"ePllVmm0.tx_csel", {
        TranslationUnitRoc{"reg079ePllVmm0.tx_csel", 0b0000'1111},
    }},
    {"ePllVmm1.ePllPhase40MHz_0", {
        TranslationUnitRoc{"reg080ePllVmm1.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_1", {
        TranslationUnitRoc{"reg081ePllVmm1.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_2", {
        TranslationUnitRoc{"reg082ePllVmm1.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_3", {
        TranslationUnitRoc{"reg083ePllVmm1.ePllPhase40MHz_3", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_0", {
        TranslationUnitRoc{"reg080ePllVmm1.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg084ePllVmm1.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_1", {
        TranslationUnitRoc{"reg081ePllVmm1.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg084ePllVmm1.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_2", {
        TranslationUnitRoc{"reg082ePllVmm1.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg085ePllVmm1.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_3", {
        TranslationUnitRoc{"reg083ePllVmm1.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg085ePllVmm1.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm1.ePllInstantLock", {
        TranslationUnitRoc{"reg086ePllVmm1.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllVmm1.ePllReset", {
        TranslationUnitRoc{"reg086ePllVmm1.ePllReset", 0b0100'0000},
    }},
    {"ePllVmm1.bypassPLL", {
        TranslationUnitRoc{"reg086ePllVmm1.bypassPLL", 0b0010'0000},
    }},
    {"ePllVmm1.ePllLockEn", {
        TranslationUnitRoc{"reg086ePllVmm1.ePllLockEn", 0b0001'0000},
    }},
    {"ePllVmm1.ePllReferenceFrequency", {
        TranslationUnitRoc{"reg086ePllVmm1.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllVmm1.ePllCap", {
        TranslationUnitRoc{"reg086ePllVmm1.ePllCap", 0b0000'0011},
    }},
    {"ePllVmm1.ePllRes", {
        TranslationUnitRoc{"reg087ePllVmm1.ePllRes", 0b1111'0000},
    }},
    {"ePllVmm1.ePllIcp", {
        TranslationUnitRoc{"reg087ePllVmm1.ePllIcp", 0b0000'1111},
    }},
    {"ePllVmm1.ePllEnablePhase", {
        TranslationUnitRoc{"reg088ePllVmm1.ePllEnablePhase"},
    }},
    {"ePllVmm1.tp_bypass_0", {
        TranslationUnitRoc{"reg089ePllVmm1.tp_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm1.tp_bypass_1", {
        TranslationUnitRoc{"reg089ePllVmm1.tp_bypass_1", 0b1000'0000},
    }},
    {"ePllVmm1.tp_bypass_2", {
        TranslationUnitRoc{"reg090ePllVmm1.tp_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm1.tp_bypass_3", {
        TranslationUnitRoc{"reg090ePllVmm1.tp_bypass_3", 0b1000'0000},
    }},
    {"ePllVmm1.tp_phase_0", {
        TranslationUnitRoc{"reg089ePllVmm1.tp_phase_0", 0b0000'0111},
    }},
    {"ePllVmm1.tp_phase_1", {
        TranslationUnitRoc{"reg089ePllVmm1.tp_phase_1", 0b0111'0000},
    }},
    {"ePllVmm1.tp_phase_2", {
        TranslationUnitRoc{"reg090ePllVmm1.tp_phase_2", 0b0000'0111},
    }},
    {"ePllVmm1.tp_phase_3", {
        TranslationUnitRoc{"reg090ePllVmm1.tp_phase_3", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_05delay_0", {
        TranslationUnitRoc{"reg091ePllVmm1.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_1", {
        TranslationUnitRoc{"reg092ePllVmm1.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_2", {
        TranslationUnitRoc{"reg093ePllVmm1.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_3", {
        TranslationUnitRoc{"reg094ePllVmm1.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_delay_0", {
        TranslationUnitRoc{"reg091ePllVmm1.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_1", {
        TranslationUnitRoc{"reg092ePllVmm1.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_2", {
        TranslationUnitRoc{"reg093ePllVmm1.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_3", {
        TranslationUnitRoc{"reg094ePllVmm1.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_bypass_0", {
        TranslationUnitRoc{"reg091ePllVmm1.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_1", {
        TranslationUnitRoc{"reg092ePllVmm1.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_2", {
        TranslationUnitRoc{"reg093ePllVmm1.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_3", {
        TranslationUnitRoc{"reg094ePllVmm1.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_phase_0", {
        TranslationUnitRoc{"reg091ePllVmm1.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_1", {
        TranslationUnitRoc{"reg092ePllVmm1.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_2", {
        TranslationUnitRoc{"reg093ePllVmm1.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_3", {
        TranslationUnitRoc{"reg094ePllVmm1.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllVmm1.tx_enable", {
        TranslationUnitRoc{"reg095ePllVmm1.tx_enable", 0b1111'0000},
    }},
    {"ePllVmm1.tx_csel", {
        TranslationUnitRoc{"reg095ePllVmm1.tx_csel", 0b0000'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_0", {
        TranslationUnitRoc{"reg096ePllTdc.ePllPhase40MHz_0", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_1", {
        TranslationUnitRoc{"reg097ePllTdc.ePllPhase40MHz_1", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_2", {
        TranslationUnitRoc{"reg098ePllTdc.ePllPhase40MHz_2", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_3", {
        TranslationUnitRoc{"reg099ePllTdc.ePllPhase40MHz_3", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase160MHz_0", {
        TranslationUnitRoc{"reg096ePllTdc.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg100ePllTdc.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_1", {
        TranslationUnitRoc{"reg097ePllTdc.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg100ePllTdc.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_2", {
        TranslationUnitRoc{"reg098ePllTdc.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg101ePllTdc.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_3", {
        TranslationUnitRoc{"reg099ePllTdc.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg101ePllTdc.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllTdc.ePllInstantLock", {
        TranslationUnitRoc{"reg102ePllTdc.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllTdc.ePllReset", {
        TranslationUnitRoc{"reg102ePllTdc.ePllReset", 0b0100'0000},
    }},
    {"ePllTdc.bypassPLL", {
        TranslationUnitRoc{"reg102ePllTdc.bypassPLL", 0b0010'0000},
    }},
    {"ePllTdc.ePllLockEn", {
        TranslationUnitRoc{"reg102ePllTdc.ePllLockEn", 0b0001'0000},
    }},
    {"ePllTdc.ePllReferenceFrequency", {
        TranslationUnitRoc{"reg102ePllTdc.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllTdc.ePllCap", {
        TranslationUnitRoc{"reg102ePllTdc.ePllCap", 0b0000'0011},
    }},
    {"ePllTdc.ePllRes", {
        TranslationUnitRoc{"reg103ePllTdc.ePllRes", 0b1111'0000},
    }},
    {"ePllTdc.ePllIcp", {
        TranslationUnitRoc{"reg103ePllTdc.ePllIcp", 0b0000'1111},
    }},
    {"ePllTdc.ePllEnablePhase", {
        TranslationUnitRoc{"reg104ePllTdc.ePllEnablePhase"},
    }},
    {"ePllTdc.enable160MHzOnBCR", {
        TranslationUnitRoc{"reg105ePllTdc.enable160MHzOnBCR", 0b1111'0000},
    }},
    {"ePllTdc.enable160MHzOn40MHz", {
        TranslationUnitRoc{"reg105ePllTdc.enable160MHzOn40MHz", 0b0000'1111},
    }},
    {"ePllTdc.tx_enable_bcr", {
        TranslationUnitRoc{"reg106ePllTdc.tx_enable_bcr", 0b1111'0000},
    }},
    {"ePllTdc.tx_csel_bcr", {
        TranslationUnitRoc{"reg106ePllTdc.tx_csel_bcr", 0b0000'1111},
    }},
    {"ePllTdc.ctrl_05delay_0", {
        TranslationUnitRoc{"reg107ePllTdc.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_1", {
        TranslationUnitRoc{"reg108ePllTdc.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_2", {
        TranslationUnitRoc{"reg109ePllTdc.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_3", {
        TranslationUnitRoc{"reg110ePllTdc.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_delay_0", {
        TranslationUnitRoc{"reg107ePllTdc.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_1", {
        TranslationUnitRoc{"reg108ePllTdc.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_2", {
        TranslationUnitRoc{"reg109ePllTdc.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_3", {
        TranslationUnitRoc{"reg110ePllTdc.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_bypass_0", {
        TranslationUnitRoc{"reg107ePllTdc.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_1", {
        TranslationUnitRoc{"reg108ePllTdc.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_2", {
        TranslationUnitRoc{"reg109ePllTdc.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_3", {
        TranslationUnitRoc{"reg110ePllTdc.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_phase_0", {
        TranslationUnitRoc{"reg107ePllTdc.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_1", {
        TranslationUnitRoc{"reg108ePllTdc.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_2", {
        TranslationUnitRoc{"reg109ePllTdc.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_3", {
        TranslationUnitRoc{"reg110ePllTdc.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllTdc.tx_enable", {
        TranslationUnitRoc{"reg111ePllTdc.tx_enable", 0b1111'0000},
    }},
    {"ePllTdc.tx_csel", {
        TranslationUnitRoc{"reg111ePllTdc.tx_csel", 0b0000'1111},
    }},
    {"FIXME.ePllInstantLock", {
        TranslationUnitRoc{"reg112.ePllInstantLock", 0b1000'0000},
    }},
    {"FIXME.ePllReset", {
        TranslationUnitRoc{"reg112.ePllReset", 0b0100'0000},
    }},
    {"FIXME.bypassPLL", {
        TranslationUnitRoc{"reg112.bypassPLL", 0b0010'0000},
    }},
    {"FIXME.ePllLockEn", {
        TranslationUnitRoc{"reg112.ePllLockEn", 0b0001'0000},
    }},
    {"FIXME.ePllReferenceFrequency", {
        TranslationUnitRoc{"reg112.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"FIXME.ePllCap", {
        TranslationUnitRoc{"reg112.ePllCap", 0b0000'0011},
    }},
    {"FIXME.ePllRes", {
        TranslationUnitRoc{"reg113.ePllRes", 0b1111'0000},
    }},
    {"FIXME.ePllIcp", {
        TranslationUnitRoc{"reg113.ePllIcp", 0b0000'1111},
    }},
    {"FIXME.ePllEnablePhase", {
        TranslationUnitRoc{"reg114.ePllEnablePhase"},
    }},
    {"FIXME.ePllPhase40MHz_0", {
        TranslationUnitRoc{"reg115.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"FIXME.ePllPhase40MHz_1", {
        TranslationUnitRoc{"reg116.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"FIXME.ePllPhase40MHz_2", {
        TranslationUnitRoc{"reg117.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"FIXME.ePllPhase160MHz_0", {
        TranslationUnitRoc{"reg115.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg118.ePllPhase160MHz_0[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"FIXME.ePllPhase160MHz_1", {
        TranslationUnitRoc{"reg116.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg118.ePllPhase160MHz_1[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"FIXME.ePllPhase160MHz_2", {
        TranslationUnitRoc{"reg117.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg119.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"FIXME.tp_bypass_global", {
        TranslationUnitRoc{"reg119.tp_bypass_global", 0b1000'0000}
    }},
    {"FIXME.tp_phase_global", {
        TranslationUnitRoc{"reg119.tp_phase_global", 0b0111'0000}
    }},
    {"FIXME.TDS_BCR_INV", {
        TranslationUnitRoc{"reg120.TDS_BCR_INV", 0b1111'0000}
    }},
    {"FIXME.LockOutInv", {
        TranslationUnitRoc{"reg120.LockOutInv", 0b0000'1000}
    }},
    {"FIXME.testOutEn", {
        TranslationUnitRoc{"reg120.testOutEn", 0b0000'0100}
    }},
    {"FIXME.testOutMux", {
        TranslationUnitRoc{"reg120.testOutMux", 0b0000'0011}
    }},
    {"FIXME.vmmBcrInv", {
        TranslationUnitRoc{"reg121vmmBcrInv.vmmBcrInv"}
    }},
    {"FIXME.vmmEnaInv", {
        TranslationUnitRoc{"reg122vmmEnaInv.vmmEnaInv"}
    }},
    {"FIXME.vmmL0Inv", {
        TranslationUnitRoc{"reg123vmmL0Inv.vmmL0Inv"}
    }},
    {"FIXME.vmmTpInv", {
        TranslationUnitRoc{"reg124vmmTpInv.vmmTpInv"}
    }}
};

static const TranslationMapRoc TRANSLATION_MAP_ROC_DIGITAL = {
    {"rocId.l1_first", {
        TranslationUnitRoc{"reg000rocId.l1_first", 0b1000'0000}
    }},
    {"rocId.even_parity", {
        TranslationUnitRoc{"reg000rocId.even_parity", 0b0100'0000}
    }},
    {"rocId.roc_id", {
        TranslationUnitRoc{"reg000rocId.roc_id", 0b0011'1111}
    }},
    {"elinkSpeed.sRoc0", {
        TranslationUnitRoc{"reg001elinkSpeed.sroc0", 0b0000'0011}
    }},
    {"elinkSpeed.sRoc1", {
        TranslationUnitRoc{"reg001elinkSpeed.sroc1", 0b0000'1100}
    }},
    {"elinkSpeed.sRoc2", {
        TranslationUnitRoc{"reg001elinkSpeed.sroc2", 0b0011'0000}
    }},
    {"elinkSpeed.sRoc3", {
        TranslationUnitRoc{"reg001elinkSpeed.sroc3", 0b1100'0000}
    }},
    {"sRoc0VmmConnections.vmm0", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc0VmmConnections.vmm1", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc0VmmConnections.vmm2", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc0VmmConnections.vmm3", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc0VmmConnections.vmm4", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc0VmmConnections.vmm5", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc0VmmConnections.vmm6", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc0VmmConnections.vmm7", {
        TranslationUnitRoc{"reg002sRoc0VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc1VmmConnections.vmm0", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc1VmmConnections.vmm1", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc1VmmConnections.vmm2", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc1VmmConnections.vmm3", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc1VmmConnections.vmm4", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc1VmmConnections.vmm5", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc1VmmConnections.vmm6", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc1VmmConnections.vmm7", {
        TranslationUnitRoc{"reg003sRoc1VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc2VmmConnections.vmm0", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc2VmmConnections.vmm1", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc2VmmConnections.vmm2", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc2VmmConnections.vmm3", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc2VmmConnections.vmm4", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc2VmmConnections.vmm5", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc2VmmConnections.vmm6", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc2VmmConnections.vmm7", {
        TranslationUnitRoc{"reg004sRoc2VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc3VmmConnections.vmm0", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc3VmmConnections.vmm1", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc3VmmConnections.vmm2", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc3VmmConnections.vmm3", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc3VmmConnections.vmm4", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc3VmmConnections.vmm5", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc3VmmConnections.vmm6", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc3VmmConnections.vmm7", {
        TranslationUnitRoc{"reg005sRoc3VmmConnections.vmm7", 0b1000'0000}
    }},
    {"eopEnable.sRoc0", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc0_eop_enable", 0b0001'0000}
    }},
    {"eopEnable.sRoc1", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc1_eop_enable", 0b0010'0000}
    }},
    {"eopEnable.sRoc2", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc2_eop_enable", 0b0100'0000}
    }},
    {"eopEnable.sRoc3", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc3_eop_enable", 0b1000'0000}
    }},
    {"nullEventEnable.sRoc0", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc0_nullevt_enable", 0b0001'0000}
    }},
    {"nullEventEnable.sRoc1", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc1_nullevt_enable", 0b0010'0000}
    }},
    {"nullEventEnable.sRoc2", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc2_nullevt_enable", 0b0100'0000}
    }},
    {"nullEventEnable.sRoc3", {
        TranslationUnitRoc{"reg006eopAndNullEventEnable.sroc3_nullevt_enable", 0b1000'0000}
    }},
    {"sRocEnable.bypass", {
        TranslationUnitRoc{"reg007sRocEnable.bypass", 0b1000'0000}
    }},
    {"sRocEnable.timeoutEnable", {
        TranslationUnitRoc{"reg007sRocEnable.timeoutEnable", 0b0100'0000}
    }},
    {"sRocEnable.TTCStartBits", {
        TranslationUnitRoc{"reg007sRocEnable.TTCStartBits", 0b0011'0000}
    }},
    {"sRocEnable.sRoc0", {
        TranslationUnitRoc{"reg007sRocEnable.enableSROC0", 0b0000'0001}
    }},
    {"sRocEnable.sRoc1", {
        TranslationUnitRoc{"reg007sRocEnable.enableSROC1", 0b0000'0010}
    }},
    {"sRocEnable.sRoc2", {
        TranslationUnitRoc{"reg007sRocEnable.enableSROC2", 0b0000'0100}
    }},
    {"sRocEnable.sRoc3", {
        TranslationUnitRoc{"reg007sRocEnable.enableSROC3", 0b0000'1000}
    }},
    {"vmmEnable.vmm0", {
        TranslationUnitRoc{"reg008vmmEnable.vmm0", 0b0000'0001}
    }},
    {"vmmEnable.vmm1", {
        TranslationUnitRoc{"reg008vmmEnable.vmm1", 0b0000'0010}
    }},
    {"vmmEnable.vmm2", {
        TranslationUnitRoc{"reg008vmmEnable.vmm2", 0b0000'0100}
    }},
    {"vmmEnable.vmm3", {
        TranslationUnitRoc{"reg008vmmEnable.vmm3", 0b0000'1000}
    }},
    {"vmmEnable.vmm4", {
        TranslationUnitRoc{"reg008vmmEnable.vmm4", 0b0001'0000}
    }},
    {"vmmEnable.vmm5", {
        TranslationUnitRoc{"reg008vmmEnable.vmm5", 0b0010'0000}
    }},
    {"vmmEnable.vmm6", {
        TranslationUnitRoc{"reg008vmmEnable.vmm6", 0b0100'0000}
    }},
    {"vmmEnable.vmm7", {
        TranslationUnitRoc{"reg008vmmEnable.vmm7", 0b1000'0000}
    }},
    {"FIXME.timeout", {
        TranslationUnitRoc{"reg009timeout.timeout"}
    }},
    {"FIXME.tx_csel", {
        TranslationUnitRoc{"reg010bcOffset0_txcSel.tx_csel", 0b1111'0000}
    }},
    {"FIXME.bc_offset", {
        TranslationUnitRoc{"reg010bcOffset0_txcSel.bc_offset[11:8]", 0b0000'1111},
        TranslationUnitRoc{"reg011bcOffset1.bc_offset[7:0]"}
    }},
    {"FIXME.bc_rollover", {
        TranslationUnitRoc{"reg012bcRollover0.bc_rollover[11:8]", 0b0000'1111},
        TranslationUnitRoc{"reg013bcRollover1.bc_rollover[7:0]"}
    }},
    {"eportEnable.sRoc0", {
        TranslationUnitRoc{"reg014eportEnable.sroc0", 0b0000'0011}
    }},
    {"eportEnable.sRoc1", {
        TranslationUnitRoc{"reg014eportEnable.sroc1", 0b0000'1100}
    }},
    {"eportEnable.sRoc2", {
        TranslationUnitRoc{"reg014eportEnable.sroc2", 0b0011'0000}
    }},
    {"eportEnable.sRoc3", {
        TranslationUnitRoc{"reg014eportEnable.sroc3", 0b1100'0000}
    }},
    {"timeoutStatus.vmm0", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm0", 0b0000'0001}
    }},
    {"timeoutStatus.vmm1", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm1", 0b0000'0010}
    }},
    {"timeoutStatus.vmm2", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm2", 0b0000'0100}
    }},
    {"timeoutStatus.vmm3", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm3", 0b0000'1000}
    }},
    {"timeoutStatus.vmm4", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm4", 0b0001'0000}
    }},
    {"timeoutStatus.vmm5", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm5", 0b0010'0000}
    }},
    {"timeoutStatus.vmm6", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm6", 0b0100'0000}
    }},
    {"timeoutStatus.vmm7", {
        TranslationUnitRoc{"reg063timeoutStatus.vmm7", 0b1000'0000}
    }},
    {"busyEnable.sRoc0", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.busy_enable_sroc0", 0b0000'0001}
    }},
    {"busyEnable.sRoc1", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.busy_enable_sroc1", 0b0000'0010}
    }},
    {"busyEnable.sRoc2", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.busy_enable_sroc2", 0b0000'0100}
    }},
    {"busyEnable.sRoc3", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.busy_enable_sroc3", 0b0000'1000}
    }},
    {"tdcEnable.sRoc0", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.tdc_enable_sroc0", 0b0001'0000}
    }},
    {"tdcEnable.sRoc1", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.tdc_enable_sroc1", 0b0010'0000}
    }},
    {"tdcEnable.sRoc2", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.tdc_enable_sroc2", 0b0100'0000}
    }},
    {"tdcEnable.sRoc3", {
        TranslationUnitRoc{"reg020busyAndTdcEnable.tdc_enable_sroc3", 0b1000'0000}
    }},
    {"FIXME.busyOnLimit", {
        TranslationUnitRoc{"reg021busyOnLimit0.busy_on_limit[10:8]", 0b0000'0111, 0b0111'0000'0000},
        TranslationUnitRoc{"reg022busyOnLimit1.busy_on_limit[7:0]", 0b1111'1111, 0b0000'1111'1111}
    }},
    {"FIXME.busyOffLimit", {
        TranslationUnitRoc{"reg023busyOffLimit0.busy_off_limit[10:8]", 0b0000'0111},
        TranslationUnitRoc{"reg024busyOffLimit1.busy_off_limit[7:0]"}
    }},
    {"FIXME.l1EventsWithoutComma", {
        TranslationUnitRoc{"reg031l1EventsWithoutComma.l1_events_no_comma"}
    }},
    {"fakeVmmFailure.vmm0", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm0", 0b0000'0001}
    }},
    {"fakeVmmFailure.vmm1", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm1", 0b0000'0010}
    }},
    {"fakeVmmFailure.vmm2", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm2", 0b0000'0100}
    }},
    {"fakeVmmFailure.vmm3", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm3", 0b0000'1000}
    }},
    {"fakeVmmFailure.vmm4", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm4", 0b0001'0000}
    }},
    {"fakeVmmFailure.vmm5", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm5", 0b0010'0000}
    }},
    {"fakeVmmFailure.vmm6", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm6", 0b0100'0000}
    }},
    {"fakeVmmFailure.vmm7", {
        TranslationUnitRoc{"reg019fakeVmmFailure.vmm7", 0b1000'0000}
    }}
};

static const TranslationMapTds TRANSLATION_MAP_TDS = {
    {"Strip_Match_Window", {
        TranslationUnitTds{"register0.Strip_Match_Window", 0b1000'0000}
    }},
    {"CKBC_Clock_Phase", {
        TranslationUnitTds{"register0.CKBC_Clock_Phase", 0b1000'0000}
    }},
    {"BCID_Rollover_Value", {
        TranslationUnitTds{"register0.BCID_Rollover_Value", 0b1000'0000}
    }},
    {"BCID_Offset", {
        TranslationUnitTds{"register0.BCID_Offset", 0b1000'0000}
    }},
    {"Strip_Match_Window", {
        TranslationUnitTds{"register0.Strip_Match_Window", 0b1000'0000}
    }},
};

#endif
