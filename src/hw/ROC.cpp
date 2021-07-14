#include "NSWConfiguration/hw/ROC.h"

#include <iterator>
#include <stdexcept>

#include <fmt/core.h>

#include <boost/property_tree/ptree.hpp>

#include <ers/ers.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/ConfigConverter.h"

nsw::hw::ROC::ROC(const nsw::FEBConfig& config) :
  m_rocAnalog(config.getRocAnalog()),
  m_rocDigital(config.getRocDigital()),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress())
{}

void nsw::hw::ROC::writeConfiguration() const
{
  constexpr bool INACTIVE = false;
  constexpr bool ACTIVE = true;

  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  setCoreResetN(opcConnection, ACTIVE);
  setPllResetN(opcConnection, ACTIVE);
  setSResetN(opcConnection, ACTIVE);

  setSResetN(opcConnection, INACTIVE);

  nsw::hw::SCA::sendI2cMasterConfig(opcConnection, m_scaAddress, m_rocAnalog);

  setPllResetN(opcConnection, INACTIVE);
  setCoreResetN(opcConnection, INACTIVE);

  nsw::hw::SCA::sendI2cMasterConfig(opcConnection, m_scaAddress, m_rocDigital);
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

  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);

  if (isAnalog) {
    constexpr bool ACTIVE = true;
    setPllResetN(opcConnection, ACTIVE);
  }
  nsw::hw::SCA::sendI2cMasterSingle(
    opcConnection, fmt::format("{}.{}", m_scaAddress, sectionName), {value}, regName);

  if (isAnalog) {
    constexpr bool INACTIVE = false;
    setPllResetN(opcConnection, INACTIVE);
  }
}

std::uint8_t nsw::hw::ROC::readRegister(const std::uint8_t regAddress) const
{
  if (std::find(std::begin(UNUSED_REGISTERS), std::end(UNUSED_REGISTERS), regAddress) != std::end(UNUSED_REGISTERS)) {
    throw UnusedRegisterException(fmt::format("Cannot read unused register {}", regAddress));
  }
  constexpr unsigned int DELAY = 2;
  const auto [sclLine, sdaLine] = [&address = m_scaAddress,
                                   regAddress]() -> std::pair<unsigned int, unsigned int> {
    const auto isMmfe8 = address.find("MMFE8") != std::string::npos;
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
  const auto& opcConnection = nsw::OpcManager::getConnection(m_opcserverIp);
  return opcConnection->readRocRaw(
    fmt::format("{}.gpio.bitBanger", m_scaAddress), sclLine, sdaLine, regAddress, DELAY);
}

void nsw::hw::ROC::enableVmmCaptureInputs() const
{
  boost::property_tree::ptree tree;
  tree.put_child("reg008vmmEnable",
                 m_rocDigital.getConfig().get_child("reg008vmmEnable"));
  const auto configConverter = ConfigConverter<ConfigConversionType::ROC_DIGITAL>(tree, ConfigType::REGISTER_BASED);
  const auto translatedPtree = configConverter.getFlatRegisterBasedConfig(m_rocDigital);
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

void nsw::hw::ROC::setSResetN(const OpcClientPtr& opcConnection, const bool state) const
{
  setReset(opcConnection, "rocSResetN", state);
}

void nsw::hw::ROC::setPllResetN(const OpcClientPtr& opcConnection, const bool state) const
{
  setReset(opcConnection, "rocPllResetN", state);

  if (not state) {
    bool roc_locked = false;
    while (!roc_locked) {
      const bool rPll1 =
        nsw::hw::SCA::readGPIO(opcConnection, fmt::format("{}.gpio.rocPllLocked", m_scaAddress));
      const bool rPll2 =
        nsw::hw::SCA::readGPIO(opcConnection, fmt::format("{}.gpio.rocPllRocLocked", m_scaAddress));
      roc_locked = rPll1 && rPll2;
    }
  }
}

void nsw::hw::ROC::setCoreResetN(const OpcClientPtr& opcConnection, const bool state) const
{
  setReset(opcConnection, "rocCoreResetN", state);
}

void nsw::hw::ROC::setReset(const OpcClientPtr& opcConnection,
                            const std::string& resetName,
                            const bool state) const
{
  // Active = Low
  nsw::hw::SCA::sendGPIO(
    opcConnection, fmt::format("{}.gpio.{}", m_scaAddress, resetName), not state);
}
