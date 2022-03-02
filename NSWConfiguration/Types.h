// Definition of terminology
// address: Opc Address of a I2c element/register. In a I2c master, there can be several of these
// register: Name of subregister within the address. In an I2c element, there can be several of these.
//           The main registers under I2c (address) is defined in the OpcServer configuration.
//           The sub-registers are only used in the sw to build bitstreams and not known by Opc Server

#ifndef NSWCONFIGURATION_TYPES_H_
#define NSWCONFIGURATION_TYPES_H_

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>

#include <boost/property_tree/ptree.hpp>
namespace i2c {
    using RegisterSizePair = std::pair<std::string, size_t>;

    using RegisterSizeVector = std::vector<RegisterSizePair>;

    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;

    using AddressSizeMap = std::map<std::string, size_t>;

    using AddressRegisterSizeMap = std::map<std::string, AddressSizeMap>;

    using AddressBitstreamMap = std::map<std::string, std::string>;
}  // namespace i2c

namespace gbtx {

    struct regMap {
        std::string name;
        std::string description;
        std::vector<std::size_t> shifts;
        std::vector<std::size_t> registers;
    };

}

namespace nsw {
    using DeviceMap = std::unordered_map<
        std::string,
        std::unordered_map<std::string, boost::property_tree::ptree>>;

    struct GBTxSingleConfig{
        std::string iPath;
        std::string opcServerIp;
        std::string opcNodeId;
        std::uint64_t fid_toflx;
        std::uint64_t fid_tohost;
        std::string boardType;
        bool trainGBTxPhaseAlignment;
        int trainGBTxPhaseWaitTime;
    };

} // namespace nsw

#endif  // NSWCONFIGURATION_TYPES_H_
