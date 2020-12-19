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

static const TranslationMap TRANSLATION_MAP_ROC_ANALOG = {
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
        TranslationUnit{"reg077ePllVmm0.ctrl_bypass_2", 0b0000'1000},
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
        TranslationUnit{"reg093ePllVmm1.ctrl_bypass_2", 0b0000'1000},
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
        TranslationUnit{"reg096ePllTdc.ePllPhase40MHz_0", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_1", {
        TranslationUnit{"reg097ePllTdc.ePllPhase40MHz_1", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_2", {
        TranslationUnit{"reg098ePllTdc.ePllPhase40MHz_2", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase40MHz_3", {
        TranslationUnit{"reg099ePllTdc.ePllPhase40MHz_3", 0b0111'1111},
    }},
    {"ePllTdc.ePllPhase160MHz_0", {
        TranslationUnit{"reg096ePllTdc.ePllPhase160MHz_0[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg100ePllTdc.ePllPhase160MHz_0[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_1", {
        TranslationUnit{"reg097ePllTdc.ePllPhase160MHz_1[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg100ePllTdc.ePllPhase160MHz_1[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_2", {
        TranslationUnit{"reg098ePllTdc.ePllPhase160MHz_2[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg101ePllTdc.ePllPhase160MHz_2[3:0]", 0b0000'1111, 0b0000'1111}
    }},
    {"ePllTdc.ePllPhase160MHz_3", {
        TranslationUnit{"reg099ePllTdc.ePllPhase160MHz_3[4]", 0b1000'0000, 0b0001'0000},
        TranslationUnit{"reg101ePllTdc.ePllPhase160MHz_3[3:0]", 0b1111'0000, 0b0000'1111}
    }},
    {"ePllTdc.ePllInstantLock", {
        TranslationUnit{"reg102ePllTdc.ePllInstantLock", 0b1000'0000},
    }},
    {"ePllTdc.ePllReset", {
        TranslationUnit{"reg102ePllTdc.ePllReset", 0b0100'0000},
    }},
    {"ePllTdc.bypassPLL", {
        TranslationUnit{"reg102ePllTdc.bypassPLL", 0b0010'0000},
    }},
    {"ePllTdc.ePllLockEn", {
        TranslationUnit{"reg102ePllTdc.ePllLockEn", 0b0001'0000},
    }},
    {"ePllTdc.ePllReferenceFrequency", {
        TranslationUnit{"reg102ePllTdc.ePllReferenceFrequency", 0b0000'1100},
    }},
    {"ePllTdc.ePllCap", {
        TranslationUnit{"reg102ePllTdc.ePllCap", 0b0000'0011},
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
        TranslationUnit{"reg109ePllTdc.ctrl_bypass_2", 0b0000'1000},
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

static const TranslationMap TRANSLATION_MAP_ROC_DIGITAL = {
    {"rocId.l1_first", {
        TranslationUnit{"reg000rocId.l1_first", 0b1000'0000}
    }},
    {"rocId.even_parity", {
        TranslationUnit{"reg000rocId.even_parity", 0b0100'0000}
    }},
    {"rocId.roc_id", {
        TranslationUnit{"reg000rocId.roc_id", 0b0011'1111}
    }},
    {"elinkSpeed.sRoc0", {
        TranslationUnit{"reg001elinkSpeed.sroc0", 0b0000'0011}
    }},
    {"elinkSpeed.sRoc1", {
        TranslationUnit{"reg001elinkSpeed.sroc1", 0b0000'1100}
    }},
    {"elinkSpeed.sRoc2", {
        TranslationUnit{"reg001elinkSpeed.sroc2", 0b0011'0000}
    }},
    {"elinkSpeed.sRoc3", {
        TranslationUnit{"reg001elinkSpeed.sroc3", 0b1100'0000}
    }},
    {"sRoc0VmmConnections.vmm0", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc0VmmConnections.vmm1", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc0VmmConnections.vmm2", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc0VmmConnections.vmm3", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc0VmmConnections.vmm4", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc0VmmConnections.vmm5", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc0VmmConnections.vmm6", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc0VmmConnections.vmm7", {
        TranslationUnit{"reg002sRoc0VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc1VmmConnections.vmm0", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc1VmmConnections.vmm1", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc1VmmConnections.vmm2", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc1VmmConnections.vmm3", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc1VmmConnections.vmm4", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc1VmmConnections.vmm5", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc1VmmConnections.vmm6", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc1VmmConnections.vmm7", {
        TranslationUnit{"reg003sRoc1VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc2VmmConnections.vmm0", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc2VmmConnections.vmm1", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc2VmmConnections.vmm2", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc2VmmConnections.vmm3", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc2VmmConnections.vmm4", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc2VmmConnections.vmm5", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc2VmmConnections.vmm6", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc2VmmConnections.vmm7", {
        TranslationUnit{"reg004sRoc2VmmConnections.vmm7", 0b1000'0000}
    }},
    {"sRoc3VmmConnections.vmm0", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm0", 0b0000'0001}
    }},
    {"sRoc3VmmConnections.vmm1", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm1", 0b0000'0010}
    }},
    {"sRoc3VmmConnections.vmm2", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm2", 0b0000'0100}
    }},
    {"sRoc3VmmConnections.vmm3", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm3", 0b0000'1000}
    }},
    {"sRoc3VmmConnections.vmm4", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm4", 0b0001'0000}
    }},
    {"sRoc3VmmConnections.vmm5", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm5", 0b0010'0000}
    }},
    {"sRoc3VmmConnections.vmm6", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm6", 0b0100'0000}
    }},
    {"sRoc3VmmConnections.vmm7", {
        TranslationUnit{"reg005sRoc3VmmConnections.vmm7", 0b1000'0000}
    }},
    {"eopEnable.sRoc0", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc0_eop_enable", 0b0001'0000}
    }},
    {"eopEnable.sRoc1", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc1_eop_enable", 0b0010'0000}
    }},
    {"eopEnable.sRoc2", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc2_eop_enable", 0b0100'0000}
    }},
    {"eopEnable.sRoc3", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc3_eop_enable", 0b1000'0000}
    }},
    {"nullEventEnable.sRoc0", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc0_nullevt_enable", 0b0001'0000}
    }},
    {"nullEventEnable.sRoc1", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc1_nullevt_enable", 0b0010'0000}
    }},
    {"nullEventEnable.sRoc2", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc2_nullevt_enable", 0b0100'0000}
    }},
    {"nullEventEnable.sRoc3", {
        TranslationUnit{"reg006eopAndNullEventEnable.sroc3_nullevt_enable", 0b1000'0000}
    }},
    {"sRocEnable.bypass", {
        TranslationUnit{"reg007sRocEnable.bypass", 0b1000'0000}
    }},
    {"sRocEnable.timeoutEnable", {
        TranslationUnit{"reg007sRocEnable.timeoutEnable", 0b0100'0000}
    }},
    {"sRocEnable.TTCStartBits", {
        TranslationUnit{"reg007sRocEnable.TTCStartBits", 0b0011'0000}
    }},
    {"sRocEnable.sRoc0", {
        TranslationUnit{"reg007sRocEnable.enableSROC0", 0b0000'0001}
    }},
    {"sRocEnable.sRoc1", {
        TranslationUnit{"reg007sRocEnable.enableSROC1", 0b0000'0010}
    }},
    {"sRocEnable.sRoc2", {
        TranslationUnit{"reg007sRocEnable.enableSROC2", 0b0000'0100}
    }},
    {"sRocEnable.sRoc3", {
        TranslationUnit{"reg007sRocEnable.enableSROC3", 0b0000'1000}
    }},
    {"vmmEnable.vmm0", {
        TranslationUnit{"reg008vmmEnable.vmm0", 0b0000'0001}
    }},
    {"vmmEnable.vmm1", {
        TranslationUnit{"reg008vmmEnable.vmm1", 0b0000'0010}
    }},
    {"vmmEnable.vmm2", {
        TranslationUnit{"reg008vmmEnable.vmm2", 0b0000'0100}
    }},
    {"vmmEnable.vmm3", {
        TranslationUnit{"reg008vmmEnable.vmm3", 0b0000'1000}
    }},
    {"vmmEnable.vmm4", {
        TranslationUnit{"reg008vmmEnable.vmm4", 0b0001'0000}
    }},
    {"vmmEnable.vmm5", {
        TranslationUnit{"reg008vmmEnable.vmm5", 0b0010'0000}
    }},
    {"vmmEnable.vmm6", {
        TranslationUnit{"reg008vmmEnable.vmm6", 0b0100'0000}
    }},
    {"vmmEnable.vmm7", {
        TranslationUnit{"reg008vmmEnable.vmm7", 0b1000'0000}
    }},
    {"FIXME.timeout", {
        TranslationUnit{"reg009timeout.timeout"}
    }},
    {"FIXME.tx_csel", {
        TranslationUnit{"reg010bcOffset0_txcSel.tx_csel", 0b1111'0000}
    }},
    {"FIXME.bc_offset", {
        TranslationUnit{"reg010bcOffset0_txcSel.bc_offset[11:8]", 0b0000'1111},
        TranslationUnit{"reg011bcOffset1.bc_offset[7:0]"}
    }},
    {"FIXME.bc_rollover", {
        TranslationUnit{"reg012bcRollover0.bc_rollover[11:8]", 0b0000'1111},
        TranslationUnit{"reg013bcRollover1.bc_rollover[7:0]"}
    }},
    {"eportEnable.sRoc0", {
        TranslationUnit{"reg014eportEnable.sroc0", 0b0000'0011}
    }},
    {"eportEnable.sRoc1", {
        TranslationUnit{"reg014eportEnable.sroc1", 0b0000'1100}
    }},
    {"eportEnable.sRoc2", {
        TranslationUnit{"reg014eportEnable.sroc2", 0b0011'0000}
    }},
    {"eportEnable.sRoc3", {
        TranslationUnit{"reg014eportEnable.sroc3", 0b1100'0000}
    }},
    {"timeoutStatus.vmm0", {
        TranslationUnit{"reg063timeoutStatus.vmm0", 0b0000'0001}
    }},
    {"timeoutStatus.vmm1", {
        TranslationUnit{"reg063timeoutStatus.vmm1", 0b0000'0010}
    }},
    {"timeoutStatus.vmm2", {
        TranslationUnit{"reg063timeoutStatus.vmm2", 0b0000'0100}
    }},
    {"timeoutStatus.vmm3", {
        TranslationUnit{"reg063timeoutStatus.vmm3", 0b0000'1000}
    }},
    {"timeoutStatus.vmm4", {
        TranslationUnit{"reg063timeoutStatus.vmm4", 0b0001'0000}
    }},
    {"timeoutStatus.vmm5", {
        TranslationUnit{"reg063timeoutStatus.vmm5", 0b0010'0000}
    }},
    {"timeoutStatus.vmm6", {
        TranslationUnit{"reg063timeoutStatus.vmm6", 0b0100'0000}
    }},
    {"timeoutStatus.vmm7", {
        TranslationUnit{"reg063timeoutStatus.vmm7", 0b1000'0000}
    }},
    {"busyEnable.sRoc0", {
        TranslationUnit{"reg020busyAndTdcEnable.busy_enable_sroc0", 0b0000'0001}
    }},
    {"busyEnable.sRoc1", {
        TranslationUnit{"reg020busyAndTdcEnable.busy_enable_sroc1", 0b0000'0010}
    }},
    {"busyEnable.sRoc2", {
        TranslationUnit{"reg020busyAndTdcEnable.busy_enable_sroc2", 0b0000'0100}
    }},
    {"busyEnable.sRoc3", {
        TranslationUnit{"reg020busyAndTdcEnable.busy_enable_sroc3", 0b0000'1000}
    }},
    {"tdcEnable.sRoc0", {
        TranslationUnit{"reg020busyAndTdcEnable.tdc_enable_sroc0", 0b0001'0000}
    }},
    {"tdcEnable.sRoc1", {
        TranslationUnit{"reg020busyAndTdcEnable.tdc_enable_sroc1", 0b0010'0000}
    }},
    {"tdcEnable.sRoc2", {
        TranslationUnit{"reg020busyAndTdcEnable.tdc_enable_sroc2", 0b0100'0000}
    }},
    {"tdcEnable.sRoc3", {
        TranslationUnit{"reg020busyAndTdcEnable.tdc_enable_sroc3", 0b1000'0000}
    }},
    {"FIXME.busyOnLimit", {
        TranslationUnit{"reg021busyOnLimit0.busy_on_limit[10:8]", 0b0000'0111, 0b0111'0000'0000},
        TranslationUnit{"reg022busyOnLimit1.busy_on_limit[7:0]", 0b1111'1111, 0b0000'1111'1111}
    }},
    {"FIXME.busyOffLimit", {
        TranslationUnit{"reg023busyOffLimit0.busy_off_limit[10:8]", 0b0000'0111},
        TranslationUnit{"reg024busyOffLimit1.busy_off_limit[7:0]"}
    }},
    {"FIXME.l1EventsWithoutComma", {
        TranslationUnit{"reg031l1EventsWithoutComma.l1_events_no_comma"}
    }},
    {"fakeVmmFailure.vmm0", {
        TranslationUnit{"reg019fakeVmmFailure.vmm0", 0b0000'0001}
    }},
    {"fakeVmmFailure.vmm1", {
        TranslationUnit{"reg019fakeVmmFailure.vmm1", 0b0000'0010}
    }},
    {"fakeVmmFailure.vmm2", {
        TranslationUnit{"reg019fakeVmmFailure.vmm2", 0b0000'0100}
    }},
    {"fakeVmmFailure.vmm3", {
        TranslationUnit{"reg019fakeVmmFailure.vmm3", 0b0000'1000}
    }},
    {"fakeVmmFailure.vmm4", {
        TranslationUnit{"reg019fakeVmmFailure.vmm4", 0b0001'0000}
    }},
    {"fakeVmmFailure.vmm5", {
        TranslationUnit{"reg019fakeVmmFailure.vmm5", 0b0010'0000}
    }},
    {"fakeVmmFailure.vmm6", {
        TranslationUnit{"reg019fakeVmmFailure.vmm6", 0b0100'0000}
    }},
    {"fakeVmmFailure.vmm7", {
        TranslationUnit{"reg019fakeVmmFailure.vmm7", 0b1000'0000}
    }}
};

#endif
