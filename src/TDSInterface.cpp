#include "NSWConfiguration/TDSInterface.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/I2cInterface.h"

void nsw::DeviceInterface::TDS::writeConfiguration(const nsw::FEBConfig& config,
                                                   const std::size_t     numTds,
                                                   const bool reset_tds) {
  const auto  sca_address = config.getAddress();
  const auto& opc_connection =
    OpcManager::getConnection(config.getOpcServerIp());
  const auto isPfeb = config.getTdss().size() < 3;

  // TODO: Meaning
  constexpr bool HIGH = true;

  const auto& tds = config.getTdss().at(numTds);
  if (isPfeb) {
    // old boards, and PFEB
    nsw::DeviceInterface::I2c::sendGPIO(
      opc_connection, sca_address + ".gpio.tdsReset", HIGH);
  } else {
    // new boards
    if (tds.getName() == "tds0") {
      nsw::DeviceInterface::I2c::sendGPIO(
        opc_connection, sca_address + ".gpio.tdsaReset", HIGH);
    } else if (tds.getName() == "tds1") {
      nsw::DeviceInterface::I2c::sendGPIO(
        opc_connection, sca_address + ".gpio.tdsbReset", HIGH);
    } else if (tds.getName() == "tds2") {
      nsw::DeviceInterface::I2c::sendGPIO(
        opc_connection, sca_address + ".gpio.tdscReset", HIGH);
    } else if (tds.getName() == "tds3") {
      nsw::DeviceInterface::I2c::sendGPIO(
        opc_connection, sca_address + ".gpio.tdsdReset", HIGH);
    } else {
      throw std::runtime_error("Unknown TDS name " + tds.getName());
    }
  }

  nsw::DeviceInterface::I2c::sendI2cMasterConfig(
    opc_connection, sca_address, tds);

  if (reset_tds) {
    // copy out the configuration, etc
    I2cMasterConfig tdss(tds);
    // TDS resets

    constexpr std::uint32_t RESET_PLL   = 0x20;
    constexpr std::uint32_t RESET_LOGIC = 0x06;
    constexpr std::uint32_t RESET_SER   = 0x14;
    constexpr std::uint32_t RESET_OFF   = 0x0;
    tdss.setRegisterValue("register12", "resets", RESET_PLL);
    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    // logic
    tdss.setRegisterValue("register12", "resets", RESET_LOGIC);
    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    // SER
    tdss.setRegisterValue("register12", "resets", RESET_SER);
    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);

    nsw::DeviceInterface::I2c::sendI2cMasterSingle(
      opc_connection, sca_address, tdss, "register12");

    ERS_LOG("SCA " << sca_address << " TDS " << tdss.getName()
                   << " readback register 14:");

    ERS_LOG("0x" << std::hex
                 << static_cast<uint32_t>(readRegister(config, numTds, 14)));
  }
}

std::map<std::uint8_t, std::uint64_t>
nsw::DeviceInterface::TDS::readConfiguration(const nsw::FEBConfig& config,
                                             const std::size_t     numTds) {
  std::map<std::uint8_t, std::uint64_t> result;
  for (std::uint8_t regNumber = 0;
       regNumber < static_cast<std::uint8_t>(TDS_REGISTERS.size());
       regNumber++) {
    result.emplace(
      regNumber,
      nsw::DeviceInterface::TDS::readRegister(config, numTds, regNumber));
  }
  return result;
}

void nsw::DeviceInterface::TDS::writeRegister(const nsw::FEBConfig& config,
                                              const std::size_t     numTds,
                                              const std::uint8_t    registerId,
                                              const std::uint64_t   value) {
  writeRegister(config, numTds, "register" + std::to_string(registerId), value);
}

void nsw::DeviceInterface::TDS::writeRegister(const nsw::FEBConfig& config,
                                              const std::size_t     numTds,
                                              const std::string&    regAddress,
                                              const std::uint64_t   value) {
  const auto& opc_connection =
    OpcManager::getConnection(config.getOpcServerIp());
  nsw::DeviceInterface::I2c::sendI2cMasterSingle(
    opc_connection,
    config.getAddress() + '.' + config.getTdss().at(numTds).getName(),
    nsw::stringToByteVector(std::to_string(value)),
    regAddress);
}

std::uint64_t nsw::DeviceInterface::TDS::readRegister(
  const nsw::FEBConfig& config,
  const std::size_t     numTds,
  const std::uint8_t    registerId) {
  const std::string registerName = "register" + std::to_string(registerId);
  const std::string ptreeName    = [registerId, registerName]() {
    const std::uint8_t readonlyRegisterThreshold = 13;
    if (registerId > readonlyRegisterThreshold) {
      return registerName + "_READONLY";
    }
    return registerName;
  }();

  // Get size of register
  const auto tds           = config.getTdss().at(numTds);
  const auto size_in_bytes = tds.getTotalSize(ptreeName) / NUM_BITS_IN_BYTE;
  const std::string full_node_name = config.getAddress() + "." + registerName;
  const auto        dataread       = nsw::DeviceInterface::I2c::readI2c(
    OpcManager::getConnection(config.getOpcServerIp()),
    full_node_name,
    size_in_bytes);
  return std::stoul(nsw::vectorToBitString(dataread), nullptr, 2);
}