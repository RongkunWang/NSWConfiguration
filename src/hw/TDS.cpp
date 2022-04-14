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

nsw::hw::TDS::TDS(OpcManager& manager, const FEBConfig& config, const std::size_t numTds) :
  ScaAddressBase(config.getAddress()),
  OpcConnectionBase(manager, config.getOpcServerIp(), config.getAddress()),
  m_config(config.getTdss().at(numTds)),
  m_isPfeb(config.getTdss().size() < 3)
{}

void nsw::hw::TDS::writeConfiguration(const bool resetTds) const
{
  // Assert that TDS is not in reset
  constexpr bool INCATIVE_HIGH = true;

  if (m_isPfeb) {
    // old boards, and PFEB
    nsw::hw::SCA::sendGPIO(
      getConnection(), fmt::format("{}.gpio.tdsReset", getScaAddress()), INCATIVE_HIGH);
  } else {
    // new boards
    if (m_config.getName() == "tds0") {
      nsw::hw::SCA::sendGPIO(
        getConnection(), fmt::format("{}.gpio.tdsaReset", getScaAddress()), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds1") {
      nsw::hw::SCA::sendGPIO(
        getConnection(), fmt::format("{}.gpio.tdsbReset", getScaAddress()), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds2") {
      nsw::hw::SCA::sendGPIO(
        getConnection(), fmt::format("{}.gpio.tdscReset", getScaAddress()), INCATIVE_HIGH);
    } else if (m_config.getName() == "tds3") {
      nsw::hw::SCA::sendGPIO(
        getConnection(), fmt::format("{}.gpio.tdsdReset", getScaAddress()), INCATIVE_HIGH);
    } else {
      throw std::logic_error(fmt::format("Unknown TDS name {}", m_config.getName()));
    }
  }

  nsw::hw::SCA::sendI2cMasterConfig(getConnection(), getScaAddress(), m_config);

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
    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    // logic
    tdss.setRegisterValue("register12", "resets", RESET_LOGIC);
    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);
    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    // SER
    tdss.setRegisterValue("register12", "resets", RESET_SER);
    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    tdss.setRegisterValue("register12", "resets", RESET_OFF);

    nsw::hw::SCA::sendI2cMasterSingle(getConnection(), getScaAddress(), tdss, "register12");

    ERS_LOG("SCA " << getScaAddress() << " TDS " << tdss.getName() << " readback register 14:");

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
  nsw::hw::SCA::sendI2cMasterSingle(getConnection(),
                                    fmt::format("{}.{}", getScaAddress(), m_config.getName()),
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
    fmt::format("{}.{}.{}", getScaAddress(), m_config.getName(), registerName);
  return nsw::hw::SCA::readI2c(getConnection(), fullNodeName, sizeInBytes);
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

  nsw::hw::SCA::sendI2cMasterConfig(getConnection(), getScaAddress(), config);
}

void nsw::hw::TDS::writeValue(const std::string& name, const unsigned int value) const
{
  writeValues({std::pair{name, value}});
}

unsigned int nsw::hw::TDS::readValue(const std::string& name) const
{
  return std::cbegin(readValues(std::array{name}))->second;
}

std::map<std::string, unsigned int> nsw::hw::TDS::readValues(
  const std::span<const std::string> names) const
{
  // Lambda to const init the register names. Pulls in the names and asks the config
  // converter for the registers for the given names
  const auto regNames = [&names] {
    std::unordered_set<std::string> result{};
    for (const auto& name : names) {
      result.merge(ConfigConverter<ConfigConversionType::TDS>::getRegsForValue(name));
    }
    return result;
  }();

  std::map<std::string, __uint128_t> registerValues{};
  std::transform(std::cbegin(regNames),
                 std::cend(regNames),
                 std::inserter(registerValues, std::begin(registerValues)),
                 [&](const auto& regName) -> std::pair<std::string, __uint128_t> {
                   const auto byteVector = readRegister(static_cast<std::uint8_t>(
                     std::distance(std::cbegin(TDS_REGISTERS), TDS_REGISTERS.find(regName))));
                   // byte vector to 128 bit integer conversion
                   __uint128_t result{0};
                   auto counter = byteVector.size();
                   for (const auto byte : byteVector) {
                     result |= static_cast<__uint128_t>(byte) << (NUM_BITS_IN_BYTE * --counter);
                   }
                   return {regName, result};
                 });

  const auto configConverter = ConfigConverter<ConfigConversionType::TDS>(
    transformMapToPtree(ConfigConverter<ConfigConversionType::TDS>::convertRegisterToSubRegister(
      transformMapToPtree(registerValues), names)),
    ConfigType::REGISTER_BASED);
  const auto values = configConverter.getValueBasedConfig();

  return transformPtreetoMap<unsigned int>(values);
}
