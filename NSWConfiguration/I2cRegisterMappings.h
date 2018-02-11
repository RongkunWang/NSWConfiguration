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


static const i2c::AddressRegisterMap ROC_ANALOG_REGISTER_SIZE = {
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
