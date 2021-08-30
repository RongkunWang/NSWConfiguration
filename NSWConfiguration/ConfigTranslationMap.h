#ifndef NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H
#define NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H

#include "extern/fmt/include/fmt/core.h"
#include <iomanip>
#include <map>
#include <sstream>
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
using TranslationUnitArt = TranslationUnit<std::uint32_t>;

using TranslationMapRoc = std::map<std::string, std::vector<TranslationUnitRoc>>;
using TranslationMapTds = std::map<std::string, std::vector<TranslationUnitTds>>;
using TranslationMapArt = std::map<std::string, std::vector<TranslationUnitArt>>;

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
    {"ePllCore.ePllInstantLock", {
        TranslationUnitRoc{"reg112.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllCore.ePllReset", {
        TranslationUnitRoc{"reg112.ePllReset", 0b0100'0000},
    }},
    {"ePllCore.bypassPLL", {
        TranslationUnitRoc{"reg112.bypassPLL", 0b0010'0000},
    }},
    {"ePllCore.ePllLockEn", {
        TranslationUnitRoc{"reg112.ePllLockEn", 0b0001'0000},
    }},
    {"ePllCore.ePllReferenceFrequency", {
        TranslationUnitRoc{"reg112.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllCore.ePllCap", {
        TranslationUnitRoc{"reg112.ePllCap", 0b0000'0011},
    }},
    {"ePllCore.ePllRes", {
        TranslationUnitRoc{"reg113.ePllRes", 0b1111'0000},
    }},
    {"ePllCore.ePllIcp", {
        TranslationUnitRoc{"reg113.ePllIcp", 0b0000'1111},
    }},
    {"ePllCore.ePllEnablePhase", {
        TranslationUnitRoc{"reg114.ePllEnablePhase"},
    }},
    {"ePllCore.ePllPhase40MHz_0", {
        TranslationUnitRoc{"reg115.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"ePllCore.ePllPhase40MHz_1", {
        TranslationUnitRoc{"reg116.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"ePllCore.ePllPhase40MHz_2", {
        TranslationUnitRoc{"reg117.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"ePllCore.ePllPhase160MHz_0", {
        TranslationUnitRoc{"reg115.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg118.ePllPhase160MHz_0[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllCore.ePllPhase160MHz_1", {
        TranslationUnitRoc{"reg116.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg118.ePllPhase160MHz_1[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllCore.ePllPhase160MHz_2", {
        TranslationUnitRoc{"reg117.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnitRoc{"reg119.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllCore.tp_bypass_global", {
        TranslationUnitRoc{"reg119.tp_bypass_global", 0b1000'0000}
    }},
    {"ePllCore.tp_phase_global", {
        TranslationUnitRoc{"reg119.tp_phase_global", 0b0111'0000}
    }},
    {"tdsBcrInvert", {
        TranslationUnitRoc{"reg120.TDS_BCR_INV", 0b1111'0000}
    }},
    {"lockOutInvert", {
        TranslationUnitRoc{"reg120.LockOutInv", 0b0000'1000}
    }},
    {"testOutEnable", {
        TranslationUnitRoc{"reg120.testOutEn", 0b0000'0100}
    }},
    {"testOutMux", {
        TranslationUnitRoc{"reg120.testOutMux", 0b0000'0011}
    }},
    {"vmmBcrInvert", {
        TranslationUnitRoc{"reg121vmmBcrInv.vmmBcrInv"}
    }},
    {"vmmEnaInvert", {
        TranslationUnitRoc{"reg122vmmEnaInv.vmmEnaInv"}
    }},
    {"vmmL0Invert", {
        TranslationUnitRoc{"reg123vmmL0Inv.vmmL0Inv"}
    }},
    {"vmmTpInvert", {
        TranslationUnitRoc{"reg124vmmTpInv.vmmTpInv"}
    }}
};

static const TranslationMapRoc TRANSLATION_MAP_ROC_DIGITAL = {
    {"l1_first", {
        TranslationUnitRoc{"reg000rocId.l1_first", 0b1000'0000}
    }},
    {"even_parity", {
        TranslationUnitRoc{"reg000rocId.even_parity", 0b0100'0000}
    }},
    {"roc_id", {
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
    {"bypassMode", {
        TranslationUnitRoc{"reg007sRocEnable.bypass", 0b1000'0000}
    }},
    {"timeoutEnable", {
        TranslationUnitRoc{"reg007sRocEnable.timeoutEnable", 0b0100'0000}
    }},
    {"ttcStartBits", {
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
    {"timeout", {
        TranslationUnitRoc{"reg009timeout.timeout"}
    }},
    {"tx_csel", {
        TranslationUnitRoc{"reg010bcOffset0_txcSel.tx_csel", 0b1111'0000}
    }},
    {"bc_offset", {
        TranslationUnitRoc{"reg010bcOffset0_txcSel.bc_offset[11:8]", 0b0000'1111},
        TranslationUnitRoc{"reg011bcOffset1.bc_offset[7:0]"}
    }},
    {"bc_rollover", {
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
    {"busyOnLimit", {
        TranslationUnitRoc{"reg021busyOnLimit0.busy_on_limit[10:8]", 0b0000'0111, 0b0111'0000'0000},
        TranslationUnitRoc{"reg022busyOnLimit1.busy_on_limit[7:0]", 0b1111'1111, 0b0000'1111'1111}
    }},
    {"busyOffLimit", {
        TranslationUnitRoc{"reg023busyOffLimit0.busy_off_limit[10:8]", 0b0000'0111},
        TranslationUnitRoc{"reg024busyOffLimit1.busy_off_limit[7:0]"}
    }},
    {"l1EventsWithoutComma", {
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

static const TranslationMapTds TRANSLATION_MAP_TDS = [] () {
    // FIXME: Origin of magic numbers: I2cRegisterMappings.h
    TranslationMapTds table = {
        {"Strip_Match_Window", {
            TranslationUnitTds{"register0.Strip_Match_Window", 0x0000'000F}
        }},
        {"CKBC_Clock_Phase", {
            TranslationUnitTds{"register0.CKBC_Clock_Phase", 0x0000'00F0}
        }},
        {"BCID_Rollover_Value", {
            TranslationUnitTds{"register0.BCID_Rollover_Value", 0x000F'FF00}
        }},
        {"BCID_Offset", {
            TranslationUnitTds{"register0.BCID_Offset", 0xFFF0'0000}
        }},
        {"SER_PLL_R", {
            TranslationUnitTds{"register1.SER_PLL_R", 0xF800}
        }},
        {"SER_PLL_I", {
            TranslationUnitTds{"register1.SER_PLL_I", 0x07C0}
        }},
        {"Ck160_0_Phase", {
            TranslationUnitTds{"register1.Ck160_0_Phase", 0x003C}
        }},
        {"Ck160_1_Phase", {
            TranslationUnitTds{"register1.Ck160_1_Phase", 0x0003}
        }},
        {"resets", {
            TranslationUnitTds{"register12.resets", 0x0000'00FF}
        }},
        {"PRBS_en", {
            TranslationUnitTds{"register12.PRBS_e", 0x0000'0100}
        }},
        {"stripTDS_globaltest", {
            TranslationUnitTds{"register12.stripTDS_globaltest", 0x0000'0200}
        }},
        {"test_frame2Router_enable", {
            TranslationUnitTds{"register12.test_frame2Router_enable", 0x0000'0400}
        }},
        {"bypass_scrambler", {
            TranslationUnitTds{"register12.bypass_scrambler", 0x0000'0800}
        }},
        {"bypass_trigger", {
            TranslationUnitTds{"register12.bypass_trigger", 0x0000'1000}
        }},
        {"prompt_circuit", {
            TranslationUnitTds{"register12.prompt_circuit", 0x000F'0000}
        }},
        {"bypass", {
            TranslationUnitTds{"register12.bypass", 0x00F0'0000}
        }},
        {"timer", {
            TranslationUnitTds{"register12.timer", 0xFF00'0000}
        }}
    };
    for (unsigned int i = 0; i < 128; i++) {
        std::stringstream ss;
        ss << "Chan" << std::setw(3) << i;
        table[ss.str() + "_Disable"] = std::vector{TranslationUnitTds{"register2." + ss.str(), __uint128_t{0x1} << i}};
    }
    const auto reg3and4 = [&table] (const std::string& regname, const auto start) {
        for (unsigned int i = 0; i < 8; i++) {
            std::stringstream ss;
            ss << "trig_lut" << std::hex << i + start;
            table[ss.str()] = std::vector{TranslationUnitTds{regname + '.' + ss.str(), __uint128_t{0x7FFF} << i * 16}};
        }
    };
    reg3and4("register3", 0);
    reg3and4("register4", 8);
    const auto reg5to10 = [&table] (const std::string& regname, const auto start) {
        for (unsigned int i = 0; i < 16; i++) {
            std::stringstream ss;
            ss << "Pad_Chan" << std::hex << i + start;
            table[ss.str() + "_Delay"] = std::vector{TranslationUnitTds{regname + '.' + ss.str(), __uint128_t{0x1F} << i * 8}};
        }
    };
    reg5to10("register5", 0);
    reg5to10("register6", 16);
    reg5to10("register7", 32);
    reg5to10("register8", 48);
    reg5to10("register9", 64);
    reg5to10("register10", 80);
    for (unsigned int i = 0; i < 8; i++) {
        std::stringstream ss;
        ss << "Pad_Chan" << std::hex << i + 96;
        table[ss.str() + "_Delay"] = std::vector{TranslationUnitTds{"register11" + ss.str(), __uint128_t{0x1F} << i * 8}};
    }
    return table;
}();

static const auto TRANSLATION_MAP_ART_PS = [] () -> TranslationMapArt {
    const auto get = [](const int number) -> TranslationMapArt {
        const auto nRegsPerBlock = 15;
        const auto nFirstRxOff = 62;
        const auto nFirstRxTermEnable = 66;
        return {
            {fmt::format("{:02}.dllLockedV", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllLockedV", nRegsPerBlock * number + 0), 0b1000'0000}
            }},
            {fmt::format("{:02}.reserved", number), {
                TranslationUnitRoc{fmt::format("{:02}.reserved", nRegsPerBlock * number + 0), 0b0100'0000}
            }},
            {fmt::format("{:02}.dllLockCfg", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllLockCfg", nRegsPerBlock * number + 0), 0b0011'0000}
            }},
            {fmt::format("{:02}.muxEn2to8", number), {
                TranslationUnitRoc{fmt::format("{:02}.muxEn2to8", nRegsPerBlock * number + 0), 0b0000'1000}
            }},
            {fmt::format("{:02}.muzEn1to8", number), {
                TranslationUnitRoc{fmt::format("{:02}.muzEn1to8", nRegsPerBlock * number + 0), 0b0000'0100}
            }},
            {fmt::format("{:02}.dllCoarseLockDetection", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllCoarseLockDetection", nRegsPerBlock * number + 0), 0b0000'0010}
            }},
            {fmt::format("{:02}.dllResetFromCfg", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllResetFromCfg", nRegsPerBlock * number + 0), 0b0000'0001}
            }},
            {fmt::format("{:02}.dataRateDll", number), {
                TranslationUnitRoc{fmt::format("{:02}.dataRateDll", nRegsPerBlock * number + 1), 0b1100'0000}
            }},
            {fmt::format("{:02}.dllConfirmCountSelect", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllConfirmCountSelect", nRegsPerBlock * number + 1), 0b0011'0000}
            }},
            {fmt::format("{:02}.dllChargePumpCurrent", number), {
                TranslationUnitRoc{fmt::format("{:02}.dllChargePumpCurrent", nRegsPerBlock * number + 1), 0b0000'1111}
            }},
            {fmt::format("{:02}.enableGroup", number), {
                TranslationUnitRoc{fmt::format("{:02}.enableGroup", nRegsPerBlock * number + 2), 0b1000'0000}
            }},
            {fmt::format("{:02}.outRegEn", number), {
                TranslationUnitRoc{fmt::format("{:02}.outRegEn", nRegsPerBlock * number + 2), 0b0100'0000}
            }},
            {fmt::format("{:02}.dataRate", number), {
                TranslationUnitRoc{fmt::format("{:02}.dataRate", nRegsPerBlock * number + 2), 0b0011'0000}
            }},
            {fmt::format("{:02}.sampleClockSel", number), {
                TranslationUnitRoc{fmt::format("{:02}.sampleClockSel", nRegsPerBlock * number + 2), 0b0000'1100}
            }},
            {fmt::format("{:02}.trackMode", number), {
                TranslationUnitRoc{fmt::format("{:02}.trackMode", nRegsPerBlock * number + 2), 0b0000'0011}
            }},
            {fmt::format("{:02}.enableChannel", number), {
                TranslationUnitRoc{fmt::format("{:02}.enableChannel", nRegsPerBlock * number + 3)}
            }},
            {fmt::format("{:02}.resetChannel", number), {
                TranslationUnitRoc{fmt::format("{:02}.resetChannel", nRegsPerBlock * number + 4)}
            }},
            {fmt::format("{:02}.trainChannel", number), {
                TranslationUnitRoc{fmt::format("{:02}.trainChannel", nRegsPerBlock * number + 5)}
            }},
            {fmt::format("{:02}.phaseSelectChannel0input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel0input", nRegsPerBlock * number + 6), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel1input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel1input", nRegsPerBlock * number + 6), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel2input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel2input", nRegsPerBlock * number + 7), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel3input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel3input", nRegsPerBlock * number + 7), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel4input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel4input", nRegsPerBlock * number + 8), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel5input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel5input", nRegsPerBlock * number + 8), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel6input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel6input", nRegsPerBlock * number + 9), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel7input", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel7input", nRegsPerBlock * number + 9), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel0output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel0output", nRegsPerBlock * number + 10), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel1output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel1output", nRegsPerBlock * number + 10), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel2output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel0output", nRegsPerBlock * number + 11), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel3output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel1output", nRegsPerBlock * number + 11), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel4output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel0output", nRegsPerBlock * number + 12), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel5output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel1output", nRegsPerBlock * number + 12), 0b1111'0000}
            }},
            {fmt::format("{:02}.phaseSelectChannel6output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel0output", nRegsPerBlock * number + 13), 0b0000'1111}
            }},
            {fmt::format("{:02}.phaseSelectChannel7output", number), {
                TranslationUnitRoc{fmt::format("{:02}.phaseSelectChannel1output", nRegsPerBlock * number + 13), 0b1111'0000}
            }},
            {fmt::format("{:02}.rxOff", number), {
                TranslationUnitRoc{fmt::format("{:02}.rxOff", nFirstRxOff + number)}
            }},
            {fmt::format("{:02}.rxTermEnable", number), {
                TranslationUnitRoc{fmt::format("{:02}.rxTermEnable", nFirstRxTermEnable + number)}
            }},
        };
    };
    TranslationMapArt table{};
    for (auto i=0; i<4; i++) {
        table.merge(get(i));
    }
    return table;
}();

static const TranslationMapArt TRANSLATION_MAP_ART_CORE = {
    {"c_disable_arthist", {
        TranslationUnitRoc{"00.c_disable_arthist", 0b1000'0000}
    }},
    {"c_bypass_pa", {
        TranslationUnitRoc{"00.c_bypass_pa", 0b0100'0000}
    }},
    {"rxterm", {
        TranslationUnitRoc{"00.rxterm", 0b0011'0000}
    }},
    {"txcset", {
        TranslationUnitRoc{"00.txcset", 0b0000'1111}
    }},
    {"cfg_dout_time2", {
        TranslationUnitRoc{"01.cfg_dout_time2", 0b1000'0000}
    }},
    {"cfg_bcr_sel", {
        TranslationUnitRoc{"01.cfg_bcr_sel", 0b0100'0000}
    }},
    {"cfg_bcrout_sel", {
        TranslationUnitRoc{"01.cfg_bcrout_sel", 0b0010'0000}
    }},
    {"cfg_art_revall", {
        TranslationUnitRoc{"01.cfg_art_revall", 0b0001'0000}
    }},
    {"cfg_art_revbank", {
        TranslationUnitRoc{"01.cfg_art_revbank", 0b0000'1111}
    }},
    {"cfg_dout_pattern", {
        TranslationUnitRoc{"02.cfg_dout_pattern", 0b1000'0000}
    }},
    {"cfg_dout_artbypass", {
        TranslationUnitRoc{"02.cfg_dout_artbypass", 0b0100'0000}
    }},
    {"cfg_dout_hitlist", {
        TranslationUnitRoc{"02.cfg_dout_hitlist", 0b0010'0000}
    }},
    {"cfg_dout_artflags", {
        TranslationUnitRoc{"02.cfg_dout_artflags", 0b0001'0000}
    }},
    {"cfg_dout_time", {
        TranslationUnitRoc{"02.cfg_dout_time", 0b0000'1000}
    }},
    {"cfg_artmask", {
        TranslationUnitRoc{"02.cfg_artmask", 0b0000'0111}
    }},
    {"cfg_deser_flagmask", {
        TranslationUnitRoc{"03.cfg_deser_flagmask"}
    }},
    {"cfg_deser_flagpatt", {
        TranslationUnitRoc{"04.cfg_deser_flagpatt"}
    }},
    {"cfg_din_invert", {
        TranslationUnitRoc{"05.cfg_din_invert[7:0]", 0b1111'1111, 0x0000'00FF},
        TranslationUnitRoc{"06.cfg_din_invert[15:8]", 0b1111'1111, 0x0000'FF00},
        TranslationUnitRoc{"07.cfg_din_invert[23:16]", 0b1111'1111, 0x00FF'0000},
        TranslationUnitRoc{"08.cfg_din_invert[31:24]", 0b1111'1111, 0xFF00'0000}
    }},
    {"cfg_din_mask", {
        TranslationUnitRoc{"09.cfg_din_mask[7:0]", 0b1111'1111, 0x0000'00FF},
        TranslationUnitRoc{"10.cfg_din_mask[7:0]", 0b1111'1111, 0x0000'FF00},
        TranslationUnitRoc{"11.cfg_din_mask[7:0]", 0b1111'1111, 0x00FF'0000},
        TranslationUnitRoc{"12.cfg_din_mask[7:0]", 0b1111'1111, 0xFF00'0000}
    }},
    {"cfg_bcid0", {
        TranslationUnitRoc{"13.cfg_bcid0[7:0]", 0b1111'1111, 0x0000'00FF},
        TranslationUnitRoc{"14.cfg_bcid0[11:8]", 0b0000'1111, 0x0000'0F00}
    }},
    {"cfg_bcid1", {
        TranslationUnitRoc{"14.cfg_bcid1[3:0]", 0b1111'0000, 0x0000'000F},
        TranslationUnitRoc{"15.cfg_bcid1[11:4]", 0b1111'1111, 0x0000'0FF0}
    }},
    {"cfg_artbypass_sel_ch0", {
        TranslationUnitRoc{"16.cfg_artbypass_sel_ch0[4:0]", 0b0001'1111}
    }},
    {"cfg_artbypass_sel_ch1", {
        TranslationUnitRoc{"16.cfg_artbypass_sel_ch1[2:0]", 0b1110'0000},
        TranslationUnitRoc{"17.cfg_artbypass_sel_ch1[4:3]", 0b0000'0011}
    }},
    {"cfg_artbypass_sel_ch2", {
        TranslationUnitRoc{"17.cfg_artbypass_sel_ch2[4:0]", 0b0111'1100}
    }},
    {"cfg_artbypass_sel_ch3", {
        TranslationUnitRoc{"17.cfg_artbypass_sel_ch3[0]", 0b1000'0000},
        TranslationUnitRoc{"18.cfg_artbypass_sel_ch3[4:1]", 0b0000'1111}
    }},
    {"cfg_artbypass_sel_ch4", {
        TranslationUnitRoc{"18.cfg_artbypass_sel_ch4[3:0]", 0b1111'0000},
        TranslationUnitRoc{"19.cfg_artbypass_sel_ch4[4]", 0b0000'0001}
    }},
    {"cfg_artbypass_sel_ch5", {
        TranslationUnitRoc{"19.cfg_artbypass_sel_ch5[4:0]", 0b0011'1110}
    }},
    {"cfg_artbypass_sel_ch6", {
        TranslationUnitRoc{"19.cfg_artbypass_sel_ch6[1:0]", 0b1100'0000},
        TranslationUnitRoc{"20.cfg_artbypass_sel_ch6[4:2]", 0b0000'0111}
    }},
    {"cfg_artbypass_sel_ch7", {
        TranslationUnitRoc{"20.cfg_artbypass_sel_ch7[4:0]", 0b1111'1000}
    }},
    {"cfg_pattern_data_phi0", {
        TranslationUnitRoc{"21.cfg_pattern_data_phi0[7:0]"},
        TranslationUnitRoc{"22.cfg_pattern_data_phi0[13:8]", 0b0011'1111}
    }},
    {"cfg_pattern_data_phi1", {
        TranslationUnitRoc{"22.cfg_pattern_data_phi1[1:0]", 0b1100'0000},
        TranslationUnitRoc{"23.cfg_pattern_data_phi1[9:2]"},
        TranslationUnitRoc{"24.cfg_pattern_data_phi1[13:10]", 0b0000'1111}
    }},
    {"cfg_pattern_data_phi2", {
        TranslationUnitRoc{"24.cfg_pattern_data_phi2[3:0]", 0b1111'0000},
        TranslationUnitRoc{"25.cfg_pattern_data_phi2[11:4]"},
        TranslationUnitRoc{"26.cfg_pattern_data_phi2[13:12]", 0b0000'0011}
    }},
    {"cfg_pattern_data_phi3", {
        TranslationUnitRoc{"26.cfg_pattern_data_phi3[5:0]", 0b1111'1100},
        TranslationUnitRoc{"27.cfg_pattern_data_phi3[13:6]"}
    }},
    {"cfg_pattern_data_phi4", {
        TranslationUnitRoc{"21.cfg_pattern_data_phi4[7:0]"},
        TranslationUnitRoc{"22.cfg_pattern_data_phi4[13:8]", 0b0011'1111}
    }},
    {"cfg_pattern_data_phi5", {
        TranslationUnitRoc{"22.cfg_pattern_data_phi5[1:0]", 0b1100'0000},
        TranslationUnitRoc{"23.cfg_pattern_data_phi5[9:2]"},
        TranslationUnitRoc{"24.cfg_pattern_data_phi5[13:10]", 0b0000'1111}
    }},
    {"cfg_pattern_data_phi6", {
        TranslationUnitRoc{"24.cfg_pattern_data_phi6[3:0]", 0b1111'0000},
        TranslationUnitRoc{"25.cfg_pattern_data_phi6[11:4]"},
        TranslationUnitRoc{"26.cfg_pattern_data_phi6[13:12]", 0b0000'0011}
    }},
    {"cfg_pattern_data_phi7", {
        TranslationUnitRoc{"26.cfg_pattern_data_phi7[5:0]", 0b1111'1100},
        TranslationUnitRoc{"27.cfg_pattern_data_phi7[13:6]"}
    }},
};

#endif
