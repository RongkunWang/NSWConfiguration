#ifndef NSWCONFIGURATION_TP_I2CREGISTERMAPPINGS_H_
#define NSWCONFIGURATION_TP_I2CREGISTERMAPPINGS_H_

/*
 * ----------------------------------------------------------------------------------
 * -- Source auto-generated via jinja2 templates for use with wuppercodegen
 * --
 * -- Company:  The Weizmann Institute of Science
 * -- Engineer: Noam Inbari (noam.inbari@weizmann.ac.il)
 * --
 * -- Edits and review: Lawrence Lee (lawrence.lee.jr@cern.ch)
 * --
 * ----------------------------------------------------------------------------------
 */

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "NSWConfiguration/Types.h"


// -------------  NSW -------------
static const char TP_NAME_NSW[] ="NSW";
static const i2c::AddressRegisterMap TP_REGISTERS_NSW = {
    { "NSW_Firmware_version_time_stamp_0000_READONLY", { {"register", 32 } } },
    { "NSW_EndcapSector_ID_0001", { {"register", 32 } } },
    { "NSW_XL1ID_0002", { {"register", 8 } } },
};
static const std::vector<std::string> TP_REGISTERORDER_NSW = {
    "NSW_Firmware_version_time_stamp_0000_READONLY",
    "NSW_EndcapSector_ID_0001",
    "NSW_XL1ID_0002",
};

/*
 * Map-Pairing Register Files Arrays
 */
#define NUM_REGISTER_FILES 1

const i2c::AddressRegisterMap registerFilesArr[NUM_REGISTER_FILES] = {
    TP_REGISTERS_NSW,
};

const std::string registerFilesNamesArr[NUM_REGISTER_FILES] = {
    TP_NAME_NSW,
};

const std::vector<std::string> registerFilesOrderArr[NUM_REGISTER_FILES] = {
    TP_REGISTERORDER_NSW,
};

#endif // NSWCONFIGURATION_TP_I2CREGISTERMAPPINGS_H_
