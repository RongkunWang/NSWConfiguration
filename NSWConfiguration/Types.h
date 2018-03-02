// Definition of terminology
// address: Opc Address of a I2c element/register. In a I2c master, there can be several of these
// register: Name of subregister within the address. In an I2c element, there can be several of these.
//           The main registers under I2c (address) is defined in the OpcServer configuration.
//           The sub-registers are only used in the sw to build bitstreams and not known by Opc Server

#ifndef NSWCONFIGURATION_TYPES_H_
#define NSWCONFIGURATION_TYPES_H_

#include <map>
#include <vector>
#include <string>
#include <utility>

namespace i2c {
    using RegisterSizePair = std::pair<std::string, size_t>;

    using RegisterSizeVector = std::vector<RegisterSizePair>;

    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;

    using AddressSizeMap = std::map<std::string, size_t>;

    using AddressRegisterSizeMap = std::map<std::string, AddressSizeMap>;

    using AddressBitstreamMap = std::map<std::string, std::string>;
}  // namespace i2c

#endif  // NSWCONFIGURATION_TYPES_H_
