#ifndef NSWCONFIGURATION_CONFIGCONVERTER_H
#define NSWCONFIGURATION_CONFIGCONVERTER_H

#include <map>
#include <string>
#include <type_traits>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/ConfigTranslationMap.h"

using boost::property_tree::ptree;


/** \brief Converts between register-based and value-based configurations
 */
class ConfigConverter
{
public:
    /** \brief Struct holding ptree and mask
     *  Conversion without sub-registers needs to save both the new ptree as well as
     *  keeping track of the parts (sub-registers) which were set.
     */
    struct TranslatedConfig
    {
        ptree m_ptree;                              ///< translated ptree
        std::map<std::string, unsigned int> m_mask; ///< map holding mask of values set in ptree
    };

    /** \brief Type of configuration
     */
    enum class ConfigType
    {
        REGISTER_BASED,
        VALUE_BASED
    };

    /** \brief Type of register Address space
     */
    enum class RegisterAddressSpace
    {
        ROC_ANALOG,
        ROC_DIGITAL,
        TDS,
        VMM
    };

    /** \brief Create object from ptree
     * Constructor to initialize object
     *  \param t_config configuration to be converted
     *  \param t_addressSpace ROC analog, digital,...
     *  \param t_type type of configuration (register or value-based)
     */
    explicit ConfigConverter(const ptree &t_config, const ConfigType t_type);
    explicit ConfigConverter(const ptree &t_config, const RegisterAddressSpace &t_addressSpace, const ConfigType t_type);

    /** \brief Obtain value-based ptree of configuration
     * The value-based ptree does not contain register values but configuration
     *  values. For example, clock phases are a single value and not split across
     *  registers. Values belonging to similar things, e.g. a single VMM, are grouped.
     *  \return value-based ptree
     */
    [[nodiscard]]
    ptree getValueBasedConfig() const;

    /** \brief Obtain register-based ptree of configuration with subregisters
     *  The register-based ptree with sub-registers corresponds to the traditional
     *  representation of the configuration. Every register contains named
     *  sub-registers with their corresponding value.
     *  \return register-based ptree
     */
    [[nodiscard]]
    ptree getRegisterBasedConfig() const;

    /** \brief Obtain register-based ptree of configuration without subregisters
     *  The register-based ptree without sub-registers has a single value per register.
     *  Missing values per register are read back from the hardware.
     *  \param t_opcIp IP of the OPC server
     *  \param t_scaAddress FIXME
     *  \return register-based ptree
     */
    [[nodiscard]]
    ptree getRegisterBasedConfigWithoutSubregisters(const std::string &t_opcIp,
                                                    const std::string &t_scaAddress) const;

    /** \brief Obtain register-based ptree of configuration without subregisters
     *  The register-based ptree without sub-registers has a single value per register.
     *  Missing values per register are read from the supplied ptree.
     *  \param t_config reference ptree for missing values
     *  \return register-based ptree
     */
    [[nodiscard]]
    ptree getRegisterBasedConfigWithoutSubregisters(const nsw::I2cMasterConfig &t_config) const;

private:
    /** \brief Convert register-based configuration ptree to value-based ptree
     *  The conversion in this direction is slower as it should not be used while configuring
     *
     *  1. Get all paths from ptree
     *  2. Iterate through paths of ptree and
     *     a) Find the correct entry in TRANSLATION_MAP
     *        (key is now TranslationUnit.m_RegisterName)
     *     b) Calculate value: Shift value in old ptree by number of trailing zeros in maskValue
     *        to the left.
     *     c) Check if register is already in new ptree
     *        yes: Add it to the value
     *        no:  Put it into the new ptree
     *  \param t_config register-based configuration
     *  \return value-based configuration
     */
    [[nodiscard]]
    ptree convertRegisterToValue(const ptree &t_config) const;

    /** \brief Convert value-based configuration ptree to register-based ptree
     *  The returned register-based ptree contains sub-registers.
     *
     *  1. Get all paths from input ptree
     *  2. Validate that all paths are in TRANSLATION_MAP
     *  3. Iterate through paths of input ptree and caluclate for each translation unit
     *     value & translationUnit.m_maskValue. This value needs to be shifted to the right by the
     *     number of trailing zeros of the mask. Put this value into TranslationUnit.m_RegisterName
     *     in new ptree. (TranslationUnit = TRANSLATION_MAP[path_input])
     *  \param t_config value-based configuration
     *  \return register-based configuration with sub-registers
     */
    [[nodiscard]]
    ptree convertValueToRegister(const ptree &t_config) const;

    /** \brief Convert value-based configuration ptree to register-based ptree
     *  The returned register-based ptree contains no sub-registers. The total mask per register keeps
     *  track which part of the register are set.
     *
     *  1. Get all paths from input ptree
     *  2. Validate that all paths are in TRANSLATION_MAP
     *  3. a) Iterate through paths of input ptree and caluclate for each translation unit
     *        value & translationUnit.m_maskValue. This value needs to be shifted to the right by the
     *        number of trailing zeros of the mask.
     *     b) Check if register (TranslationUnit.m_RegisterName) already exists in new ptree:
     *        yes: Add it to the existing value
     *        no:  Put this value into TranslationUnit.m_RegisterName into the new ptree
     *     c) Keep track of the total mask applied per register
     *  \param t_config value-based configuration
     *  \return register-based configuration without sub-registers and total mask per register
     */
    [[nodiscard]]
    TranslatedConfig convertValueToRegisterNoSubRegister(const ptree &t_config) const;

    /** \brief Get all paths in a ptree
     *  \param t_tree input ptree
     *  \return vector of all paths
     */
    [[nodiscard]]
    std::vector<std::string> getAllPaths(const ptree &t_tree) const;

    /** \brief Check that all paths exist in translation map
     *  \param t_paths vector of all paths in ptree
     *  \throw std::runtime_error not all paths present in translation map
     */
    void checkPaths(const std::vector<std::string>& t_paths) const;

    /** \brief Internal function to read back missing parts of registers
     *  Loops over converted configuration and calls t_func to obtain the reference value if a
     *  part of the register is missing.
     *  \tparam Func callable function parameter
     *  \param t_paths t_func callable which takes a string as parameter and returns an uint8_t
     *  \throw std::runtime_error not all paths present in translation map
     */
    template<typename Func>
    [[nodiscard]] ptree readMissingRegisterParts(const Func& t_func) const
    {
        // TODO: Concept when available
        static_assert(std::is_invocable_r<uint8_t, Func, std::string>::value, "Uh oh! the function is not invokable as we want it");
        const auto tmp = convertValueToFlatRegister(m_valueTree);
        auto config = tmp.m_ptree;
        const auto &masks = tmp.m_mask;

        for (const auto &[registerName, mask] : masks)
        {
            //const auto registerSize = static_cast<int>(t_config.getTotalSize(registerName));
            const int registerSize = 8; // FIXME
            const auto numberBitsSet = __builtin_popcount(mask);

            if (registerSize == numberBitsSet)
            {
                if (__builtin_ctz(~mask) != registerSize)
                {
                    throw std::runtime_error("Number of bits match register size, but not all values are consecutive for register " + registerName);
                }
                continue;
            }

            config.put(registerName, config.get<unsigned int>(registerName) | (t_func(registerName) & (~mask)));
        }

        return config;
    }

    TranslationMap m_translationMap;    ///< map used for translation (analog, digital, tds, ...)
    ptree m_registerTree;               ///< register-based ptree
    ptree m_valueTree;                  ///< value-based ptree
};

#endif
