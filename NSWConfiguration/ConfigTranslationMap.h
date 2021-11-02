#ifndef NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H
#define NSWCONFIGURATION_CONFIGTRANSLATIONMAP_H

#include <iomanip>
#include <map>
#include <sstream>
#include <vector>
#include <string>

#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"

/** \brief Defines translation between register and value based configuration entries
 *  Every value has a list of translation units which define how the value is mapped to
 *  registers.
 */
template<typename RegisterSize>
struct TranslationUnit {
  std::string m_registerName{};  ///< <register>.<subregister>

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
  // clang-format off
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
  // clang-format on
};

static const TranslationMapRoc TRANSLATION_MAP_ROC_DIGITAL = [] () {
  TranslationMapRoc table{
    // clang-format off
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
    }},
    {"seu.status", {
      TranslationUnitRoc{"reg044seu_READONLY.seu", 0b0000'0001}
    }},
    {"seu.counter", {
      TranslationUnitRoc{"reg053seuCounter_READONLY.seu_counter"}
    }}
    // clang-format on
  };
  constexpr std::size_t FIRST_REG_VMM_CAPTURE = 32;
  for (std::size_t vmmId = 0; vmmId < nsw::MAX_NUMBER_OF_VMM; vmmId++) {
    table.merge(TranslationMapRoc{
      {fmt::format("vmmCaptureStatus.vmm{}.fifoFullError", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}vmmCapture{}Status_READONLY.fifo_full_err", FIRST_REG_VMM_CAPTURE + vmmId, vmmId), 0b0001'0000}
      }},
      {fmt::format("vmmCaptureStatus.vmm{}.coherencyError", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}vmmCapture{}Status_READONLY.coherency_err", FIRST_REG_VMM_CAPTURE + vmmId, vmmId), 0b0000'1000}
      }},
      {fmt::format("vmmCaptureStatus.vmm{}.decoderError", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}vmmCapture{}Status_READONLY.dec_err", FIRST_REG_VMM_CAPTURE + vmmId, vmmId), 0b0000'0100}
      }},
      {fmt::format("vmmCaptureStatus.vmm{}.misalignmentError", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}vmmCapture{}Status_READONLY.misaligned_err", FIRST_REG_VMM_CAPTURE + vmmId, vmmId), 0b0000'0010}
      }},
      {fmt::format("vmmCaptureStatus.vmm{}.aligned", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}vmmCapture{}Status_READONLY.aligned", FIRST_REG_VMM_CAPTURE + vmmId, vmmId), 0b0000'0001}
      }}
    });
  }
  constexpr std::size_t FIRST_REG_SROC_STATUS = 40;
  for (std::size_t srocId = 0; srocId < nsw::roc::NUM_SROCS; srocId++) {
    table.merge(TranslationMapRoc{
      {fmt::format("sRocStatus.sRoc{}.ttcFifoFull", srocId), {
        TranslationUnitRoc{fmt::format("reg{:03}sRoc{}Status_READONLY.ttc_fifo_full", FIRST_REG_SROC_STATUS + srocId, srocId), 0b0000'0100}
      }},
      {fmt::format("sRocStatus.sRoc{}.encoderError", srocId), {
        TranslationUnitRoc{fmt::format("reg{:03}sRoc{}Status_READONLY.enc_err", FIRST_REG_SROC_STATUS + srocId, srocId), 0b0000'0010}
      }},
      {fmt::format("sRocStatus.sRoc{}.eventFull", srocId), {
        TranslationUnitRoc{fmt::format("reg{:03}sRoc{}Status_READONLY.event_full", FIRST_REG_SROC_STATUS + srocId, srocId), 0b0000'0001}
      }}
    });
  }
  constexpr std::size_t FIRST_REG_VMM_PARITY = 45;
  for (std::size_t vmmId = 0; vmmId < nsw::MAX_NUMBER_OF_VMM; vmmId++) {
    table.merge(TranslationMapRoc{
      {fmt::format("vmmParityCounter.vmm{}", vmmId), {
        TranslationUnitRoc{fmt::format("reg{:03}parityCounterVmm{}_READONLY.parity_counter", FIRST_REG_VMM_PARITY + vmmId, vmmId)}
      }}
    });
  }
  return table;
}();

static const TranslationMapTds TRANSLATION_MAP_TDS = []() {
  // FIXME: Origin of magic numbers: I2cRegisterMappings.h
  // clang-format off
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
    {"Ck160_1_Phase", {
      TranslationUnitTds{"register1.Ck160_1_Phase", 0xF800}
    }},
    {"Ck160_0_Phase", {
      TranslationUnitTds{"register1.Ck160_0_Phase", 0x07C0}
    }},
    {"SER_PLL_I", {
      TranslationUnitTds{"register1.SER_PLL_I", 0x003C}
    }},
    {"SER_PLL_R", {
      TranslationUnitTds{"register1.SER_PLL_R", 0x0003}
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
    }},
    {"Monitor_Strip64", {
      TranslationUnitTds{"register14_READONLY.Monitor_Strip64", 0x7FFF'F000'0000}
    }},
    {"Monitor_Strip0", {
      TranslationUnitTds{"register14_READONLY.Monitor_Strip0", 0x000'07FF'FF00}
    }},
    {"SER_lock", {
      TranslationUnitTds{"register14_READONLY.SER_lock", 0x0000'0000'0002}
    }},
    {"Pll_lock", {
      TranslationUnitTds{"register14_READONLY.Pll_lock", 0x0000'0000'0001}
    }},
    {"strip_trigger_bcid", {
      TranslationUnitTds{"register15_READONLY.strip_trigger_bcid", 0x0FFF'0000}
    }},
    {"strip_trigger_band_phid", {
      TranslationUnitTds{"register15_READONLY.strip_trigger_band_phid", 0x0000'1FFF}
    }}
  };
  // clang-format on
  for (unsigned int i = 0; i < 128; i++) {
    const auto key = fmt::format("Chan{:03}", i);
    table[fmt::format("{}_Disable", key)] =
      std::vector{TranslationUnitTds{fmt::format("register2.{}", key), __uint128_t{0x1} << i}};
  }
  const auto reg3and4 = [&table](const std::string& regname, const unsigned int start) {
    for (unsigned int i = 0; i < 8; i++) {
      const auto key = fmt::format("trig_lut{:x}", i + start);
      table[key] = std::vector{
        TranslationUnitTds{fmt::format("{}.{}", regname, key), __uint128_t{0x7FFF} << i * 16}};
    }
  };
  reg3and4("register3", 0);
  reg3and4("register4", 8);
  const auto reg5to10 = [&table](const std::string& regname, const unsigned int start) {
    for (unsigned int i = 0; i < 16; i++) {
      const auto key = fmt::format("Pad_Chan{:x}", i + start);
      table[fmt::format("{}_Delay", key)] = std::vector{
        TranslationUnitTds{fmt::format("{}.{}", regname, key), __uint128_t{0x1F} << i * 8}};
    }
  };
  reg5to10("register5", 0);
  reg5to10("register6", 16);
  reg5to10("register7", 32);
  reg5to10("register8", 48);
  reg5to10("register9", 64);
  reg5to10("register10", 80);
  for (unsigned int i = 0; i < 8; i++) {
    const auto key = fmt::format("Pad_Chan{:x}", i + 96);
    table[fmt::format("{}_Delay", key)] = std::vector{
      TranslationUnitTds{fmt::format("register11.{}", key), __uint128_t{0x1F} << i * 8}};
  }
  for (std::size_t i = 0; i < 16; i++) {
    constexpr auto N_ELEMENTS_PER_BLOCK = 81;
    const auto end = (N_ELEMENTS_PER_BLOCK) * (i + 1) - 1;
    const auto start = N_ELEMENTS_PER_BLOCK * i;
    table[fmt::format("CRC_CONFIG[{}:{}]", end, start)] = std::vector{TranslationUnitTds{
      fmt::format("register13_READONLY.CRC[{}:{}]", end, start), __uint128_t{0xFF} << i * 8u}};
  }
  return table;
}();

static const auto TRANSLATION_MAP_ART_PS = []() -> TranslationMapArt {
  const auto get = [](const int number) -> TranslationMapArt {
    const auto nRegsPerBlock = 15;
    const auto nFirstRxOff = 62;
    const auto nFirstRxTermEnable = 66;
    return {
      // clang-format off
      {fmt::format("{:02}.dllLockedV", number), {
        TranslationUnitArt{fmt::format("{:02}.dllLockedV", nRegsPerBlock * number + 0), 0b1000'0000}
      }},
      {fmt::format("{:02}.reserved", number), {
        TranslationUnitArt{fmt::format("{:02}.reserved", nRegsPerBlock * number + 0), 0b0100'0000}
      }},
      {fmt::format("{:02}.dllLockCfg", number), {
        TranslationUnitArt{fmt::format("{:02}.dllLockCfg", nRegsPerBlock * number + 0), 0b0011'0000}
      }},
      {fmt::format("{:02}.muxEn2to8", number), {
        TranslationUnitArt{fmt::format("{:02}.muxEn2to8", nRegsPerBlock * number + 0), 0b0000'1000}
      }},
      {fmt::format("{:02}.muxEn1to8", number), {
        TranslationUnitArt{fmt::format("{:02}.muxEn1to8", nRegsPerBlock * number + 0), 0b0000'0100}
      }},
      {fmt::format("{:02}.dllCoarseLockDetection", number), {
        TranslationUnitArt{fmt::format("{:02}.dllCoarseLockDetection", nRegsPerBlock * number + 0), 0b0000'0010}
      }},
      {fmt::format("{:02}.dllResetFromCfg", number), {
        TranslationUnitArt{fmt::format("{:02}.dllResetFromCfg", nRegsPerBlock * number + 0), 0b0000'0001}
      }},
      {fmt::format("{:02}.dataRateDll", number), {
        TranslationUnitArt{fmt::format("{:02}.dataRateDll", nRegsPerBlock * number + 1), 0b1100'0000}
      }},
      {fmt::format("{:02}.dllConfirmCountSelect", number), {
        TranslationUnitArt{fmt::format("{:02}.dllConfirmCountSelect", nRegsPerBlock * number + 1), 0b0011'0000}
      }},
      {fmt::format("{:02}.dllChargePumpCurrent", number), {
        TranslationUnitArt{fmt::format("{:02}.dllChargePumpCurrent", nRegsPerBlock * number + 1), 0b0000'1111}
      }},
      {fmt::format("{:02}.enableGroup", number), {
        TranslationUnitArt{fmt::format("{:02}.enableGroup", nRegsPerBlock * number + 2), 0b1000'0000}
      }},
      {fmt::format("{:02}.outRegEn", number), {
        TranslationUnitArt{fmt::format("{:02}.outRegEn", nRegsPerBlock * number + 2), 0b0100'0000}
      }},
      {fmt::format("{:02}.dataRate", number), {
        TranslationUnitArt{fmt::format("{:02}.dataRate", nRegsPerBlock * number + 2), 0b0011'0000}
      }},
      {fmt::format("{:02}.sampleClockSel", number), {
        TranslationUnitArt{fmt::format("{:02}.sampleClockSel", nRegsPerBlock * number + 2), 0b0000'1100}
      }},
      {fmt::format("{:02}.trackMode", number), {
        TranslationUnitArt{fmt::format("{:02}.trackMode", nRegsPerBlock * number + 2), 0b0000'0011}
      }},
      {fmt::format("{:02}.enableChannel", number), {
        TranslationUnitArt{fmt::format("{:02}.enableChannel", nRegsPerBlock * number + 3)}
      }},
      {fmt::format("{:02}.resetChannel", number), {
        TranslationUnitArt{fmt::format("{:02}.resetChannel", nRegsPerBlock * number + 4)}
      }},
      {fmt::format("{:02}.trainChannel", number), {
        TranslationUnitArt{fmt::format("{:02}.trainChannel", nRegsPerBlock * number + 5)}
      }},
      {fmt::format("{:02}.phaseSelectChannel0input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel0input", nRegsPerBlock * number + 6), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel1input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel1input", nRegsPerBlock * number + 6), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel2input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel2input", nRegsPerBlock * number + 7), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel3input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel3input", nRegsPerBlock * number + 7), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel4input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel4input", nRegsPerBlock * number + 8), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel5input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel5input", nRegsPerBlock * number + 8), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel6input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel6input", nRegsPerBlock * number + 9), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel7input", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel7input", nRegsPerBlock * number + 9), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel0output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel0output", nRegsPerBlock * number + 10), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel1output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel1output", nRegsPerBlock * number + 10), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel2output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel2output", nRegsPerBlock * number + 11), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel3output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel3output", nRegsPerBlock * number + 11), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel4output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel4output", nRegsPerBlock * number + 12), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel5output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel5output", nRegsPerBlock * number + 12), 0b1111'0000}
      }},
      {fmt::format("{:02}.phaseSelectChannel6output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel6output", nRegsPerBlock * number + 13), 0b0000'1111}
      }},
      {fmt::format("{:02}.phaseSelectChannel7output", number), {
        TranslationUnitArt{fmt::format("{:02}.phaseSelectChannel7output", nRegsPerBlock * number + 13), 0b1111'0000}
      }},
      {fmt::format("{:02}.rxOff", number), {
        TranslationUnitArt{fmt::format("{:02}.rxOff", nFirstRxOff + number)}
      }},
      {fmt::format("{:02}.rxTermEnable", number), {
        TranslationUnitArt{fmt::format("{:02}.rxTermEnable", nFirstRxTermEnable + number)}
      }},
      // clang-format on
    };
  };
  TranslationMapArt table{
    // clang-format off
    {"dllLockedMask", {
      TranslationUnitArt{"60.dllLockedMask", 0b1111'0000}
    }},
    {"txcset", {
      TranslationUnitArt{"60.txcset", 0b0000'1111}
    }},
    {"resetAll", {
      TranslationUnitArt{"61.resetAll", 0b0010'0000}
    }},
    {"test_mux", {
      TranslationUnitArt{"61.test_mux", 0b0001'0000}
    }},
    {"rxOffSclk", {
      TranslationUnitArt{"61.rxOffSclk", 0b0000'1000}
    }},
    {"rxTermSclk", {
      TranslationUnitArt{"61.rxTermSclk", 0b0000'0100}
    }},
    {"rxTermClk_i", {
      TranslationUnitArt{"61.rxTermClk_i", 0b0000'0011}
    }}
    // clang-format on
  };
  for (auto i = 0; i < 4; i++) {
    table.merge(get(i));
  }
  return table;
}();

static const TranslationMapArt TRANSLATION_MAP_ART_CORE = {
  // clang-format off
  {"c_disable_arthist", {
    TranslationUnitArt{"00.c_disable_arthist", 0b1000'0000}
  }},
  {"c_bypass_pa", {
    TranslationUnitArt{"00.c_bypass_pa", 0b0100'0000}
  }},
  {"rxterm", {
    TranslationUnitArt{"00.rxterm", 0b0011'0000}
  }},
  {"txcset", {
    TranslationUnitArt{"00.txcset", 0b0000'1111}
  }},
  {"cfg_dout_time2", {
    TranslationUnitArt{"01.cfg_dout_time2", 0b1000'0000}
  }},
  {"cfg_bcr_sel", {
    TranslationUnitArt{"01.cfg_bcr_sel", 0b0100'0000}
  }},
  {"cfg_bcrout_sel", {
    TranslationUnitArt{"01.cfg_bcrout_sel", 0b0010'0000}
  }},
  {"cfg_art_revall", {
    TranslationUnitArt{"01.cfg_art_revall", 0b0001'0000}
  }},
  {"cfg_art_revbank", {
    TranslationUnitArt{"01.cfg_art_revbank", 0b0000'1111}
  }},
  {"cfg_dout_pattern", {
    TranslationUnitArt{"02.cfg_dout_pattern", 0b1000'0000}
  }},
  {"cfg_dout_artbypass", {
    TranslationUnitArt{"02.cfg_dout_artbypass", 0b0100'0000}
  }},
  {"cfg_dout_hitlist", {
    TranslationUnitArt{"02.cfg_dout_hitlist", 0b0010'0000}
  }},
  {"cfg_dout_artflags", {
    TranslationUnitArt{"02.cfg_dout_artflags", 0b0001'0000}
  }},
  {"cfg_dout_time", {
    TranslationUnitArt{"02.cfg_dout_time", 0b0000'1000}
  }},
  {"cfg_artmask", {
    TranslationUnitArt{"02.cfg_artmask", 0b0000'0111}
  }},
  {"cfg_deser_flagmask", {
    TranslationUnitArt{"03.cfg_deser_flagmask"}
  }},
  {"cfg_deser_flagpatt", {
    TranslationUnitArt{"04.cfg_deser_flagpatt"}
  }},
  {"cfg_din_invert", {
    TranslationUnitArt{"05.cfg_din_invert[7:0]", 0b1111'1111, 0x0000'00FF},
    TranslationUnitArt{"06.cfg_din_invert[15:8]", 0b1111'1111, 0x0000'FF00},
    TranslationUnitArt{"07.cfg_din_invert[23:16]", 0b1111'1111, 0x00FF'0000},
    TranslationUnitArt{"08.cfg_din_invert[31:24]", 0b1111'1111, 0xFF00'0000}
  }},
  {"cfg_din_mask", {
    TranslationUnitArt{"09.cfg_din_mask[7:0]", 0b1111'1111, 0x0000'00FF},
    TranslationUnitArt{"10.cfg_din_mask[15:8]", 0b1111'1111, 0x0000'FF00},
    TranslationUnitArt{"11.cfg_din_mask[23:16]", 0b1111'1111, 0x00FF'0000},
    TranslationUnitArt{"12.cfg_din_mask[31:24]", 0b1111'1111, 0xFF00'0000}
  }},
  {"cfg_bcid0", {
    TranslationUnitArt{"13.cfg_bcid0[7:0]", 0b1111'1111, 0x0000'00FF},
    TranslationUnitArt{"14.cfg_bcid0[11:8]", 0b0000'1111, 0x0000'0F00}
  }},
  {"cfg_bcid1", {
    TranslationUnitArt{"14.cfg_bcid1[3:0]", 0b1111'0000, 0x0000'000F},
    TranslationUnitArt{"15.cfg_bcid1[11:4]", 0b1111'1111, 0x0000'0FF0}
  }},
  {"cfg_artbypass_sel_ch0", {
    TranslationUnitArt{"16.cfg_artbypass_sel_ch0[4:0]", 0b0001'1111}
  }},
  {"cfg_artbypass_sel_ch1", {
    TranslationUnitArt{"16.cfg_artbypass_sel_ch1[2:0]", 0b1110'0000, 0b0000'0111},
    TranslationUnitArt{"17.cfg_artbypass_sel_ch1[4:3]", 0b0000'0011, 0b0001'1000}
  }},
  {"cfg_artbypass_sel_ch2", {
    TranslationUnitArt{"17.cfg_artbypass_sel_ch2[4:0]", 0b0111'1100}
  }},
  {"cfg_artbypass_sel_ch3", {
    TranslationUnitArt{"17.cfg_artbypass_sel_ch3[0]", 0b1000'0000, 0b0000'0001},
    TranslationUnitArt{"18.cfg_artbypass_sel_ch3[4:1]", 0b0000'1111, 0b0001'1110}
  }},
  {"cfg_artbypass_sel_ch4", {
    TranslationUnitArt{"18.cfg_artbypass_sel_ch4[3:0]", 0b1111'0000, 0b0000'1111},
    TranslationUnitArt{"19.cfg_artbypass_sel_ch4[4]", 0b0000'0001, 0b0001'0000}
  }},
  {"cfg_artbypass_sel_ch5", {
    TranslationUnitArt{"19.cfg_artbypass_sel_ch5[4:0]", 0b0011'1110}
  }},
  {"cfg_artbypass_sel_ch6", {
    TranslationUnitArt{"19.cfg_artbypass_sel_ch6[1:0]", 0b1100'0000, 0b0000'0011},
    TranslationUnitArt{"20.cfg_artbypass_sel_ch6[4:2]", 0b0000'0111, 0b0001'1100}
  }},
  {"cfg_artbypass_sel_ch7", {
    TranslationUnitArt{"20.cfg_artbypass_sel_ch7[4:0]", 0b1111'1000}
  }},
  {"cfg_pattern_data_phi0", {
    TranslationUnitArt{"21.cfg_pattern_data_phi0[7:0]", 0b1111'1111, 0x00FF},
    TranslationUnitArt{"22.cfg_pattern_data_phi0[13:8]", 0b0011'1111, 0x3F00}
  }},
  {"cfg_pattern_data_phi1", {
    TranslationUnitArt{"22.cfg_pattern_data_phi1[1:0]", 0b1100'0000, 0x0003},
    TranslationUnitArt{"23.cfg_pattern_data_phi1[9:2]", 0b1111'1111, 0x03FC},
    TranslationUnitArt{"24.cfg_pattern_data_phi1[13:10]", 0b0000'1111, 0x3C00}
  }},
  {"cfg_pattern_data_phi2", {
    TranslationUnitArt{"24.cfg_pattern_data_phi2[3:0]", 0b1111'0000, 0x000F},
    TranslationUnitArt{"25.cfg_pattern_data_phi2[11:4]", 0b1111'1111, 0x0FF0},
    TranslationUnitArt{"26.cfg_pattern_data_phi2[13:12]", 0b0000'0011, 0x3000}
  }},
  {"cfg_pattern_data_phi3", {
    TranslationUnitArt{"26.cfg_pattern_data_phi3[5:0]", 0b1111'1100, 0x003F},
    TranslationUnitArt{"27.cfg_pattern_data_phi3[13:6]", 0b1111'1111, 0x3FC0}
  }},
  {"cfg_pattern_data_phi4", {
    TranslationUnitArt{"28.cfg_pattern_data_phi4[7:0]", 0b1111'1111, 0x00FF},
    TranslationUnitArt{"29.cfg_pattern_data_phi4[13:8]", 0b0011'1111, 0x3F00}
  }},
  {"cfg_pattern_data_phi5", {
    TranslationUnitArt{"29.cfg_pattern_data_phi5[1:0]", 0b1100'0000, 0x0003},
    TranslationUnitArt{"30.cfg_pattern_data_phi5[9:2]", 0b1111'1111, 0x03FC},
    TranslationUnitArt{"31.cfg_pattern_data_phi5[13:10]", 0b0000'1111, 0x3C00}
  }},
  {"cfg_pattern_data_phi6", {
    TranslationUnitArt{"31.cfg_pattern_data_phi6[3:0]", 0b1111'0000, 0x000F},
    TranslationUnitArt{"32.cfg_pattern_data_phi6[11:4]", 0b1111'1111, 0x0FF0},
    TranslationUnitArt{"33.cfg_pattern_data_phi6[13:12]", 0b0000'0011, 0x3000}
  }},
  {"cfg_pattern_data_phi7", {
    TranslationUnitArt{"33.cfg_pattern_data_phi7[5:0]", 0b1111'1100, 0x003F},
    TranslationUnitArt{"34.cfg_pattern_data_phi7[13:6]", 0b1111'1111, 0x3FC0}
  }},
  // clang-format on
};

#endif
