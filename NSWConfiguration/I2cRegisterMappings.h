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

static const i2c::AddressRegisterMap ROC_DIGITAL_REGISTERS = {
    { "SROCEnableReg7", {{"Bypass", 1}, {"TimeoutEnable", 1}, {"TTCStartBits", 2},
                         {"enableSROC3", 1}, {"enableSROC2", 1}, {"enableSROC1", 1}, {"enableSROC0", 1}}}
};

#endif  // NSWCONFIGURATION_I2CREGISTERMAPPINGS_H_
