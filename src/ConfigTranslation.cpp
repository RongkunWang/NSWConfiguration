#include "NSWConfiguration/ConfigTranslation.h"

#include "NSWConfiguration/ConfigTranslationMap.h"

#include <algorithm>

#include "boost/property_tree/ptree.hpp"

using boost::property_tree::ptree;

ConfigConverter::ConfigConverter(const ptree &t_config, const ConfigType t_type) : m_registerTree([this, t_type, &t_config]() {
                                                                                       if (t_type == ConfigType::REGISTER_BASED)
                                                                                       {
                                                                                           return t_config;
                                                                                       }
                                                                                       return convertNewToOld(t_config);
                                                                                   }()),
                                                                                   m_valueTree([this, t_type, &t_config]() {
                                                                                       if (t_type == ConfigType::VALUE_BASED)
                                                                                       {
                                                                                           return t_config;
                                                                                       }
                                                                                       return convertOldToNew(t_config);
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


void ConfigConverter::checkPaths(const std::vector<std::string>& t_paths) const
{
    if (not std::all_of(std::begin(t_paths), std::end(t_paths), [](const auto &t_element) {
            return TRANSLATION_MAP.find(t_element) != TRANSLATION_MAP.end();
        }))
    {
        throw std::runtime_error("Did not find all nodes in translation map");
    }
}


ptree ConfigConverter::convertNewToOld(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    ptree newTree;
    std::for_each(std::begin(allPaths), std::end(allPaths), [&t_config, &newTree](const auto &t_element) {
        const auto translationUnits = TRANSLATION_MAP.at(t_element);
        std::for_each(std::begin(translationUnits), std::end(translationUnits), [&t_config, &newTree, &t_element](const auto &t_unit) {
            // rotate to right by first least significant non-zero bit
            auto value = (t_unit.m_maskValue & t_config.get<unsigned int>(t_element)) >> __builtin_ctz(t_unit.m_maskValue);
            newTree.put(t_unit.m_registerName, value);
        });
    });

    return newTree;
}

ConfigConverter::TranslatedConfig ConfigConverter::convertNewToOldNoSubRegister(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);
    checkPaths(allPaths);

    const auto calcValue = [](const auto t_val, const auto t_maskRegister, const auto t_maskValue) {
        return (t_val & t_maskValue) >> __builtin_ctz(t_maskValue) << __builtin_ctz(t_maskRegister);
    };

    ptree newTree;
    std::map<std::string, unsigned int> mask;
    std::for_each(std::begin(allPaths), std::end(allPaths), [&mask, &t_config, &newTree, &calcValue](const auto &t_element) {
        const auto translationUnit = TRANSLATION_MAP.at(t_element);
        std::for_each(std::begin(translationUnit), std::end(translationUnit), [&mask, &t_config, &newTree, &t_element, &calcValue](const auto &t_unit) {
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

ptree ConfigConverter::convertOldToNew(const ptree &t_config) const
{
    const auto allPaths = getAllPaths(t_config);

    const auto findOldRegisterName = [](const std::string &t_name) {
        for (const auto &[newName, element] : TRANSLATION_MAP)
        {
            const auto item = std::find_if(std::begin(element), std::end(element), [&t_name](const auto &t_unit) {
                return t_name == t_unit.m_registerName;
            });
            if (item != std::end(element))
            {
                return std::make_pair(newName, *item);
            }
        }
        throw std::runtime_error("Did not find node in translation map");
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

[[nodiscard]] ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters(const nsw::I2cMasterConfig &t_config) const
{
    const auto tmp = convertNewToOldNoSubRegister(m_valueTree);
    auto config = tmp.m_ptree;
    const auto &masks = tmp.m_mask;
    const auto bitstreamMap = t_config.getBitstreamMap();

    for (const auto &[registerName, mask] : masks)
    {
        const auto registerSize = static_cast<int>(t_config.getTotalSize(registerName));
        const auto numberBitsSet = __builtin_popcount(mask);

        if (registerSize == numberBitsSet)
        {
            if (__builtin_ctz(~mask) != registerSize)
            {
                throw std::runtime_error("Number of bits match register size, but not all values are consecutive for register " + registerName);
            }
            continue;
        }

        config.put(registerName, config.get<unsigned int>(registerName) | (static_cast<unsigned int>(std::stoul(bitstreamMap.at(registerName), nullptr, 2)) & (~mask)));
    }

    return config;
}

[[nodiscard]] ptree ConfigConverter::getRegisterBasedConfigWithoutSubregisters(const std::string &t_opcIp,
                                                                               const std::string &t_scaAddress) const
{
    throw std::runtime_error("Not implemented");
}
