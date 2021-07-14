#include "NSWConfiguration/ConfigConverter.h"

#include "NSWConfiguration/ConfigTranslationMap.h"
#include "NSWConfiguration/ConfigSender.h"

#include <algorithm>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

namespace nsw {
template<ConfigConversionType DeviceType>
const std::map<ConfigConversionType, i2c::AddressRegisterMap>
  ConfigConverter<DeviceType>::REGISTER_MAPPINGS = {
    {ConfigConversionType::ROC_DIGITAL, ROC_DIGITAL_REGISTERS},
    {ConfigConversionType::ROC_ANALOG, ROC_ANALOG_REGISTERS},
    {ConfigConversionType::TDS, TDS_REGISTERS},
};

template<ConfigConversionType DeviceType>
ConfigConverter<DeviceType>::ConfigConverter(const ptree &t_config, const ConfigType t_type) :
    m_translationMap([] () {
       if (DeviceType == ConfigConversionType::ROC_ANALOG)
       {
           return TRANSLATION_MAP_ROC_ANALOG;
       }
       if (DeviceType == ConfigConversionType::ROC_DIGITAL)
       {
           return TRANSLATION_MAP_ROC_DIGITAL;
       }
       throw std::runtime_error("Not implemented");
   }()),
   m_registerTree([this, t_type, &t_config]() {
       if (t_type == ConfigType::REGISTER_BASED)
       {
           return t_config;
       }
       return convertValueToSubRegister(t_config);
   }()),
   m_valueTree([this, t_type, &t_config]() {
       if (t_type == ConfigType::VALUE_BASED)
       {
           return t_config;
       }
       return convertSubRegisterToValue(t_config);
   }())
{
}

template<>
ConfigConverter<ConfigConversionType::TDS>::ConfigConverter(const ptree &t_config, const ConfigType t_type) :
    m_translationMap(TRANSLATION_MAP_TDS),
   m_registerTree([this, t_type, &t_config]() {
       if (t_type == ConfigType::REGISTER_BASED)
       {
           return t_config;
       }
       return convertValueToSubRegister(t_config);
   }()),
   m_valueTree([this, t_type, &t_config]() {
       if (t_type == ConfigType::VALUE_BASED)
       {
           return t_config;
       }
       return convertSubRegisterToValue(t_config);
   }())
{
}

template<ConfigConversionType DeviceType>
std::vector<std::string> ConfigConverter<DeviceType>::getAllPaths(const ptree &t_tree) const
{
    const auto iterate = [](const ptree &t_tree) {
        std::vector<std::string> paths;
        const auto iterate_impl = [&paths](const auto func, const ptree &t_tree, const std::string &t_path = "") -> void {
            if (t_tree.empty())
            {
                paths.push_back(t_path);
            }

            for (const auto &child : t_tree)
            {
                if (t_path.empty())
                {
                    func(func, child.second, child.first);
                }
                else
                {
                    func(func, child.second, t_path + "." + child.first);
                }
            }
        };
        iterate_impl(iterate_impl, t_tree);
        return paths;
    };
    return iterate(t_tree);
}

template<ConfigConversionType DeviceType>
void ConfigConverter<DeviceType>::checkPaths(const std::vector<std::string> &t_paths) const
{
    if (not std::all_of(std::begin(t_paths), std::end(t_paths), [this](const auto &t_element) {
            return m_translationMap.find(t_element) != m_translationMap.end();
        }))
    {
        throw std::runtime_error("Did not find all nodes in translation map");
    }
}

template<ConfigConversionType DeviceType>
ptree ConfigConverter<DeviceType>::convertValueToSubRegister(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    ptree newTree;
    std::for_each(std::begin(allPaths), std::end(allPaths), [this, &t_config, &newTree](const auto &t_element) {
        const auto translationUnits = m_translationMap.at(t_element);
        std::for_each(std::begin(translationUnits), std::end(translationUnits), [&t_config, &newTree, &t_element](const auto &t_unit) {
            // rotate to right by first least significant non-zero bit
            auto value = (t_unit.m_maskValue & t_config.get<translationMapIntType_t<DeviceType>>(t_element)) >> ctz(t_unit.m_maskValue);
            newTree.put(t_unit.m_registerName, value);
        });
    });

    return newTree;
}

template<ConfigConversionType DeviceType>
typename ConfigConverter<DeviceType>::TranslatedConfig ConfigConverter<DeviceType>::convertValueToFlatRegister(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    const auto calcValue = [](const auto t_val, const auto t_maskRegister, const auto t_maskValue) {
        return ((t_val & t_maskValue) >> ctz(t_maskValue)) << ctz(t_maskRegister);
    };

    ptree newTree;
    std::map<std::string, translationMapIntType_t<DeviceType>> mask;
    std::for_each(std::begin(allPaths), std::end(allPaths), [this, &mask, &t_config, &newTree, &calcValue](const auto &t_element) {
        const auto translationUnit = m_translationMap.at(t_element);
        std::for_each(std::begin(translationUnit), std::end(translationUnit), [this, &mask, &t_config, &newTree, &t_element, &calcValue](const auto &t_unit) {
            const auto value = calcValue(t_config.get<translationMapIntType_t<DeviceType>>(t_element), t_unit.m_maskRegister, t_unit.m_maskValue);
            const auto name = t_unit.m_registerName.substr(0, t_unit.m_registerName.find('.'));
            newTree.put(name, newTree.get(name, 0u) + value);
            if (mask.find(name) != mask.end())
            {
                mask[name] = mask.at(name) | t_unit.m_maskRegister;
            }
            else
            {
                mask[name] = t_unit.m_maskRegister;
            }
        });
    });

    return {newTree, mask};
}

template<ConfigConversionType DeviceType>
ptree ConfigConverter<DeviceType>::convertSubRegisterToValue(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);

    const auto findOldRegisterName = [this](const std::string &t_name) {
        for (const auto &[newName, element] : m_translationMap)
        {
            const auto item = std::find_if(std::begin(element), std::end(element), [&t_name](const auto &t_unit) {
                return t_name == t_unit.m_registerName;
            });
            if (item != std::end(element))
            {
                return std::make_pair(newName, *item);
            }
        }
        throw std::runtime_error("Did not find node " + t_name + " in translation map");
    };

    const auto calcValue = [](const auto t_val, const auto t_maskValue) {
        return t_val << ctz(t_maskValue);
    };

    ptree newTree;
    std::for_each(std::begin(allPaths), std::end(allPaths), [&t_config, &newTree, &calcValue, &findOldRegisterName](const auto &t_element) {
        const auto tmp = findOldRegisterName(t_element);
        const auto newName = tmp.first;
        const auto unit = tmp.second;
        const auto value = calcValue(t_config.get<translationMapIntType_t<DeviceType>>(t_element), unit.m_maskValue);
        newTree.put(newName, newTree.get(newName, 0u) + value);
    });

    return newTree;
}

template<ConfigConversionType DeviceType>
[[nodiscard]] ptree ConfigConverter<DeviceType>::getValueBasedConfig() const
{
    return m_valueTree;
}

template<ConfigConversionType DeviceType>
[[nodiscard]] ptree ConfigConverter<DeviceType>::getSubRegisterBasedConfig() const
{
    return m_registerTree;
}

template<ConfigConversionType DeviceType>
[[nodiscard]] ptree ConfigConverter<DeviceType>::getFlatRegisterBasedConfig(const nsw::I2cMasterConfig &t_config) const
{
    const auto bitstreamMap = t_config.getBitstreamMap();
    const auto func = [&bitstreamMap] (const std::string& t_registerName) {
        return binaryStringToInt(bitstreamMap.at(t_registerName));
    };

    return readMissingRegisterParts(func);
}

template<ConfigConversionType DeviceType>
[[nodiscard]] ptree ConfigConverter<DeviceType>::getFlatRegisterBasedConfig(const std::string &t_opcIp,
                                                                const std::string &t_scaAddress) const
{
    nsw::ConfigSender configSender;
    const auto func = [&t_opcIp, &t_scaAddress, &configSender] (const std::string& t_registerName) {
        const auto regNumber = std::stoi(t_registerName.substr(3, 3));

        if (DeviceType == ConfigConversionType::ROC_DIGITAL)
        {
            return configSender.readBackRocDigital(t_opcIp, t_scaAddress, static_cast<uint8_t>(regNumber));
        }
        if (DeviceType == ConfigConversionType::ROC_ANALOG)
        {
            return configSender.readBackRocAnalog(t_opcIp, t_scaAddress, static_cast<uint8_t>(regNumber));
        }
        throw std::runtime_error("Only translation of ROC_DIGITAL and ROC_ANALOG are implemented for now.");
    };

    return readMissingRegisterParts(func);
}

template<ConfigConversionType DeviceType>
int ConfigConverter<DeviceType>::ctz(const translationMapIntType_t<DeviceType> t_val) {
    return __builtin_ctz(t_val);
}

template<>
int ConfigConverter<ConfigConversionType::TDS>::ctz(const translationMapIntType_t<ConfigConversionType::TDS> t_val) {
    const auto trailing = static_cast<std::uint64_t>(t_val);
    auto result = __builtin_ctzll(trailing);
    constexpr auto SIZE_LEADING = 64;
    if (result != SIZE_LEADING) {
        return result;
    }
    const std::uint64_t leading = t_val >> static_cast<unsigned int>(SIZE_LEADING);
    return SIZE_LEADING + __builtin_ctzll(leading);
}

template<ConfigConversionType DeviceType>
int ConfigConverter<DeviceType>::popcount(const translationMapIntType_t<DeviceType> t_val) {
    return __builtin_popcount(t_val);
}

template<>
int ConfigConverter<ConfigConversionType::TDS>::popcount(const translationMapIntType_t<ConfigConversionType::TDS> t_val) {
    const auto trailing = static_cast<std::uint64_t>(t_val);
    const auto leading = static_cast<std::uint64_t>(t_val >> 64U);
    return __builtin_popcountll(leading) + __builtin_popcountll(trailing);
}

template<ConfigConversionType DeviceType>
translationMapIntType_t<DeviceType> ConfigConverter<DeviceType>::binaryStringToInt(const std::string& t_string) {
    return std::stoul(t_string, nullptr, 2);
}

template<>
translationMapIntType_t<ConfigConversionType::TDS> ConfigConverter<ConfigConversionType::TDS>::binaryStringToInt(const std::string& t_string) {
    if (t_string.size() > 128U) {
        throw std::runtime_error("String longer that 128. Cannot convert into a 128bit integer.");
    }
    if (t_string.size() <= 64U) {
        return static_cast<__uint128_t>(std::stoul(t_string, nullptr, 2));
    }
    const std::string leading = t_string.substr(0, 64);
    const std::string trailing = t_string.substr(64);
    return (static_cast<__uint128_t>(std::stoul(leading, nullptr, 2)) << 64U) + std::stoul(trailing, nullptr, 2);
}

template class ConfigConverter<ConfigConversionType::ROC_ANALOG>;
template class ConfigConverter<ConfigConversionType::ROC_DIGITAL>;
template class ConfigConverter<ConfigConversionType::TDS>;

} // namespace nsw
