#ifndef NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H
#define NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H

#include <map>
#include <vector>
#include <string>

struct TranslationUnit
{
    std::string m_registerName{""};
    unsigned int m_maskRegister{static_cast<unsigned int>(-1)};
    unsigned int m_maskValue{static_cast<unsigned int>(-1)};
};

using TranslationMap = std::map<std::string, std::vector<TranslationUnit>>;

static const TranslationMap TRANSLATION_MAP = {
    {"ePllVmm0.ePllPhase40MHz_0", {
        TranslationUnit{"reg064ePllVmm0.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_1", {
        TranslationUnit{"reg065ePllVmm0.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_2", {
        TranslationUnit{"reg066ePllVmm0.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase40MHz_3", {
        TranslationUnit{"reg067ePllVmm0.ePllPhase40MHz_3", 0b0111'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_0", {
        TranslationUnit{"reg064ePllVmm0.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg068ePllVmm0.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_1", {
        TranslationUnit{"reg065ePllVmm0.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg068ePllVmm0.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_2", {
        TranslationUnit{"reg066ePllVmm0.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg069ePllVmm0.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm0.ePllPhase160MHz_3", {
        TranslationUnit{"reg067ePllVmm0.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg069ePllVmm0.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm0.ePllInstantLock", {
        TranslationUnit{"reg070ePllVmm0.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllVmm0.ePllReset", {
        TranslationUnit{"reg070ePllVmm0.ePllReset", 0b0100'0000},
    }},
    {"ePllVmm0.bypassPLL", {
        TranslationUnit{"reg070ePllVmm0.bypassPLL", 0b0010'0000},
    }},
    {"ePllVmm0.ePllLockEn", {
        TranslationUnit{"reg070ePllVmm0.ePllLockEn", 0b0001'0000},
    }},
    {"ePllVmm0.ePllReferenceFrequency", {
        TranslationUnit{"reg070ePllVmm0.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllVmm0.ePllCap", {
        TranslationUnit{"reg070ePllVmm0.ePllCap", 0b0000'0011},
    }},
    {"ePllVmm0.ePllRes", {
        TranslationUnit{"reg071ePllVmm0.ePllRes", 0b1111'0000},
    }},
    {"ePllVmm0.ePllIcp", {
        TranslationUnit{"reg071ePllVmm0.ePllIcp", 0b0000'1111},
    }},
    {"ePllVmm0.ePllEnablePhase", {
        TranslationUnit{"reg072ePllVmm0.ePllEnablePhase"},
    }},
    {"ePllVmm0.tp_bypass_0", {
        TranslationUnit{"reg073ePllVmm0.tp_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm0.tp_bypass_1", {
        TranslationUnit{"reg073ePllVmm0.tp_bypass_1", 0b1000'0000},
    }},
    {"ePllVmm0.tp_bypass_2", {
        TranslationUnit{"reg074ePllVmm0.tp_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm0.tp_bypass_3", {
        TranslationUnit{"reg074ePllVmm0.tp_bypass_3", 0b1000'0000},
    }},
    {"ePllVmm0.tp_phase_0", {
        TranslationUnit{"reg073ePllVmm0.tp_phase_0", 0b0000'0111},
    }},
    {"ePllVmm0.tp_phase_1", {
        TranslationUnit{"reg073ePllVmm0.tp_phase_1", 0b0111'0000},
    }},
    {"ePllVmm0.tp_phase_2", {
        TranslationUnit{"reg074ePllVmm0.tp_phase_2", 0b0000'0111},
    }},
    {"ePllVmm0.tp_phase_3", {
        TranslationUnit{"reg074ePllVmm0.tp_phase_3", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_05delay_0", {
        TranslationUnit{"reg075ePllVmm0.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_1", {
        TranslationUnit{"reg076ePllVmm0.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_2", {
        TranslationUnit{"reg077ePllVmm0.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_05delay_3", {
        TranslationUnit{"reg078ePllVmm0.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllVmm0.ctrl_delay_0", {
        TranslationUnit{"reg075ePllVmm0.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_1", {
        TranslationUnit{"reg076ePllVmm0.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_2", {
        TranslationUnit{"reg077ePllVmm0.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_delay_3", {
        TranslationUnit{"reg078ePllVmm0.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllVmm0.ctrl_bypass_0", {
        TranslationUnit{"reg075ePllVmm0.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_1", {
        TranslationUnit{"reg076ePllVmm0.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_2", {
        TranslationUnit{"reg075ePllVmm0.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_bypass_3", {
        TranslationUnit{"reg078ePllVmm0.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllVmm0.ctrl_phase_0", {
        TranslationUnit{"reg075ePllVmm0.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_1", {
        TranslationUnit{"reg076ePllVmm0.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_2", {
        TranslationUnit{"reg077ePllVmm0.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllVmm0.ctrl_phase_3", {
        TranslationUnit{"reg078ePllVmm0.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllVmm0.tx_enable", {
        TranslationUnit{"reg079ePllVmm0.tx_enable", 0b1111'0000},
    }},
    {"ePllVmm0.tx_csel", {
        TranslationUnit{"reg079ePllVmm0.tx_csel", 0b0000'1111},
    }},
    {"ePllVmm1.ePllPhase40MHz_0", {
        TranslationUnit{"reg080ePllVmm1.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_1", {
        TranslationUnit{"reg081ePllVmm1.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_2", {
        TranslationUnit{"reg082ePllVmm1.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase40MHz_3", {
        TranslationUnit{"reg083ePllVmm1.ePllPhase40MHz_3", 0b0111'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_0", {
        TranslationUnit{"reg080ePllVmm1.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg084ePllVmm1.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_1", {
        TranslationUnit{"reg081ePllVmm1.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg084ePllVmm1.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_2", {
        TranslationUnit{"reg082ePllVmm1.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg085ePllVmm1.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllVmm1.ePllPhase160MHz_3", {
        TranslationUnit{"reg083ePllVmm1.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg085ePllVmm1.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllVmm1.ePllInstantLock", {
        TranslationUnit{"reg086ePllVmm1.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllVmm1.ePllReset", {
        TranslationUnit{"reg086ePllVmm1.ePllReset", 0b0100'0000},
    }},
    {"ePllVmm1.bypassPLL", {
        TranslationUnit{"reg086ePllVmm1.bypassPLL", 0b0010'0000},
    }},
    {"ePllVmm1.ePllLockEn", {
        TranslationUnit{"reg086ePllVmm1.ePllLockEn", 0b0001'0000},
    }},
    {"ePllVmm1.ePllReferenceFrequency", {
        TranslationUnit{"reg086ePllVmm1.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllVmm1.ePllCap", {
        TranslationUnit{"reg086ePllVmm1.ePllCap", 0b0000'0011},
    }},
    {"ePllVmm1.ePllRes", {
        TranslationUnit{"reg087ePllVmm1.ePllRes", 0b1111'0000},
    }},
    {"ePllVmm1.ePllIcp", {
        TranslationUnit{"reg087ePllVmm1.ePllIcp", 0b0000'1111},
    }},
    {"ePllVmm1.ePllEnablePhase", {
        TranslationUnit{"reg088ePllVmm1.ePllEnablePhase"},
    }},
    {"ePllVmm1.tp_bypass_0", {
        TranslationUnit{"reg089ePllVmm1.tp_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm1.tp_bypass_1", {
        TranslationUnit{"reg089ePllVmm1.tp_bypass_1", 0b1000'0000},
    }},
    {"ePllVmm1.tp_bypass_2", {
        TranslationUnit{"reg090ePllVmm1.tp_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm1.tp_bypass_3", {
        TranslationUnit{"reg090ePllVmm1.tp_bypass_3", 0b1000'0000},
    }},
    {"ePllVmm1.tp_phase_0", {
        TranslationUnit{"reg089ePllVmm1.tp_phase_0", 0b0000'0111},
    }},
    {"ePllVmm1.tp_phase_1", {
        TranslationUnit{"reg089ePllVmm1.tp_phase_1", 0b0111'0000},
    }},
    {"ePllVmm1.tp_phase_2", {
        TranslationUnit{"reg090ePllVmm1.tp_phase_2", 0b0000'0111},
    }},
    {"ePllVmm1.tp_phase_3", {
        TranslationUnit{"reg090ePllVmm1.tp_phase_3", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_05delay_0", {
        TranslationUnit{"reg091ePllVmm1.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_1", {
        TranslationUnit{"reg092ePllVmm1.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_2", {
        TranslationUnit{"reg093ePllVmm1.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_05delay_3", {
        TranslationUnit{"reg094ePllVmm1.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllVmm1.ctrl_delay_0", {
        TranslationUnit{"reg091ePllVmm1.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_1", {
        TranslationUnit{"reg092ePllVmm1.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_2", {
        TranslationUnit{"reg093ePllVmm1.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_delay_3", {
        TranslationUnit{"reg094ePllVmm1.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllVmm1.ctrl_bypass_0", {
        TranslationUnit{"reg091ePllVmm1.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_1", {
        TranslationUnit{"reg092ePllVmm1.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_2", {
        TranslationUnit{"reg091ePllVmm1.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_bypass_3", {
        TranslationUnit{"reg094ePllVmm1.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllVmm1.ctrl_phase_0", {
        TranslationUnit{"reg091ePllVmm1.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_1", {
        TranslationUnit{"reg092ePllVmm1.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_2", {
        TranslationUnit{"reg093ePllVmm1.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllVmm1.ctrl_phase_3", {
        TranslationUnit{"reg094ePllVmm1.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllVmm1.tx_enable", {
        TranslationUnit{"reg095ePllVmm1.tx_enable", 0b1111'0000},
    }},
    {"ePllVmm1.tx_csel", {
        TranslationUnit{"reg095ePllVmm1.tx_csel", 0b0000'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_0", {
        TranslationUnit{"reg096ePllTdc.ePllPhase160MHz_0", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_1", {
        TranslationUnit{"reg097ePllTdc.ePllPhase160MHz_1", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_2", {
        TranslationUnit{"reg098ePllTdc.ePllPhase160MHz_2", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_3", {
        TranslationUnit{"reg099ePllTdc.ePllPhase160MHz_3", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase160MHz_0", {
        TranslationUnit{"reg096ePllTdc.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg100ePllTdc.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_1", {
        TranslationUnit{"reg097ePllTdc.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg100ePllTdc.ePllPhase160MHz_1[3:0]", 0b0000'1111, 0b1111'0000}
    }},
    {"ePllTdc.ePllPhase160MHz_2", {
        TranslationUnit{"reg098ePllTdc.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg101ePllTdc.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_3", {
        TranslationUnit{"reg099ePllTdc.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg101ePllTdc.ePllPhase160MHz_3[3:0]", 0b0000'1111, 0b1111'0000}
    }},
    {"ePllTdc.ePllInstantLock", {
        TranslationUnit{"reg120ePllTdc.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllTdc.ePllReset", {
        TranslationUnit{"reg120ePllTdc.ePllReset", 0b0100'0000},
    }},
    {"ePllTdc.bypassPLL", {
        TranslationUnit{"reg120ePllTdc.bypassPLL", 0b0010'0000},
    }},
    {"ePllTdc.ePllLockEn", {
        TranslationUnit{"reg120ePllTdc.ePllLockEn", 0b0001'0000},
    }},
    {"ePllTdc.ePllReferenceFrequency", {
        TranslationUnit{"reg120ePllTdc.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllTdc.ePllCap", {
        TranslationUnit{"reg120ePllTdc.ePllCap", 0b0000'0011},
    }},
    {"ePllTdc.ePllRes", {
        TranslationUnit{"reg103ePllTdc.ePllRes", 0b1111'0000},
    }},
    {"ePllTdc.ePllIcp", {
        TranslationUnit{"reg103ePllTdc.ePllIcp", 0b0000'1111},
    }},
    {"ePllTdc.ePllEnablePhase", {
        TranslationUnit{"reg104ePllTdc.ePllEnablePhase"},
    }},
    {"ePllTdc.enable160MHzOnBCR", {
        TranslationUnit{"reg105ePllTdc.enable160MHzOnBCR", 0b1111'0000},
    }},
    {"ePllTdc.enable160MHzOn40MHz", {
        TranslationUnit{"reg105ePllTdc.enable160MHzOn40MHz", 0b0000'1111},
    }},
    {"ePllTdc.tx_enable_bcr", {
        TranslationUnit{"reg106ePllTdc.tx_enable_bcr", 0b1111'0000},
    }},
    {"ePllTdc.tx_csel_bcr", {
        TranslationUnit{"reg106ePllTdc.tx_csel_bcr", 0b0000'1111},
    }},
    {"ePllTdc.ctrl_05delay_0", {
        TranslationUnit{"reg107ePllTdc.ctrl_05delay_0", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_1", {
        TranslationUnit{"reg108ePllTdc.ctrl_05delay_1", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_2", {
        TranslationUnit{"reg109ePllTdc.ctrl_05delay_2", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_05delay_3", {
        TranslationUnit{"reg110ePllTdc.ctrl_05delay_3", 0b1000'0000},
    }},
    {"ePllTdc.ctrl_delay_0", {
        TranslationUnit{"reg107ePllTdc.ctrl_delay_0", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_1", {
        TranslationUnit{"reg108ePllTdc.ctrl_delay_1", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_2", {
        TranslationUnit{"reg109ePllTdc.ctrl_delay_2", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_delay_3", {
        TranslationUnit{"reg110ePllTdc.ctrl_delay_3", 0b0111'0000},
    }},
    {"ePllTdc.ctrl_bypass_0", {
        TranslationUnit{"reg107ePllTdc.ctrl_bypass_0", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_1", {
        TranslationUnit{"reg108ePllTdc.ctrl_bypass_1", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_2", {
        TranslationUnit{"reg107ePllTdc.ctrl_bypass_2", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_bypass_3", {
        TranslationUnit{"reg110ePllTdc.ctrl_bypass_3", 0b0000'1000},
    }},
    {"ePllTdc.ctrl_phase_0", {
        TranslationUnit{"reg107ePllTdc.ctrl_phase_0", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_1", {
        TranslationUnit{"reg108ePllTdc.ctrl_phase_1", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_2", {
        TranslationUnit{"reg109ePllTdc.ctrl_phase_2", 0b0000'0111},
    }},
    {"ePllTdc.ctrl_phase_3", {
        TranslationUnit{"reg110ePllTdc.ctrl_phase_3", 0b0000'0111},
    }},
    {"ePllTdc.tx_enable", {
        TranslationUnit{"reg111ePllTdc.tx_enable", 0b1111'0000},
    }},
    {"ePllTdc.tx_csel", {
        TranslationUnit{"reg111ePllTdc.tx_csel", 0b0000'1111},
    }},
    {"FIXME.ePllInstantLock", {
        TranslationUnit{"reg112.ePllInstantLock", 0b1000'0000},
    }},
    {"FIXME.ePllReset", {
        TranslationUnit{"reg112.ePllReset", 0b0100'0000},
    }},
    {"FIXME.bypassPLL", {
        TranslationUnit{"reg112.bypassPLL", 0b0010'0000},
    }},
    {"FIXME.ePllLockEn", {
        TranslationUnit{"reg112.ePllLockEn", 0b0001'0000},
    }},
    {"FIXME.ePllReferenceFrequency", {
        TranslationUnit{"reg112.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"FIXME.ePllCap", {
        TranslationUnit{"reg112.ePllCap", 0b0000'0011},
    }},
    {"FIXME.ePllRes", {
        TranslationUnit{"reg113.ePllRes", 0b1111'0000},
    }},
    {"FIXME.ePllIcp", {
        TranslationUnit{"reg113.ePllIcp", 0b0000'1111},
    }},
    {"FIXME.ePllEnablePhase", {
        TranslationUnit{"reg114.ePllEnablePhase"},
    }},
    {"FIXME.ePllPhase40MHz_0", {
        TranslationUnit{"reg115.ePllPhase40MHz_0", 0b0111'1111}
    }},
    {"FIXME.ePllPhase40MHz_1", {
        TranslationUnit{"reg116.ePllPhase40MHz_1", 0b0111'1111}
    }},
    {"FIXME.ePllPhase40MHz_2", {
        TranslationUnit{"reg117.ePllPhase40MHz_2", 0b0111'1111}
    }},
    {"FIXME.ePllPhase160MHz_0", {
        TranslationUnit{"reg115.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg118.ePllPhase160MHz_0[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"FIXME.ePllPhase160MHz_1", {
        TranslationUnit{"reg116.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg118.ePllPhase160MHz_1[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"FIXME.ePllPhase160MHz_2", {
        TranslationUnit{"reg117.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg119.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"FIXME.tp_bypass_global", {
        TranslationUnit{"reg119.tp_bypass_global", 0b1000'0000}
    }},
    {"FIXME.tp_phase_global", {
        TranslationUnit{"reg119.tp_phase_global", 0b0111'0000}
    }},
    {"FIXME.TDS_BCR_INV", {
        TranslationUnit{"reg120.TDS_BCR_INV", 0b1111'0000}
    }},
    {"FIXME.LockOutInv", {
        TranslationUnit{"reg120.LockOutInv", 0b0000'1000}
    }},
    {"FIXME.testOutEn", {
        TranslationUnit{"reg120.testOutEn", 0b0000'0100}
    }},
    {"FIXME.testOutMux", {
        TranslationUnit{"reg120.testOutMux", 0b0000'0011}
    }},
    {"FIXME.vmmBcrInv", {
        TranslationUnit{"reg121vmmBcrInv.vmmBcrInv"}
    }},
    {"FIXME.vmmEnaInv", {
        TranslationUnit{"reg122vmmEnaInv.vmmEnaInv"}
    }},
    {"FIXME.vmmL0Inv", {
        TranslationUnit{"reg123vmmL0Inv.vmmL0Inv"}
    }},
    {"FIXME.vmmTpInv", {
        TranslationUnit{"reg124vmmTpInv.vmmTpInv"}
    }}
};

#endif
