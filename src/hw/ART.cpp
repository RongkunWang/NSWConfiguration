#include "NSWConfiguration/hw/ART.h"

#include <iterator>
#include <stdexcept>
#include <chrono>
#include <string>

#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/SCAInterface.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/Literals.h"

using namespace std::chrono_literals;

nsw::hw::ART::ART(const nsw::ADDCConfig& config, const std::size_t numArt) :
  m_config(config.getART(numArt)),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_numArt(numArt)
{}

void nsw::hw::ART::resetArt(const OpcClientPtr& opcConnection) const
{
  // init_sca_rst_gpio
  ERS_DEBUG(1, "ART reset, step 0...");
  DeviceInterface::SCA::sendGPIO(opcConnection,
                                 fmt::format("{}.gpio.{}SRstn", m_scaAddress, m_config.getName()),
                                 RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  DeviceInterface::SCA::sendGPIO(opcConnection,
                                 fmt::format("{}.gpio.{}CRstn", m_scaAddress, m_config.getName()),
                                 RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  DeviceInterface::SCA::sendGPIO(opcConnection,
                                 fmt::format("{}.gpio.{}Rstn", m_scaAddress, m_config.getName()),
                                 RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::resetGbtx(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "GBT reset...");
  const auto address = fmt::format("{}.gpio.gbtx{}Rstn", m_scaAddress, m_numArt);
  DeviceInterface::SCA::sendGPIO(opcConnection, address, RESET_ACTIVE_LOW);
  std::this_thread::sleep_for(10ms);
  DeviceInterface::SCA::sendGPIO(opcConnection, address, RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::configureGbtx(const OpcClientPtr& opcConnection) const
{
  //
  // SCA supports up to 16-byte payloads
  //
  ERS_DEBUG(1, "GBT configuration");
  const std::size_t chunklen = 16;
  const auto address = fmt::format("{}.{}", m_scaAddress, m_config.getNameGbtx());
  std::vector<uint8_t> datas = {};
  for (size_t i = 0; i < m_gbtxConfigurationData.size(); i++) {
    if (datas.empty()) {
      datas.push_back(static_cast<std::uint8_t>(i) & 0xff_u8);
      datas.push_back(static_cast<std::uint8_t>(i) >> 8_u8);
    }
    datas.push_back(m_gbtxConfigurationData.at(i));
    if (datas.size() == chunklen || i + 1 == m_gbtxConfigurationData.size()) {
      DeviceInterface::SCA::sendI2c(opcConnection, address, datas);
      datas.clear();
    }
  }
  // To fix 6/2 bit split from GBTx bug, try moving the 40 MHz TTC clock as suggested by DM.
  std::array gbtxData{9_u8, 0_u8, 8_u8};
  DeviceInterface::SCA::sendI2cRaw(opcConnection, address, gbtxData.data(), gbtxData.size());
  std::this_thread::sleep_for(10ms);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::resetArt2(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART reset");
  const auto addressPrefix = fmt::format("{}.gpio.{}", m_scaAddress, m_config.getName());
  ERS_DEBUG(1, "ART reset: " << addressPrefix);
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "Rstn"), RESET_ACTIVE_LOW);
  std::this_thread::sleep_for(10ms);  // reset cfg
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "Rstn"), RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "SRstn"), RESET_ACTIVE_LOW);
  std::this_thread::sleep_for(10ms);  // reset i2c
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "SRstn"), RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "CRstn"), RESET_ACTIVE_LOW);
  std::this_thread::sleep_for(10ms);  // reset core
  DeviceInterface::SCA::sendGPIO(
    opcConnection, fmt::format("{}{}", addressPrefix, "CRstn"), RESET_INACTIVE_HIGH);
  std::this_thread::sleep_for(10ms);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::configureCommonArt(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART common config");
  for (const auto& [section, config] :
       {std::make_pair("Core", m_config.core), std::make_pair("Ps", m_config.ps)}) {
    const auto address =
      fmt::format("{0}.{1}{2}.{1}{2}", m_scaAddress, m_config.getName(), section);
    const auto bitstream = config.getBitstreamMap();
    ERS_DEBUG(1, "ART common config " << address);
    for (const auto& [registerAddress, registerValue] : bitstream) {
      const std::array artData{static_cast<uint8_t>(std::stoi(registerAddress)),
                               static_cast<uint8_t>(std::stoi(registerValue, nullptr, 2))};
      DeviceInterface::SCA::sendI2cRaw(opcConnection, address, artData.data(), artData.size());
    }
  }
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::maskArt(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART mask");
  auto address = fmt::format("{0}.{1}Core.{1}Core", m_scaAddress, m_config.getName());
  for (const auto reg : m_artCoreRegisters) {
    std::array artData{reg, 0xFF_u8};
    DeviceInterface::SCA::sendI2cRaw(opcConnection, address, artData.data(), artData.size());
  }
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::trainGbtx(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "Train GBTx");

  const auto addressCore = fmt::format("{0}.{1}Core.{1}Core", m_scaAddress, m_config.getName());
  const auto addressGbtx = fmt::format("{0}.gbtx{1}.gbtx{1}", m_scaAddress, m_numArt);

  ERS_DEBUG(1, "ART pattern mode");
  for (std::size_t i = 0; i < m_artRegisters.size(); i++) {
    const std::array artData{m_artRegisters.at(i), m_artRegisterValues.at(i)};
    DeviceInterface::SCA::sendI2cRaw(opcConnection, addressCore, artData.data(), artData.size());
  }
  ERS_DEBUG(1, "-> done");

  ERS_DEBUG(1, "GBTx training mode");
  const std::array gbtxData{62_u8, 0_u8, 0x15_u8};
  DeviceInterface::SCA::sendI2cRaw(opcConnection, addressGbtx, gbtxData.data(), gbtxData.size());
  ERS_DEBUG(1, "-> done");

  ERS_DEBUG(1, "GBTx eport enable");
  for (const auto value : m_gbtxEportRegisters) {
    const std::array gbtxData{static_cast<std::uint8_t>(value & 0xff_u8),
                              static_cast<std::uint8_t>(value >> 8_u8),
                              0xff_u8};
    DeviceInterface::SCA::sendI2cRaw(opcConnection, addressGbtx, gbtxData.data(), gbtxData.size());
  }
  ERS_DEBUG(1, "-> done");

  // Pause
  std::this_thread::sleep_for(1s);

  ERS_DEBUG(1, "GBTx eport disable");
  for (const auto value : m_gbtxEportRegisters) {
    const std::array gbtxData{static_cast<std::uint8_t>(value & 0xff_u8),
                              static_cast<std::uint8_t>(value >> 8_u8),
                              0x00_u8};
    DeviceInterface::SCA::sendI2cRaw(opcConnection, addressGbtx, gbtxData.data(), gbtxData.size());
  }
  ERS_DEBUG(1, "-> done");

  // ART default mode
  ERS_DEBUG(1, "ART default mode");
  for (const auto reg : m_artRegisters) {
    const auto bitstream = m_config.core.getBitstreamMap();
    for (const auto& [registerAddress, registerValue] : bitstream) {
      if (reg == static_cast<uint8_t>(std::stoi(registerAddress))) {
        std::array artData{static_cast<uint8_t>(std::stoi(registerAddress)),
                           static_cast<uint8_t>(std::stoi(registerValue, nullptr, 2))};

        DeviceInterface::SCA::sendI2cRaw(
          opcConnection, addressCore, artData.data(), artData.size());
        break;
      }
    }
  }
  ERS_DEBUG(1, "-> done");
  ERS_DEBUG(1, " -> done (Train GBTx)");
}

void nsw::hw::ART::failsafeModeArt(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART flag mode (failsafe or no)");
  ERS_DEBUG(1, "Failsafe for: " << m_config.getName() << ": " << m_config.failsafe());
  const auto address = fmt::format("{0}.{1}Core.{1}Core", m_scaAddress, m_config.getName());

  const std::array dataFirst{3_u8, m_config.failsafe() ? 0x06_u8 : 0x0E_u8};
  DeviceInterface::SCA::sendI2cRaw(opcConnection, address, dataFirst.data(), dataFirst.size());

  const std::array dataSecond{4_u8, m_config.failsafe() ? 0x27_u8 : 0x3F_u8};
  DeviceInterface::SCA::sendI2cRaw(opcConnection, address, dataSecond.data(), dataSecond.size());
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::unmaskArt(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART unmask");
  const auto address = fmt::format("{0}.{1}Core.{1}Core", m_scaAddress, m_config.getName());
  for (const auto reg : m_artCoreRegisters) {
    const auto bitstream = m_config.core.getBitstreamMap();
    for (const auto& [registerAddress, registerValue] : bitstream) {
      if (reg == static_cast<uint8_t>(std::stoi(registerAddress))) {
        std::array artData{static_cast<uint8_t>(std::stoi(registerAddress)),
                           static_cast<uint8_t>(std::stoi(registerValue, nullptr, 2))};
        DeviceInterface::SCA::sendI2cRaw(opcConnection, address, artData.data(), artData.size());
        break;
      }
    }
  }
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::configureClockArt(const OpcClientPtr& opcConnection) const
{
  ERS_DEBUG(1, "ART BCRCLK phase");
  const auto phaseEnd = 4_u8;
  const auto address = fmt::format("{}.{}", m_scaAddress, m_config.getNameGbtx());
  for (auto phase = 0_u8; phase <= phaseEnd; phase++) {
    // coarse phase
    const std::array dataGbtx{11_u8, 0_u8, phase};
    DeviceInterface::SCA::sendI2cRaw(opcConnection, address, dataGbtx.data(), dataGbtx.size());
  }
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::writeConfiguration()
{
  ERS_LOG(m_scaAddress << " Begin configuration...");
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);

  resetArt(opcConnection);
  resetGbtx(opcConnection);
  configureGbtx(opcConnection);
  resetArt2(opcConnection);
  configureCommonArt(opcConnection);
  maskArt(opcConnection);
  trainGbtx(opcConnection);
  failsafeModeArt(opcConnection);
  unmaskArt(opcConnection);
  configureClockArt(opcConnection);

  ERS_LOG(m_scaAddress << " Configuration done.");
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::ART::readConfiguration()
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::ART::writeRegister(const std::uint8_t registerId, const std::uint32_t value) {}

std::vector<std::uint8_t> nsw::hw::ART::readRegister([[maybe_unused]] const std::uint8_t registerId)
{
  throw std::logic_error("Not implemented");
}
