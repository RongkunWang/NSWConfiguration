

#ifndef NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
#define NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_

#include <map>
#include <vector>
#include <string>
#include <utility>

    using RegisterAndSize = std::pair<std::string, size_t>;

    // Ordered map of register names and sizes in bits
    using RegisterSizeVector = std::vector<RegisterAndSize>;
    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;

    static const AddressRegisterMap ROC_REGISTER_SIZE = {
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

#endif  // NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
