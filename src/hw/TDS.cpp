#include "NSWConfiguration/hw/TDS.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::TDS::TDS(const FEBConfig& config, const std::size_t numTds) :
  m_config(config.getTdss().at(numTds)),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_isPfeb(config.getTdss().size() < 3)
{}

void nsw::hw::TDS::writeConfiguration(const bool resetTds) const
{
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);

  // Assert that TDS is not in reset
  constexpr bool INCATIVE_HIGH = true;

  if (m_isPfeb) {
    // old boards, and PFEB
    nsw::hw::SCA::sendGPIO(
      opcConnection, fmt::format("{}.gpio.tdsReset", m_scaAddress), INCATIVE_HIGH);
  } else {
    // new boards
    if (m_config.getName() == "tds0") {
      nsw::hw::SCA::sendGPIO(
        opcConnection, fmt::format("{}.gpio.tdsaReset", m_scaAddress), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds1") {
      nsw::hw::SCA::sendGPIO(
        opcConnection, fmt::format("{}.gpio.tdsbReset", m_scaAddress), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds2") {
      nsw::hw::SCA::sendGPIO(
        opcConnection, fmt::format("{}.gpio.tdscReset", m_scaAddress), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds3") {
      nsw::hw::SCA::sendGPIO(
        opcConnection, fmt::format("{}.gpio.tdsdReset", m_scaAddress), INCATIVE_HIGH);
    } else {
      throw std::logic_error(fmt::format("Unknown TDS name {}", m_config.getName()));
    }
  }

  nsw::hw::SCA::sendI2cMasterConfig(opcConnection, m_scaAddress, m_config);

  if (resetTds) {
    // copy out the configuration, etc
    I2cMasterConfig tdss(m_config);
    // TDS resets

    // See page 23 of
    // https://espace.cern.ch/ATLAS-NSW-ELX/Shared%20Documents/TDS/TDS_V2_Specification.pdf
    constexpr std::uint32_t RESET_PLL = 0x20;
    constexpr std::uint32_t RESET_LOGIC = 0x06;
    constexpr std::uint32_t RESET_SER = 0x14;
    constexpr std::uint32_t RESET_OFF = 0x0;
    tdss.setRegisterValue("register12", "resets", RESET_PLL);
    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    // logic
    tdss.setRegisterValue("register12", "resets", RESET_LOGIC);
    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    // SER
    tdss.setRegisterValue("register12", "resets", RESET_SER);
    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);

    nsw::hw::SCA::sendI2cMasterSingle(opcConnection, m_scaAddress, tdss, "register12");

    ERS_LOG("SCA " << m_scaAddress << " TDS " << tdss.getName() << " readback register 14:");

    ERS_LOG("0x" << std::hex
                 << static_cast<uint32_t>(std::stoul(nsw::vectorToBitString(readRegister(14)))));
  }
}

std::map<std::uint8_t, std::vector<std::uint8_t>> nsw::hw::TDS::readConfiguration() const
{
  std::map<std::uint8_t, std::vector<std::uint8_t>> result;
  for (std::uint8_t regNumber = 0; regNumber < static_cast<std::uint8_t>(TDS_REGISTERS.size());
       regNumber++) {
    result.emplace(regNumber, nsw::hw::TDS::readRegister(regNumber));
  }
  return result;
}

void nsw::hw::TDS::writeRegister(const std::uint8_t regAddress, const __uint128_t value) const
{
  writeRegister(fmt::format("register{}", std::to_string(regAddress)), value);
}

void nsw::hw::TDS::writeRegister(const std::string& regName, const __uint128_t value) const
{
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2cMasterSingle(opcConnection,
                                    fmt::format("{}.{}", m_scaAddress, m_config.getName()),
                                    nsw::integerToByteVector(value, m_config.getTotalSize(regName) / nsw::NUM_BITS_IN_BYTE),
                                    regName);
}

std::vector<std::uint8_t> nsw::hw::TDS::readRegister(const std::uint8_t regAddress) const
{
  const std::string registerName = fmt::format("register{}", regAddress);
  const std::string ptreeName = [regAddress, registerName]() {
    constexpr std::uint8_t READONLY_BASE_ADDRESS = 13;
    if (regAddress >= READONLY_BASE_ADDRESS) {
      return fmt::format("{}_READONLY", registerName);
    }
    return registerName;
  }();

  // Get size of register
  const auto sizeInBytes = m_config.getTotalSize(ptreeName) / NUM_BITS_IN_BYTE;
  const std::string fullNodeName =
    fmt::format("{}.{}.{}", m_scaAddress, m_config.getName(), registerName);
  return nsw::hw::SCA::readI2c(OpcManager::getConnection(m_opcserverIp), fullNodeName, sizeInBytes);
}

void nsw::hw::TDS::writeValues(const std::map<std::string, unsigned int>& values) const
{
  const auto valuesTree = transformMapToPtree(values);

  const auto configConverter =
    ConfigConverter<ConfigConversionType::TDS>(valuesTree, ConfigType::VALUE_BASED);
  const auto config =
    nsw::I2cMasterConfig(configConverter.getFlatRegisterBasedConfig(m_config.getBitstreamMap()),
                         m_config.getName(),
                         TDS_REGISTERS,
                         true);

  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);

  nsw::hw::SCA::sendI2cMasterConfig(opcConnection, m_scaAddress, config);
}

void nsw::hw::TDS::writeValue(const std::string& name, const unsigned int value) const
{
  writeValues({std::pair{name, value}});
}

unsigned int nsw::hw::TDS::readValue(const std::string& name) const
{
  return std::cbegin(readValues(std::array{name}))->second;
}
