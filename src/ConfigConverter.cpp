#include "NSWConfiguration/ConfigConverter.h"

#include "NSWConfiguration/ConfigTranslationMap.h"
#include "NSWConfiguration/ConfigSender.h"

#include <algorithm>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

ConfigConverter::ConfigConverter(const ptree &t_config, const RegisterAddressSpace &t_addressSpace, const ConfigType t_type) : m_translationMap([this, t_addressSpace] () {
                                                                                                                                   if (t_addressSpace == RegisterAddressSpace::ROC_ANALOG)
                                                                                                                                   {
                                                                                                                                       return TRANSLATION_MAP_ROC_ANALOG;
                                                                                                                                   }
                                                                                                                                   if (t_addressSpace == RegisterAddressSpace::ROC_DIGITAL)
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
                                                                                                                                   return convertValueToRegister(t_config);
                                                                                                                               }()),
                                                                                                                               m_valueTree([this, t_type, &t_config]() {
                                                                                                                                   if (t_type == ConfigType::VALUE_BASED)
                                                                                                                                   {
                                                                                                                                       return t_config;
                                                                                                                                   }
                                                                                                                                   return convertRegisterToValue(t_config);
                                                                                                                               }())
{
}

std::vector<std::string> ConfigConverter::getAllPaths(const ptree &t_tree) const
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

void ConfigConverter::checkPaths(const std::vector<std::string> &t_paths) const
{
    if (not std::all_of(std::begin(t_paths), std::end(t_paths), [this](const auto &t_element) {
            return m_translationMap.find(t_element) != m_translationMap.end();
        }))
    {
        throw std::runtime_error("Did not find all nodes in translation map");
    }
}

ptree ConfigConverter::convertValueToRegister(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    ptree newTree;
    std::for_each(std::begin(allPaths), std::end(allPaths), [this, &t_config, &newTree](const auto &t_element) {
        const auto translationUnits = m_translationMap.at(t_element);
        std::for_each(std::begin(translationUnits), std::end(translationUnits), [&t_config, &newTree, &t_element](const auto &t_unit) {
            // rotate to right by first least significant non-zero bit
            auto value = (t_unit.m_maskValue & t_config.get<unsigned int>(t_element)) >> __builtin_ctz(t_unit.m_maskValue);
            newTree.put(t_unit.m_registerName, value);
        });
    });

    return newTree;
}

ConfigConverter::TranslatedConfig ConfigConverter::convertValueToRegisterNoSubRegister(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    const auto calcValue = [](const auto t_val, const auto t_maskRegister, const auto t_maskValue) {
        return ((t_val & t_maskValue) >> __builtin_ctz(t_maskValue)) << __builtin_ctz(t_maskRegister);
    };

    ptree newTree;
    std::map<std::string, unsigned int> mask;
    std::for_each(std::begin(allPaths), std::end(allPaths), [this, &mask, &t_config, &newTree, &calcValue](const auto &t_element) {
        const auto translationUnit = m_translationMap.at(t_element);
        std::for_each(std::begin(translationUnit), std::end(translationUnit), [this, &mask, &t_config, &newTree, &t_element, &calcValue](const auto &t_unit) {
            const auto value = calcValue(t_config.get<unsigned int>(t_element), t_unit.m_maskRegister, t_unit.m_maskValue);
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

ptree ConfigConverter::convertRegisterToValue(const ptree &t_config) const
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
        return t_val << __builtin_ctz(t_maskValue);
    };

    ptree newTree;
    std::for_each(std::begin(allPaths), std::end(allPaths), [&t_config, &newTree, &calcValue, &findOldRegisterName](const auto &t_element) {
        const auto tmp = findOldRegisterName(t_element);
        const auto newName = tmp.first;
        const auto unit = tmp.second;
        const auto value = calcValue(t_config.get<unsigned int>(t_element), unit.m_maskValue);
        newTree.put(newName, newTree.get(newName, 0u) + value);
    });

    return newTree;
}

[[nodiscard]] ptree ConfigConverter::getValueBasedConfig() const
{
    return m_valueTree;
}

[[nodiscard]] ptree ConfigConverter::getRegisterBasedConfig() const
{
    return m_registerTree;
}

[[nodiscard]] ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters(const nsw::I2cMasterConfig &t_config) const
{
    const auto bitstreamMap = t_config.getBitstreamMap();
    const auto func = [&bitstreamMap] (const std::string& t_registerName) {
        return static_cast<unsigned int>(std::stoul(bitstreamMap.at(t_registerName), nullptr, 2));
    };

    const auto registerSize = t_config.getTotalSize(bitstreamMap.begin()->first);

    return readMissingRegisterParts(func, registerSize);
}

template<ConfigConverter::RegisterAddressSpace DeviceType>
[[nodiscard]] ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters(const std::string &t_opcIp,
                                                                               const std::string &t_scaAddress) const
{
    nsw::ConfigSender configSender;
    const auto func = [&t_opcIp, &t_scaAddress, &configSender] (const std::string& t_registerName) {
        const auto regNumber = std::stoi(t_registerName.substr(3, 3));

        if (DeviceType == RegisterAddressSpace::ROC_DIGITAL)
        {
            return configSender.readBackRocDigital(t_opcIp, t_scaAddress, static_cast<uint8_t>(regNumber));
        }
        if (DeviceType == RegisterAddressSpace::ROC_ANALOG)
        {
            return configSender.readBackRocAnalog(t_opcIp, t_scaAddress, static_cast<uint8_t>(regNumber));
        }
        throw std::runtime_error("Only translation of ROC_DIGITAL and ROC_ANALOG are implemented for now.");
    };

    return readMissingRegisterParts(func, m_registerSizeMapping.at(DeviceType));
}

template ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters<ConfigConverter::RegisterAddressSpace::ROC_ANALOG>(const std::string&, const std::string&) const;
template ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters<ConfigConverter::RegisterAddressSpace::ROC_DIGITAL>(const std::string&, const std::string&) const;
