#include "NSWConfiguration/ConfigSenderRoc.h"

#include <iterator>

#include <boost/property_tree/ptree.hpp>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/I2cSender.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/ConfigConverter.h"

void nsw::ConfigSender::ROC::writeConfiguration(const nsw::FEBConfig& config) {
  constexpr bool HIGH = true;
  constexpr bool LOW  = true;

  const auto& opc_connection =
    OpcManager::getConnection(config.getOpcServerIp());
  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocCoreResetN", LOW);
  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocPllResetN", LOW);
  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocSResetN", LOW);

  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocSResetN", HIGH);

  nsw::ConfigSender::I2c::sendI2cMasterConfig(
    opc_connection, config.getAddress(), config.getRocAnalog());

  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocPllResetN", HIGH);

  ERS_DEBUG(2, "Waiting for ROC Pll locks...");
  bool roc_locked = false;
  while (!roc_locked) {
    bool rPll1 = nsw::ConfigSender::I2c::readGPIO(
      opc_connection, config.getAddress() + ".gpio.rocPllLocked");
    bool rPll2 = nsw::ConfigSender::I2c::readGPIO(
      opc_connection, config.getAddress() + ".gpio.rocPllRocLocked");
    roc_locked = rPll1 && rPll2;
    ERS_DEBUG(2, "rocPllLocked: " << rPll1 << ", rocPllRocLocked: " << rPll2);
  }

  nsw::ConfigSender::I2c::sendGPIO(
    opc_connection, config.getAddress() + ".gpio.rocCoreResetN", HIGH);

  nsw::ConfigSender::I2c::sendI2cMasterConfig(
    opc_connection, config.getAddress(), config.getRocDigital());
}

void nsw::ConfigSender::ROC::writeRegister(const std::string& opcserver_ipport,
                                           const std::string& sca_address,
                                           const std::uint8_t registerId,
                                           const std::uint8_t value) {
  const auto        isAnalog    = registerId > nsw::roc::NUM_DIGITAL_REGISTERS;
  const std::string sectionName = [isAnalog]() {
    if (isAnalog) {
      return std::string{ROC_ANALOG_NAME};
    }
    return std::string{ROC_DIGITAL_NAME};
  }();
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
    if (registerId - nsw::roc::NUM_DIGITAL_REGISTERS >
        ROC_DIGITAL_REGISTERS.size()) {
      throw std::runtime_error("Digital ROC register out of range: " +
                               std::to_string(registerId));
    }
    auto iter = std::begin(ROC_DIGITAL_REGISTERS);
    std::advance(iter, registerId - nsw::roc::NUM_DIGITAL_REGISTERS);
    return iter->first;
  }();
  nsw::ConfigSender::I2c::sendI2cMasterSingle(
    nsw::OpcManager::getConnection(opcserver_ipport),
    sca_address + '.' + sectionName,
    {value},
    regAddress);
}

void nsw::ConfigSender::ROC::writeRegister(const std::string& opcserver_ipport,
                                           const std::string& sca_address,
                                           const std::string& regAddress,
                                           const std::uint8_t value) {
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
  nsw::ConfigSender::I2c::sendI2cMasterSingle(
    nsw::OpcManager::getConnection(opcserver_ipport),
    sca_address + '.' + sectionName,
    {value},
    regAddress);
}

std::uint8_t nsw::ConfigSender::ROC::readRegister(
  const std::string& opcserver_ipport,
  const std::string& sca_address,
  const std::uint8_t registerId) {
  constexpr unsigned int DELAY = 2;
  const auto [sclLine, sdaLine] =
    [&sca_address, registerId]() -> std::pair<unsigned int, unsigned int> {
    const auto isMmfe8  = sca_address.find("MMFE8") != std::string::npos;
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
  const auto& opc_connection = nsw::OpcManager::getConnection(opcserver_ipport);
  return opc_connection->readRocRaw(
    sca_address + ".gpio.bitBanger", sclLine, sdaLine, registerId, DELAY);
}

void nsw::ConfigSender::ROC::enableVmmCaptureInputs(const nsw::FEBConfig& feb) {
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
  nsw::ConfigSender::I2c::sendI2cMasterConfig(
    nsw::OpcManager::getConnection(feb.getOpcServerIp()),
    feb.getAddress(),
    partialConfig);
}

void nsw::ConfigSender::ROC::disableVmmCaptureInputs(
  const nsw::FEBConfig& feb) {
  boost::property_tree::ptree tree;
  tree.put("reg008vmmEnable", 0);
  const auto partialConfig =
    nsw::I2cMasterConfig(tree, ROC_DIGITAL_NAME, ROC_DIGITAL_REGISTERS, true);
  nsw::ConfigSender::I2c::sendI2cMasterConfig(
    nsw::OpcManager::getConnection(feb.getOpcServerIp()),
    feb.getAddress(),
    partialConfig);
}
