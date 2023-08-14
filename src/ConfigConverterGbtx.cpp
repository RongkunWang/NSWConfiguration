#include "NSWConfiguration/ConfigConverterGbtx.h"

#include <bits/ranges_algo.h>
#include <cstdint>
#include <limits>
#include <ranges>

#include "NSWConfiguration/GBTxRegisterMap.h"

std::unordered_set<std::uint16_t> nsw::gbtx::getRegsForValue(const std::string& name)
{
  const auto iter = GBTX_REGISTER_MAP.find(name);
  const auto regs = iter->second.registers;
  return {std::cbegin(regs), std::cend(regs)};
}

std::map<std::uint16_t, std::uint8_t> nsw::gbtx::convertValuesToRegisters(
  const std::map<std::string, std::uint32_t>& values,
  const GBTxConfig& reference)
{
  const auto valuesWithHoles = internal::convertValuesWithHoles(values);
  std::map<std::uint16_t, std::uint8_t> result{};
  std::ranges::transform(
    valuesWithHoles,
    std::inserter(result, std::end(result)),
    [&reference](const auto& pair) -> decltype(result)::value_type {
      return {pair.first, internal::superimposeReference(pair.first, pair.second, reference)};
    });
  return result;
}

std::map<std::string, std::uint32_t> nsw::gbtx::convertRegistersToValues(
  const std::map<std::uint16_t, std::uint8_t>& registers,
  const std::span<const std::string> names)
{
  std::map<std::string, std::uint32_t> result{};
  std::ranges::transform(names,
                         std::inserter(result, std::end(result)),
                         [&registers](const auto& name) -> decltype(result)::value_type {
                           return {name, internal::readValueFromRegisters(registers, name)};
                         });
}

std::map<std::uint16_t, nsw::gbtx::internal::RegisterValue>
nsw::gbtx::internal::convertValuesWithHoles(const std::map<std::string, std::uint32_t>& values)
{
  auto coveredParts = values |
                      std::views::transform(nsw::gbtx::internal::convertValueToRegisterSpace) |
                      std::views::join;
  std::map<std::uint16_t, RegisterValue> mergedCoveredParts{};
  for (const auto& coveredPart : coveredParts) {
    if (not mergedCoveredParts.contains(coveredPart.reg)) {
      mergedCoveredParts.try_emplace(coveredPart.reg, RegisterValue{});
    }
    mergedCoveredParts.at(coveredPart.reg).mask |= coveredPart.val.mask;
    mergedCoveredParts.at(coveredPart.reg).val |= coveredPart.val.val;
  }
  return mergedCoveredParts;
}

std::vector<nsw::gbtx::internal::RegisterSpacePart>
nsw::gbtx::internal::convertValueToRegisterSpace(const std::string& name, const std::uint32_t value)
{
  std::vector<internal::RegisterSpacePart> result{};
  const auto& valueDefinition = GBTX_REGISTER_MAP.at(name);
  result.reserve(std::size(valueDefinition.registers));
  if (std::size(valueDefinition.registers) != std::size(valueDefinition.shifts)) {
    throw std::logic_error("Fuck you");
  }
  const auto numRegs = getRegCounter(valueDefinition);
  for (std::size_t regCounter = 0; regCounter < numRegs; ++regCounter) {
    for (std::size_t i = 0; i < std::size(valueDefinition.registers); ++i) {
      constexpr static auto TAKE_8 = std::uint8_t{0xFF};
      constexpr static std::size_t MAX_REGISTER_SIZE{8};
      result.emplace_back(
        valueDefinition.registers.at(i) + regCounter,
        RegisterValue{.val = static_cast<uint8_t>(
                        (((value & TAKE_8) << ((numRegs - regCounter - 1) * MAX_REGISTER_SIZE))
                         << valueDefinition.shifts.at(i))),
                      .mask = getMask(valueDefinition.size, valueDefinition.shifts.at(i))});
    }
  }
  return result;
}

std::uint8_t nsw::gbtx::internal::superimposeReference(std::uint16_t reg,
                                                       const RegisterValue& value,
                                                       const GBTxConfig& reference)
{
  const auto holes = static_cast<std::uint8_t>(~value.mask);
  if (holes == 0) {
    return value.val;
  }
  return value.val & static_cast<std::uint8_t>(reference.reg(reg) & holes);
}

std::uint32_t nsw::gbtx::internal::readValueFromRegisters(
  const std::map<std::uint16_t, std::uint8_t>& registers,
  const std::string& name)
{
  const auto& valueDefinition = GBTX_REGISTER_MAP.at(name);
  if (std::ranges::any_of(valueDefinition.registers,
                          [&registers](const auto reg) { return not registers.contains(reg); })) {
    throw std::runtime_error("kapott");
  }
  if (std::size(valueDefinition.registers) != std::size(valueDefinition.shifts)) {
    throw std::logic_error("Fuck you");
  }
  std::vector<std::uint32_t> values{};
  values.reserve(std::size(valueDefinition.registers));
  const auto numRegs = getRegCounter(valueDefinition);
  for (std::size_t i = 0; i < std::size(valueDefinition.registers); ++i) {
    std::uint32_t val{};
    for (std::size_t regCounter = 0; regCounter < numRegs; ++regCounter) {
      constexpr static std::size_t MAX_REGISTER_SIZE{8};
      const auto regId = valueDefinition.registers.at(i) + regCounter;
      if (regId > std::numeric_limits<std::uint16_t>::max()) {
        throw std::logic_error("So many chances to fuck up");
      }
      val |= static_cast<std::uint32_t>(
        convertRegisterToValue(static_cast<std::uint16_t>(regId),
                               std::min(valueDefinition.size, MAX_REGISTER_SIZE),
                               valueDefinition.shifts.at(i))
        << ((numRegs - regCounter - 1)));
    }
    values.push_back(val);
  }
  if (not std::ranges::all_of(values,
                              [&values](const auto value) { return value != values.front(); })) {
    throw std::runtime_error("Not the same value in all places");
  }
  return values.front();
}

std::uint8_t nsw::gbtx::internal::convertRegisterToValue(std::uint16_t reg,
                                                         std::size_t size,
                                                         std::size_t shift)
{
  const auto mask = getMask(size, shift);
  return static_cast<std::uint8_t>(reg & mask) >> shift;
}

std::uint8_t nsw::gbtx::internal::getMask(const std::size_t size, const std::size_t shift)
{
  return static_cast<std::uint8_t>((~(~std::uint64_t{0} << size)) << shift);
}

std::size_t nsw::gbtx::internal::getRegCounter(const ::gbtx::regMap& valueDefinition)
{
  constexpr static std::size_t MAX_REGISTER_SIZE{8};
  const auto numRegs = static_cast<std::size_t>(
    std::ceil(static_cast<double>(valueDefinition.size) / MAX_REGISTER_SIZE));
  if (numRegs > 1 and
      std::ranges::any_of(valueDefinition.shifts, [](const auto shift) { return shift != 0; })) {
    throw std::logic_error("Fucked up");
  }
  return numRegs;
}
