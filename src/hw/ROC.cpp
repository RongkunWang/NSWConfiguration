#include "NSWConfiguration/hw/ROC.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#include <fmt/core.h>

#include <boost/property_tree/ptree.hpp>

#include <ers/ers.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/Helper.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/ConfigConverter.h"

nsw::hw::ROC::ROC(OpcManager& manager, const nsw::FEBConfig& config) :
  ScaAddressBase(config.getAddress()),
  OpcConnectionBase(manager, config.getOpcServerIp(), config.getAddress()),
  m_rocAnalog(config.getRocAnalog()),
  m_rocDigital(config.getRocDigital())
{}

void nsw::hw::ROC::writeConfiguration() const
{
  constexpr bool INACTIVE = false;
  constexpr bool ACTIVE = true;

  setCoreResetN(getConnection(), ACTIVE);
  setPllResetN(getConnection(), ACTIVE);
  setSResetN(getConnection(), ACTIVE);

  setSResetN(getConnection(), INACTIVE);

  nsw::hw::SCA::sendI2cMasterConfig(getConnection(), getScaAddress(), m_rocAnalog);

  setPllResetN(getConnection(), INACTIVE);
  setCoreResetN(getConnection(), INACTIVE);

  nsw::hw::SCA::sendI2cMasterConfig(getConnection(), getScaAddress(), m_rocDigital);
}

std::map<std::uint8_t, std::uint8_t> nsw::hw::ROC::readConfiguration() const
{
  std::map<std::uint8_t, std::uint8_t> result;
  for (std::uint8_t regNumber = 0;
       regNumber <
       static_cast<std::uint8_t>(ROC_ANALOG_REGISTERS.size() + ROC_DIGITAL_REGISTERS.size());
       regNumber++) {
    try {
      result[regNumber] = readRegister(regNumber);
    }
    catch (const UnusedRegisterException&) {
      continue;
    }
  }
  return result;
}

void nsw::hw::ROC::writeRegister(const std::uint8_t regAddress, const std::uint8_t value) const
{
  if (std::find(std::begin(UNUSED_REGISTERS), std::end(UNUSED_REGISTERS), regAddress) != std::end(UNUSED_REGISTERS)) {
    throw UnusedRegisterException(fmt::format("Cannot read unused register {}", regAddress));
  }
  const auto isAnalog = regAddress >= ROC_DIGITAL_REGISTERS.size();
  const std::string regName = [isAnalog, regAddress]() {
    const auto getKeyFromMap = [](const auto& map, const auto num) {
      auto iter = std::begin(map);
      std::advance(iter, num);
      return iter->first;
    };
    if (isAnalog) {
      if (regAddress >= ROC_ANALOG_REGISTERS.size() + ROC_DIGITAL_REGISTERS.size()) {
        throw std::out_of_range(fmt::format("Analog ROC register out of range: {}", regAddress));
      }
      return getKeyFromMap(ROC_ANALOG_REGISTERS, regAddress - ROC_DIGITAL_REGISTERS.size());
    }
    if (regAddress >= ROC_DIGITAL_REGISTERS.size()) {
      throw std::out_of_range(fmt::format("Digital ROC register out of range: {}", regAddress));
    }
    return getKeyFromMap(ROC_DIGITAL_REGISTERS, regAddress);
  }();
  writeRegister(regName, value);
}

void nsw::hw::ROC::writeRegister(const std::string& regName, const std::uint8_t value) const
{
  const auto isAnalog = [&regName]() {
    if (ROC_ANALOG_REGISTERS.find(regName) != std::end(ROC_ANALOG_REGISTERS)) {
      return true;
    }
    if (ROC_DIGITAL_REGISTERS.find(regName) != std::end(ROC_DIGITAL_REGISTERS)) {
      return false;
    }
    throw std::logic_error(fmt::format("Invalid register name {}", regName));
  }();
  const std::string sectionName = [isAnalog]() {
    if (isAnalog) {
      return std::string{ROC_ANALOG_NAME};
    }
    return std::string{ROC_DIGITAL_NAME};
  }();

  if (isAnalog) {
    constexpr bool ACTIVE = true;
    setPllResetN(getConnection(), ACTIVE);
  }
  nsw::hw::SCA::sendI2cMasterSingle(
    getConnection(), fmt::format("{}.{}", getScaAddress(), sectionName), {value}, regName);

  if (isAnalog) {
    constexpr bool INACTIVE = false;
    setPllResetN(getConnection(), INACTIVE);
  }
}

std::uint8_t nsw::hw::ROC::readRegister(const std::uint8_t regAddress) const
{
  if (std::find(std::begin(UNUSED_REGISTERS), std::end(UNUSED_REGISTERS), regAddress) != std::end(UNUSED_REGISTERS)) {
    throw UnusedRegisterException(fmt::format("Cannot read unused register {}", regAddress));
  }
  constexpr unsigned int DELAY = 2;
  const auto [sclLine, sdaLine] = [address = getScaAddress(),
                                   regAddress]() -> std::pair<unsigned int, unsigned int> {
    const auto isMmfe8 = address.find("MM") != std::string::npos;  // FIXME: Use util function
    const auto isAnalog = regAddress >= ROC_DIGITAL_REGISTERS.size();
    if (isMmfe8 and isAnalog) {
      return {nsw::roc::mmfe8::analog::SCL_LINE_PIN, nsw::roc::mmfe8::analog::SDA_LINE_PIN};
    }
    if (isMmfe8 and not isAnalog) {
      return {nsw::roc::mmfe8::digital::SCL_LINE_PIN, nsw::roc::mmfe8::digital::SDA_LINE_PIN};
    }
    if (not isMmfe8 and isAnalog) {
      return {nsw::roc::sfeb::analog::SCL_LINE_PIN, nsw::roc::sfeb::analog::SDA_LINE_PIN};
    }
    return {nsw::roc::sfeb::digital::SCL_LINE_PIN, nsw::roc::sfeb::digital::SDA_LINE_PIN};
  }();
  return nsw::hw::SCA::readRocRaw(getConnection(),
                                  fmt::format("{}.gpio.bitBanger", getScaAddress()),
                                  sclLine,
                                  sdaLine,
                                  regAddress,
                                  DELAY);
}

void nsw::hw::ROC::writeValues(const std::map<std::string, unsigned int>& values) const
{
  const auto valuesTree = transformMapToPtree(values);

  // Get keys from map. Will be replaced by std::views::keys(map)
  const auto paths = [&values]() {
    std::vector<std::string> keys{};
    keys.reserve(values.size());
    std::transform(std::cbegin(values),
                   std::cend(values),
                   std::back_inserter(keys),
                   [](const auto& pair) { return pair.first; });
    return keys;
  }();
  const auto isAnalog = internal::ROC::namesAnalog(paths);

  // Lambda to const init the converted config. Pulls in this, the values and isAnalog
  const auto config = [this, &valuesTree, &isAnalog]() {
    if (isAnalog) {
      const auto configConverter =
        ConfigConverter<ConfigConversionType::ROC_ANALOG>(valuesTree, ConfigType::VALUE_BASED);
      return nsw::I2cMasterConfig(
        configConverter.getFlatRegisterBasedConfig(m_rocAnalog.getBitstreamMap()),
        ROC_ANALOG_NAME,
        ROC_ANALOG_REGISTERS,
        true);
    }
    const auto configConverter =
      ConfigConverter<ConfigConversionType::ROC_DIGITAL>(valuesTree, ConfigType::VALUE_BASED);
    return nsw::I2cMasterConfig(
      configConverter.getFlatRegisterBasedConfig(m_rocDigital.getBitstreamMap()),
      ROC_DIGITAL_NAME,
      ROC_DIGITAL_REGISTERS,
      true);
  }();

  if (isAnalog) {
    constexpr bool ACTIVE = true;
    setPllResetN(getConnection(), ACTIVE);
  }
  nsw::hw::SCA::sendI2cMasterConfig(getConnection(), getScaAddress(), config);
  if (isAnalog) {
    constexpr bool INACTIVE = false;
    setPllResetN(getConnection(), INACTIVE);
  }
}

void nsw::hw::ROC::writeValue(const std::string& name, const unsigned int value) const
{
  writeValues({std::pair{name, value}});
}

unsigned int nsw::hw::ROC::readValue(const std::string& name) const
{
  return std::cbegin(readValues(std::array{name}))->second;
}

std::map<std::string, unsigned int> nsw::hw::ROC::readValues(
  const std::span<const std::string> names) const
{
  const auto isAnalog = internal::ROC::namesAnalog(names);
  // Lambda to const init the register names. Pulls in isAnalog and the names and asks the config
  // converter for the registers for the given names
  const auto regNames = [isAnalog, &names] {
    std::unordered_set<std::string> result{};
    for (const auto& name : names) {
      if (isAnalog) {
        result.merge(ConfigConverter<ConfigConversionType::ROC_ANALOG>::getRegsForValue(name));
      } else {
        result.merge(ConfigConverter<ConfigConversionType::ROC_DIGITAL>::getRegsForValue(name));
      }
    }
    return result;
  }();

  std::map<std::string, unsigned int> registerValues{};
  std::transform(std::cbegin(regNames),
                 std::cend(regNames),
                 std::inserter(registerValues, std::begin(registerValues)),
                 [&](const auto& regName) -> std::pair<std::string, std::uint8_t> {
                   return {regName, readRegister(getRegAddress(regName, isAnalog))};
                 });

  // Lambda to const init the read-back values. Pulls in isAnalog, the read-back register values and
  // the names. Asks the config converter to do the translation. TODO: Clean up config converter
  // call (future MR)
  const auto values = [isAnalog, &registerValues, &names]() {
    if (isAnalog) {
      const auto configConverter = ConfigConverter<ConfigConversionType::ROC_ANALOG>(
        transformMapToPtree(
          ConfigConverter<ConfigConversionType::ROC_ANALOG>::convertRegisterToSubRegister(
            transformMapToPtree(registerValues), names)),
        ConfigType::REGISTER_BASED);
      return configConverter.getValueBasedConfig();
    }
    const auto configConverter = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(
      transformMapToPtree(
        ConfigConverter<ConfigConversionType::ROC_DIGITAL>::convertRegisterToSubRegister(
          transformMapToPtree(registerValues), names)),
      ConfigType::REGISTER_BASED);
    return configConverter.getValueBasedConfig();
  }();

  return transformPtreetoMap<unsigned int>(values);
}

void nsw::hw::ROC::enableVmmCaptureInputs() const
{
  boost::property_tree::ptree tree;
  tree.put_child("reg008vmmEnable",
                 m_rocDigital.getConfig().get_child("reg008vmmEnable"));
  const auto configConverter = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(tree, ConfigType::REGISTER_BASED);
  const auto translatedPtree = configConverter.getFlatRegisterBasedConfig(m_rocDigital.getBitstreamMap());
  writeRegister("reg008vmmEnable", translatedPtree.get<std::uint8_t>("reg008vmmEnable"));
}

void nsw::hw::ROC::disableVmmCaptureInputs() const
{
  writeRegister("reg008vmmEnable", 0);
}

std::uint8_t nsw::hw::ROC::readVmmCaptureStatus(const std::uint8_t vmmIndex) const
{
  if (vmmIndex >= nsw::MAX_NUMBER_OF_VMM) {
    throw std::out_of_range(
      fmt::format("VMM index is out of range 0-{}: {}", nsw::MAX_NUMBER_OF_VMM - 1, vmmIndex));
  }
  const std::uint8_t offset = 32;  // 32 is the first VMM capture status register
  return readStatusRegister(offset + vmmIndex);
}

std::uint8_t nsw::hw::ROC::readVmmParityCounter(std::uint8_t vmmIndex) const
{
  if (vmmIndex >= nsw::MAX_NUMBER_OF_VMM) {
    throw std::out_of_range(
      fmt::format("VMM index is out of range 0-{}: {}", nsw::MAX_NUMBER_OF_VMM - 1, vmmIndex));
  }
  const std::uint8_t offset = 45;  // 45 is the first VMM parity counter register
  return readStatusRegister(offset + vmmIndex);
}

std::uint8_t nsw::hw::ROC::readSrocStatus(std::uint8_t srocIndex) const
{
  if (srocIndex >= nsw::roc::NUM_SROCS) {
    throw std::out_of_range(
      fmt::format("sROC index is out of range 0-{}: {}", nsw::roc::NUM_SROCS - 1, srocIndex));
  }
  const std::uint8_t offset = 40;  // 40 is the first sROC status register
  return readStatusRegister(offset + srocIndex);
}

std::uint8_t nsw::hw::ROC::readStatusRegister(const std::uint8_t regAddress) const
{
  static_cast<void>(readRegister(regAddress));
  return readRegister(regAddress);
}

void nsw::hw::ROC::setSResetN(const nsw::OpcClientPtr opcConnection, const bool state) const
{
  setReset(opcConnection, "rocSResetN", state);
}

void nsw::hw::ROC::setPllResetN(const nsw::OpcClientPtr opcConnection, const bool state) const
{
  setReset(opcConnection, "rocPllResetN", state);

  if (not state) {
    bool roc_locked = false;
    while (!roc_locked) {
      const bool rPll1 =
        nsw::hw::SCA::readGPIO(opcConnection, fmt::format("{}.gpio.rocPllLocked", getScaAddress()));
      const bool rPll2 =
        nsw::hw::SCA::readGPIO(opcConnection, fmt::format("{}.gpio.rocPllRocLocked", getScaAddress()));
      roc_locked = rPll1 && rPll2;
    }
  }
}

void nsw::hw::ROC::setCoreResetN(const nsw::OpcClientPtr opcConnection, const bool state) const
{
  setReset(opcConnection, "rocCoreResetN", state);
}

void nsw::hw::ROC::setReset(const nsw::OpcClientPtr opcConnection,
                            const std::string& resetName,
                            const bool state) const
{
  // Active = Low
  nsw::hw::SCA::sendGPIO(
    opcConnection, fmt::format("{}.gpio.{}", getScaAddress(), resetName), not state);
}

std::uint8_t nsw::hw::ROC::getRegAddress(const std::string& regName, const bool isAnalog)
{
  if (isAnalog) {
    if (ROC_ANALOG_REGISTERS.find(regName) == std::end(ROC_ANALOG_REGISTERS)) {
      throw std::logic_error(fmt::format("Did not find register {}", regName));
    }
    return static_cast<std::uint8_t>(
      static_cast<std::uint8_t>(std::distance(std::cbegin(ROC_ANALOG_REGISTERS),
                                              ROC_ANALOG_REGISTERS.find(regName))) +
      ROC_DIGITAL_REGISTERS.size());
  }
  if (ROC_DIGITAL_REGISTERS.find(regName) == std::end(ROC_DIGITAL_REGISTERS)) {
    throw std::logic_error(fmt::format("Did not find register {}", regName));
  }
  return static_cast<std::uint8_t>(std::distance(std::cbegin(ROC_DIGITAL_REGISTERS),
                                                  ROC_DIGITAL_REGISTERS.find(regName)));
}
