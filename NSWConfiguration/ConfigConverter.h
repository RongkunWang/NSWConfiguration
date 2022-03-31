#ifndef NSWCONFIGURATION_CONFIGCONVERTER_H
#define NSWCONFIGURATION_CONFIGCONVERTER_H

#include <map>
#include <numeric>
#include <string>
#include <type_traits>
#include <span>

#include "NSWConfiguration/I2cRegisterMappings.h"
#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/ConfigTranslationMap.h"
#include "NSWConfiguration/Utility.h"

namespace nsw {
  /**
   * \brief Used to infer the conversion map
   */
  enum class ConfigConversionType { ROC_ANALOG, ROC_DIGITAL, TDS, VMM, ART, ART_PS };

  /**
   * \brief Type of configuration
   */
  enum class ConfigType { REGISTER_BASED, VALUE_BASED };

  /**
   * \brief Type selector for the translation map (depends on device type)
   *
   * Defines translation map type and register size for each device type. Every device needs
   * a template specialization of this template (see below for examples or ask @joroemer in
   * case of doubt).
   *
   * \tparam ConfigConversionType Device type
   */
  template<ConfigConversionType>
  struct translationMapTypeSelector {};

  /**
   * \brief TDS 128 bit registers
   */
  template<>
  struct translationMapTypeSelector<ConfigConversionType::TDS> {
    using mapType = TranslationMapTds;
    using intType = __uint128_t;
  };

  /**
   * \brief ROC 8 bit registers
   */
  template<>
  struct translationMapTypeSelector<ConfigConversionType::ROC_ANALOG> {
    using mapType = TranslationMapRoc;
    using intType = std::uint32_t;
  };

  /**
   * \brief ROC 8 bit registers
   */
  template<>
  struct translationMapTypeSelector<ConfigConversionType::ROC_DIGITAL> {
    using mapType = TranslationMapRoc;
    using intType = std::uint32_t;
  };

  /**
   * \brief ART 8 bit registers
   */
  template<>
  struct translationMapTypeSelector<ConfigConversionType::ART> {
    using mapType = TranslationMapArt;
    using intType = std::uint32_t;
  };

  /**
   * \brief ART 8 bit registers
   */
  template<>
  struct translationMapTypeSelector<ConfigConversionType::ART_PS> {
    using mapType = TranslationMapArt;
    using intType = std::uint32_t;
  };

  // Do not touch those for new devices
  template<ConfigConversionType DEVICE>
  using translationMapType_t = typename translationMapTypeSelector<DEVICE>::mapType;

  template<ConfigConversionType DEVICE>
  using translationMapIntType_t = typename translationMapTypeSelector<DEVICE>::intType;

  /**
   * \brief Converts between register-based and value-based configurations
   *
   * \tparam DeviceType Device type
   */
  template<ConfigConversionType DeviceType>
  class ConfigConverter
  {
    /**
     * \brief Struct holding ptree and mask
     *
     * Conversion without sub-registers needs to save both the new ptree as well as
     * keeping track of the parts (sub-registers) which were set.
     */
    struct TranslatedConfig {
      boost::property_tree::ptree m_ptree;  ///< translated ptree
      std::map<std::string, translationMapIntType_t<DeviceType>>
        m_mask;  ///< map holding mask of values set in ptree
    };

  public:
    /**
     * \brief Constructor
     *
     * \param t_config configuration to be converted
     * \param t_type type of configuration (register or value-based)
     */
    explicit ConfigConverter(const boost::property_tree::ptree& t_config, ConfigType t_type);

    /**
     * \brief Obtain value-based ptree of configuration
     *
     * The value-based ptree does not contain register values but configuration
     * values. For example, clock phases are a single value and not split across
     * registers. Values belonging to similar things, e.g. a single VMM, are grouped.
     *
     * \return value-based ptree
     */
    [[nodiscard]] boost::property_tree::ptree getValueBasedConfig() const;

    /**
     * \brief Obtain register-based ptree of configuration with subregisters
     *
     * The register-based ptree with sub-registers corresponds to the traditional
     * representation of the configuration. Every register contains named
     * sub-registers with their corresponding value.
     *
     * \return register-based ptree
     */
    [[nodiscard]] boost::property_tree::ptree getSubRegisterBasedConfig() const;

    /**
     * \brief Obtain register-based ptree of configuration without subregisters
     *
     * The register-based ptree without sub-registers has a single value per register.
     * Missing values per register are read back from the hardware.
     *
     * \param t_opcIp IP of the OPC server
     * \param t_scaAddress SCA address of FE item in Opc address space
     * \return register-based ptree
     */
    [[nodiscard]] boost::property_tree::ptree getFlatRegisterBasedConfig(
      const std::string& t_opcIp,
      const std::string& t_scaAddress) const;

    /**
     * \brief Obtain register-based ptree of configuration without subregisters
     *
     * The register-based ptree without sub-registers has a single value per register.
     * Missing values per register are read from the supplied ptree.
     *
     * \param t_reference reference bitstream map
     * \return register-based ptree
     */
    [[nodiscard]] boost::property_tree::ptree getFlatRegisterBasedConfig(
      const i2c::AddressBitstreamMap& t_reference) const;

    /**
     * \brief Get all registers for a value
     *
     * \param name Name of a value
     * \return std::unordered_set<std::string> Set of all registers
     */
    static std::unordered_set<std::string> getRegsForValue(const std::string& name);

    /**
     * \brief Convert register-based configuration ptree to a subregister-based ptree
     *
     * 1. Find all entries for registers in translation maps
     * 2. Filter the subresgisters based on t_values (only subregisters contributing to
     *    the requested values are put into the output tree)
     *
     * \param t_config register-based configuration
     * \param t_values range of reference values
     * \return value-based configuration
     */
    [[nodiscard]] static std::map<std::string, translationMapIntType_t<DeviceType>>
    convertRegisterToSubRegister(const boost::property_tree::ptree& t_config,
                                 std::span<const std::string> t_values);

  private:
    /**
     * \brief Convert register-based configuration ptree to value-based ptree
     *
     * The conversion in this direction is slower as it should not be used while configuring
     * 1. Get all paths from ptree
     * 2. Iterate through paths of ptree and
     *    a) Find the correct entry in TRANSLATION_MAP
     *       (key is now TranslationUnit.m_RegisterName)
     *    b) Calculate value: Shift value in old ptree by number of trailing zeros in maskValue
     *       to the left.
     *    c) Check if register is already in new ptree
     *       yes: Add it to the value
     *       no:  Put it into the new ptree
     *
     * \param t_config register-based configuration
     * \return value-based configuration
     */
    [[nodiscard]] boost::property_tree::ptree convertSubRegisterToValue(
      const boost::property_tree::ptree& t_config) const;

    /**
     * \brief Convert value-based configuration ptree to register-based ptree
     *
     * The returned register-based ptree contains sub-registers.
     * 1. Get all paths from input ptree
     * 2. Validate that all paths are in TRANSLATION_MAP
     * 3. Iterate through paths of input ptree and caluclate for each translation unit
     *    value & translationUnit.m_maskValue. This value needs to be shifted to the right by the
     *    number of trailing zeros of the mask. Put this value into TranslationUnit.m_RegisterName
     *    in new ptree. (TranslationUnit = TRANSLATION_MAP[path_input])
     *
     * \param t_config value-based configuration
     * \return register-based configuration with sub-registers
     */
    [[nodiscard]] boost::property_tree::ptree convertValueToSubRegister(
      const boost::property_tree::ptree& t_config) const;

    /**
     * \brief Convert value-based configuration ptree to register-based ptree
     *
     * The returned register-based ptree contains no sub-registers. The total mask per register
     * keeps track which part of the register are set.
     * 1. Get all paths from input ptree
     * 2. Validate that all paths are in TRANSLATION_MAP
     * 3. a) Iterate through paths of input ptree and caluclate for each translation unit
     *       value & translationUnit.m_maskValue. This value needs to be shifted to the right by
     * the number of trailing zeros of the mask. b) Check if register
     * (TranslationUnit.m_RegisterName) already exists in new ptree: yes: Add it to the existing
     * value no:  Put this value into TranslationUnit.m_RegisterName into the new ptree c) Keep
     * track of the total mask applied per register \param t_config value-based configuration
     *
     * \return register-based configuration without sub-registers and total mask per register
     */
    [[nodiscard]] TranslatedConfig convertValueToFlatRegister(
      const boost::property_tree::ptree& t_config) const;

    /**
     * \brief Get all paths in a ptree
     *
     * \param t_tree input ptree
     * \return vector of all paths
     */
    [[nodiscard]] static std::vector<std::string> getAllPaths(
      const boost::property_tree::ptree& t_tree);

    /**
     * \brief Check that all paths exist in translation map
     *
     * \param t_paths vector of all paths in ptree
     * \throw std::runtime_error not all paths present in translation map
     */
    void checkPaths(const std::vector<std::string>& t_paths) const;

    /**
     * \brief Internal function to read back missing parts of registers
     *
     * Loops over converted configuration and calls t_func to obtain the reference value if a
     * part of the register is missing.
     *
     * \tparam Func callable function parameter
     * \param t_func callable which takes a string as parameter and returns an uint8_t
     * \throw std::runtime_error not all paths present in translation map
     */
    template<typename Func>
    [[nodiscard]] boost::property_tree::ptree readMissingRegisterParts(const Func& t_func) const
    {
      // TODO: Concept when available
      // FIXME: return type of Func
      static_assert(std::is_invocable_r<uint8_t, Func, std::string>::value,
                    "Uh oh! the function is not invokable as we want it");
      const TranslatedConfig tmp = convertValueToFlatRegister(m_valueTree);
      auto config = tmp.m_ptree;
      const auto& masks = tmp.m_mask;

      for (const auto& [registerName, mask] : masks) {
        const auto numberBitsSet = popcount(mask);

        const auto registerSize = std::accumulate(
          std::begin(REGISTER_MAPPINGS.at(DeviceType).at(registerName)),
          std::end(REGISTER_MAPPINGS.at(DeviceType).at(registerName)),
          0,
          [](const size_t sum, const i2c::RegisterSizePair& p) { return sum + p.second; });

        if (static_cast<int>(registerSize) == numberBitsSet) {
          if (ctz(~mask) != static_cast<int>(registerSize)) {
            throw std::runtime_error("Number of bits match register size, but not all values are "
                                     "consecutive for register " +
                                     registerName);
          }
          continue;
        }

        config.put(registerName,
                   config.template get<translationMapIntType_t<DeviceType>>(registerName) |
                     (t_func(registerName) & (~mask)));
      }

      return config;
    }

    /**
     * \brief Generic ctz implementation
     *
     * Returns the number of trailing 0-bits in x, starting at the least significant bit position.
     * If x is 0, the result is undefined.
     *
     * \param t_val integer value
     * \return int number of trailing zeros
     */
    static int ctz(translationMapIntType_t<DeviceType> t_val);

    /**
     * \brief Generic popcount implementiation
     *
     * Returns the number of 1-bits in x.
     *
     * \param t_val integer value
     * \return int number of 1 bits
     */
    static int popcount(translationMapIntType_t<DeviceType> t_val);

    /**
     * \brief Converts a binary string into an int
     *
     * \param t_string string of zeros and ones
     * \return translationMapIntType_t<DeviceType> integer
     */
    static translationMapIntType_t<DeviceType> binaryStringToInt(const std::string& t_string);

    /**
     * \brief Get the translation map for each \ref DeviceType
     *
     * \return translationMapType_t<DeviceType> translation map
     */
    static const translationMapType_t<DeviceType>& getTranslationMap();

    translationMapType_t<DeviceType>
      m_translationMap;  ///< map used for translation (analog, digital, tds, ...)
    boost::property_tree::ptree m_registerTree;  ///< register-based ptree
    boost::property_tree::ptree m_valueTree;     ///< value-based ptree

    static const std::unordered_map<ConfigConversionType, i2c::AddressRegisterMap>
      REGISTER_MAPPINGS;
  };

}  // namespace nsw

#endif
