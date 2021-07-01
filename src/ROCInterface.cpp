#include "NSWConfiguration/ROCInterface.h"

#include <iterator>

#include <boost/property_tree/ptree.hpp>
#include <numeric>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/SCAInterface.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/ConfigConverter.h"

void nsw::DeviceInterface::ROC::writeConfiguration(
  const nsw::FEBConfig& config) {
  constexpr bool INACTIVE_HIGH = true;
  constexpr bool ACTIVE_LOW    = true;

  const auto& opc_connection =
    OpcManager::getConnection(config.getOpcServerIp());
  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocCoreResetN", ACTIVE_LOW);
  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocPllResetN", ACTIVE_LOW);
  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocSResetN", ACTIVE_LOW);

  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocSResetN", INACTIVE_HIGH);

  nsw::DeviceInterface::SCA::sendI2cMasterConfig(
    opc_connection, config.getAddress(), config.getRocAnalog());

  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocPllResetN", INACTIVE_HIGH);

  ERS_DEBUG(2, "Waiting for ROC Pll locks...");
  bool roc_locked = false;
  while (!roc_locked) {
    const bool rPll1 = nsw::DeviceInterface::SCA::readGPIO(
      opc_connection, config.getAddress() + ".gpio.rocPllLocked");
    const bool rPll2 = nsw::DeviceInterface::SCA::readGPIO(
      opc_connection, config.getAddress() + ".gpio.rocPllRocLocked");
    roc_locked = rPll1 && rPll2;
    ERS_DEBUG(2, "rocPllLocked: " << rPll1 << ", rocPllRocLocked: " << rPll2);
  }

  nsw::DeviceInterface::SCA::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocCoreResetN", INACTIVE_HIGH);

  nsw::DeviceInterface::SCA::sendI2cMasterConfig(
    opc_connection, config.getAddress(), config.getRocDigital());
}

std::map<std::uint8_t, std::vector<std::uint8_t>>
nsw::DeviceInterface::ROC::readConfiguration(const nsw::FEBConfig& config) {
  std::map<std::uint8_t, std::vector<std::uint8_t>> result;
  for (std::uint8_t regNumber = 0;
       regNumber <
       nsw::roc::NUM_ANALOG_REGISTERS + nsw::roc::NUM_DIGITAL_REGISTERS;
       regNumber++) {
    result.emplace(regNumber,
                   nsw::DeviceInterface::ROC::readRegister(config, regNumber));
  }
  return result;
}

void nsw::DeviceInterface::ROC::writeRegister(const nsw::FEBConfig& config,
                                              const std::uint8_t    registerId,
                                              const std::uint8_t    value) {
  const auto        isAnalog   = registerId > nsw::roc::NUM_DIGITAL_REGISTERS;
  const std::string regAddress = [isAnalog, registerId]() {
    if (isAnalog) {
      if (registerId > ROC_ANALOG_REGISTERS.size()) {
        throw std::runtime_error("Analog ROC register out of range: " +
                                 std::to_string(registerId));
      }
      auto iter = std::begin(ROC_ANALOG_REGISTERS);
      std::advance(iter, registerId);
      return iter->first;
    }
    if (registerId >
        ROC_DIGITAL_REGISTERS.size() + nsw::roc::NUM_DIGITAL_REGISTERS) {
      throw std::runtime_error("Digital ROC register out of range: " +
                               std::to_string(registerId));
    }
    auto iter = std::begin(ROC_DIGITAL_REGISTERS);
    std::advance(iter, registerId - nsw::roc::NUM_DIGITAL_REGISTERS);
    return iter->first;
  }();
  nsw::DeviceInterface::ROC::writeRegister(config, regAddress, value);
}

void nsw::DeviceInterface::ROC::writeRegister(const nsw::FEBConfig& config,
                                              const std::string&    regAddress,
                                              const std::uint8_t    value) {
  const auto isAnalog = [&regAddress]() {
    if (ROC_ANALOG_REGISTERS.find(regAddress) !=
        std::end(ROC_ANALOG_REGISTERS)) {
      return true;
    }
    if (ROC_DIGITAL_REGISTERS.find(regAddress) !=
        std::end(ROC_DIGITAL_REGISTERS)) {
      return false;
    }
    throw std::runtime_error("Invalid register name " + regAddress);
  }();
  const std::string sectionName = [isAnalog]() {
    if (isAnalog) {
      return std::string{ROC_ANALOG_NAME};
    }
    return std::string{ROC_DIGITAL_NAME};
  }();

  const auto& opcConnection =
    OpcManager::getConnection(config.getOpcServerIp());
  const auto scaAddress = config.getAddress();
  // TODO: Resets
  if (isAnalog) {
    constexpr bool ACTIVE_LOW = true;
    nsw::DeviceInterface::SCA::sendGPIO(
      opcConnection, scaAddress + ".gpio.rocPllResetN", ACTIVE_LOW);
  }
  nsw::DeviceInterface::SCA::sendI2cMasterSingle(
    opcConnection, scaAddress + '.' + sectionName, {value}, regAddress);

  if (isAnalog) {
    constexpr bool INACTIVE_HIGH = true;
    nsw::DeviceInterface::SCA::sendGPIO(
      opcConnection, scaAddress + ".gpio.rocPllResetN", INACTIVE_HIGH);

    ERS_DEBUG(2, "Waiting for ROC Pll locks...");
    bool roc_locked = false;
    while (!roc_locked) {
      const bool rPll1 = nsw::DeviceInterface::SCA::readGPIO(
        opcConnection, scaAddress + ".gpio.rocPllLocked");
      const bool rPll2 = nsw::DeviceInterface::SCA::readGPIO(
        opcConnection, scaAddress + ".gpio.rocPllRocLocked");
      roc_locked = rPll1 && rPll2;
      ERS_DEBUG(2, "rocPllLocked: " << rPll1 << ", rocPllRocLocked: " << rPll2);
    }
  }
}

std::vector<std::uint8_t> nsw::DeviceInterface::ROC::readRegister(
  const nsw::FEBConfig& config,
  const std::uint8_t    registerId) {
  const auto             scaAddress = config.getAddress();
  constexpr unsigned int DELAY      = 2;
  const auto [sclLine, sdaLine] =
    [&scaAddress, registerId]() -> std::pair<unsigned int, unsigned int> {
    const auto isMmfe8  = scaAddress.find("MMFE8") != std::string::npos;
    const auto isAnalog = registerId > nsw::roc::NUM_DIGITAL_REGISTERS;
    if (isMmfe8 and isAnalog) {
      return {nsw::roc::mmfe8::analog::SCL_LINE_PIN,
              nsw::roc::mmfe8::analog::SDA_LINE_PIN};
    }
    if (isMmfe8 and not isAnalog) {
      return {nsw::roc::mmfe8::digital::SCL_LINE_PIN,
              nsw::roc::mmfe8::digital::SDA_LINE_PIN};
    }
    if (not isMmfe8 and isAnalog) {
      return {nsw::roc::sfeb::analog::SCL_LINE_PIN,
              nsw::roc::sfeb::analog::SDA_LINE_PIN};
    }
    return {nsw::roc::sfeb::digital::SCL_LINE_PIN,
            nsw::roc::sfeb::digital::SDA_LINE_PIN};
  }();
  const auto& opc_connection =
    nsw::OpcManager::getConnection(config.getOpcServerIp());
  return {opc_connection->readRocRaw(
    scaAddress + ".gpio.bitBanger", sclLine, sdaLine, registerId, DELAY)};
}

void nsw::DeviceInterface::ROC::enableVmmCaptureInputs(
  const nsw::FEBConfig& feb) {
  boost::property_tree::ptree tree;
  tree.put_child("reg008vmmEnable",
                 feb.getConfig().get_child("rocCoreDigital.reg008vmmEnable"));
  const auto configConverter =
    ConfigConverter(tree,
                    ConfigConverter::RegisterAddressSpace::ROC_DIGITAL,
                    ConfigConverter::ConfigType::REGISTER_BASED);
  const auto translatedPtree =
    configConverter.getFlatRegisterBasedConfig(feb.getRocDigital());
  const auto partialConfig = nsw::I2cMasterConfig(
    translatedPtree, ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS, true);
  nsw::DeviceInterface::SCA::sendI2cMasterConfig(
    nsw::OpcManager::getConnection(feb.getOpcServerIp()),
    feb.getAddress(),
    partialConfig);
}

void nsw::DeviceInterface::ROC::disableVmmCaptureInputs(
  const nsw::FEBConfig& feb) {
  boost::property_tree::ptree tree;
  tree.put("reg008vmmEnable", 0);
  const auto partialConfig =
    nsw::I2cMasterConfig(tree, ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS, true);
  nsw::DeviceInterface::SCA::sendI2cMasterConfig(
    nsw::OpcManager::getConnection(feb.getOpcServerIp()),
    feb.getAddress(),
    partialConfig);
}
