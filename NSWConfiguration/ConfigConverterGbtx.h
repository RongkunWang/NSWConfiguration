#ifndef NSWCONFIGURATION_CONFIGCONVERTERGBTX_H
#define NSWCONFIGURATION_CONFIGCONVERTERGBTX_H

#include <NSWConfiguration/Types.h>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <unordered_set>

#include "NSWConfiguration/GBTxConfig.h"

namespace nsw::gbtx {
  namespace internal {
    struct RegisterValue {
      std::uint8_t val{};
      std::uint8_t mask{};
    };

    struct RegisterSpacePart {
      std::uint16_t reg{};
      RegisterValue val{};
    };
  }  // namespace internal

  /**
   * @brief Get the register IDs corresponding to a value
   *
   * @param name Name of the value
   * @return std::unordered_set<std::uint16_t> Set of register IDs
   */
  [[nodiscard]] std::unordered_set<std::uint16_t> getRegsForValue(const std::string& name);

  /**
   * @brief Get the register IDs corresponding to a range of values
   *
   * @param name Name of the values
   * @return std::unordered_set<std::uint16_t> Set of register IDs
   */
  [[nodiscard]] std::unordered_set<std::uint16_t> getRegsForValues(
    const std::ranges::viewable_range auto&& names)
  {
    std::unordered_set<std::uint16_t> result{};
    for (const auto& name : names) {
      result.merge(getRegsForValue(name));
    }
    return result;
  }

  /**
   * @brief Convert values to registers
   *
   * This function can be used if one wants to write a set of values to the hardware. It looks up
   * which registers the values correspond to and reads back missing parts of registers from a
   * reference config object. It then returns a map of registers to values of those registers which
   * can be directly written to the hardware.
   *
   * @param values Values to be converted
   * @param reference Reference if values only define parts of regs
   * @return std::map<std::uint16_t, std::uint8_t> Register view of values
   */
  [[nodiscard]] std::map<std::uint16_t, std::uint8_t> convertValuesToRegisters(
    const std::map<std::string, std::uint32_t>& values,
    const GBTxConfig& reference);

  /**
   * @brief Convert registers to values
   *
   * This function can be used if one wants to read a set of values to the hardware. It checks that
   * all necessary registers are provided for the requested values. If a value is written into
   * multiple registers it checks that all registers contain the same value.
   *
   * @param registers
   * @param names
   * @return std::map<std::string, std::uint32_t>
   */
  [[nodiscard]] std::map<std::string, std::uint32_t> convertRegistersToValues(
    const std::map<std::uint16_t, std::uint8_t>& registers,
    std::span<const std::string> names);

  /**
   * @brief Get the holes which need to be taken from a reference
   *
   * When converting values to registers some parts of registers need to be taken from a reference.
   * This function determines where those holes are.
   *
   * @param values Map of values
   * @return std::vector<internal::RegisterSpacePart> Vector of holes
   */
  [[nodiscard]] std::vector<internal::RegisterSpacePart> getHoles(
    const std::map<std::string, std::uint32_t>& values);

  namespace internal {

    /**
     * @brief Get the converted values with a mask indicating holes
     *
     * When converting values to registers some parts of registers need to be taken from a
     * reference.
     *
     * @param values Map of values
     * @return std::vector<internal::RegisterSpacePart> Converted registers with holes
     */
    [[nodiscard]] std::map<std::uint16_t, RegisterValue> convertValuesWithHoles(
      const std::map<std::string, std::uint32_t>& values);

    /**
     * @brief Convert a single value to a register
     *
     * The result indicates value and part of the register which is covered. One value can translate
     * itself into multiple registers
     *
     * @throws std::logic_error if someone fucked up the register map
     * @param name Name of the value
     * @param value Value
     * @return std::vector<RegisterSpacePart> Register representation of the value
     */
    [[nodiscard]] std::vector<RegisterSpacePart> convertValueToRegisterSpace(
      const std::string& name,
      std::uint32_t value);

    /**
     * @brief Fill holes using the reference config object
     *
     * @param reg Register address
     * @param value Value and covered parts of the register
     * @param reference Reference config object
     * @return std::uint8_t Full register
     */
    [[nodiscard]] std::uint8_t superimposeReference(std::uint16_t reg,
                                                    const RegisterValue& value,
                                                    const GBTxConfig& reference);

    /**
     * @brief Extract a single value from registers
     * 
     * @param registers Register ID mapped to value of register
     * @param name Name of the requested value
     * @return std::uint32_t Value
     */
    [[nodiscard]] std::uint32_t readValueFromRegisters(
      const std::map<std::uint16_t, std::uint8_t>& registers,
      const std::string& name);

    /**
     * @brief Extract relevant bits from a register
     * 
     * @param reg Register ID
     * @param size Mumber of bits to extract 
     * @param shift Position of the bits in the register
     * @return std::uint8_t 
     */
    [[nodiscard]] std::uint8_t convertRegisterToValue(std::uint16_t reg,
                                                      std::size_t size,
                                                      std::size_t shift);

    /**
     * @brief Get a mask of the relevant bits of a register
     * 
     * @param size Number of bits
     * @param shift Position of bits
     * @return std::uint8_t Mask
     */
    [[nodiscard]] std::uint8_t getMask(std::size_t size, std::size_t shift);

    /**
     * @brief Get the number of registers belonging to a value
     * 
     * @param valueDefinition Definition of the value
     * @return std::size_t Number of registers
     */
    [[nodiscard]] std::size_t getRegCounter(const ::gbtx::regMap& valueDefinition);
  }  // namespace internal
}  // namespace nsw::gbtx

#endif
