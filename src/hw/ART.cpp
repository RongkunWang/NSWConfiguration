#include "NSWConfiguration/hw/ART.h"

#include <stdexcept>

#include <fmt/format.h>

#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/hw/SCAInterface.h"

using namespace std::chrono_literals;

nsw::hw::ART::ART(nsw::OpcManager& manager, const nsw::ADDCConfig& config, const std::size_t numArt) :
  ScaAddressBase(config.getAddress()),
  OpcConnectionBase(manager, config.getOpcServerIp(), config.getAddress()),
  m_config(config.getART(numArt)),
  m_iArt(numArt)
{}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::ART::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::ART::writeConfiguration() const
{
  const auto sca_addr                    = getScaAddress();

  ERS_LOG(fmt::format("{} Begin configuration... (i_art = {})", sca_addr, m_iArt));

  initART();
  resetGBTx();
  configGBTx();
  resetART();
  configART();
  maskART();
  trainGBTx();
  failsafeMode();
  unmaskART();
  adjustPhaseART();

}

void nsw::hw::ART::setArtGpio(const std::string& gpio, const bool state) const
{
  // GPIO: <sca>.gpio.art<X><field>
  const auto gpioArtName  = fmt::format("{}.gpio.{}", getScaAddress(), getName());
  nsw::hw::SCA::sendGPIO(getConnection(), gpioArtName + gpio,  static_cast<unsigned>(state));
  std::this_thread::sleep_for(10000us);
}

void nsw::hw::ART::toggleArtGpio(const std::string& gpio) const
{
  constexpr static auto off(false);
  constexpr static auto on(true);

  setArtGpio(gpio, off);
  setArtGpio(gpio, on);
}


void nsw::hw::ART::initART() const
{
  // GPIO: <sca>.gpio.art<X><field>, X=0,1
  const auto gpioArtName  = fmt::format("{}.gpio.{}", getScaAddress(), getName());
  ERS_DEBUG(1, "ART reset, step 0... for " << gpioArtName);
  constexpr static auto on{true};
  // raise i2c to 1
  setArtGpio("SRstn", on);
  // raise core to 1
  setArtGpio("CRstn", on);
  // raise cfg to 1
  setArtGpio("Rstn", on);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::resetGBTx() const
{
  ERS_DEBUG(1, "reset corresponding GBT ...");
  // GPIO: <sca>.gpio.gbtx<X><field>, X=0,1
  const auto gpioGbtxName = fmt::format("{}.gpio.gbtx{}", getScaAddress(), m_iArt);
  nsw::hw::SCA::sendGPIO(getConnection(), gpioGbtxName + "Rstn", 0); 
  std::this_thread::sleep_for(10000us);
  nsw::hw::SCA::sendGPIO(getConnection(), gpioGbtxName + "Rstn", 1); 
  std::this_thread::sleep_for(10000us);
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::configGBTx() const
{
  // SCA supports up to 16-byte payloads
  // I2C: <sca>.gbtx<X>.gbtx<X>
  const auto gbtxI2CName = getAddressGbtx();
  ERS_DEBUG(1, "GBT configuration: " << gbtxI2CName);
  constexpr static std::size_t chunklen = 16;
  std::vector<std::uint8_t> datas {};
  for (std::size_t i = 0; i < m_GBTx_ConfigurationData.size(); i++) {
    if (datas.empty()) {
      datas.push_back(static_cast<std::uint8_t>(i & 0xff) );
      datas.push_back(static_cast<std::uint8_t>(i >> 8) );
    }
    datas.push_back(m_GBTx_ConfigurationData.at(i));
    if (datas.size() == chunklen || i+1 == m_GBTx_ConfigurationData.size()) {
      nsw::hw::SCA::sendI2c(getConnection(), gbtxI2CName, datas);
      datas.clear();
    }
    // To fix 6/2 bit split from GBTx bug, try moving the 40 MHz TTC clock as suggested by DM.
    // see more in https://indico.cern.ch/event/867897/
    writeGBTXRegister(9, 8);
    std::this_thread::sleep_for(10000us);
  }
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::resetART() const
{
  // GPIO: <sca>.gpio.art<X><field>
  const auto gpioArtName  = fmt::format("{}.gpio.{}", getScaAddress(), getName());
  ERS_DEBUG(1, "ART reset: " << gpioArtName);
  toggleArtGpio("Rstn");
  toggleArtGpio("SRstn");
  toggleArtGpio("CRstn");
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::configART() const
{
  ERS_DEBUG(1, "ART common config");
  for (const auto& tup : {
        std::make_pair(getNameCore(), m_config.core),
        std::make_pair(getNamePs(),   m_config.ps)}
      ) {
    ERS_DEBUG(1, "ART common config " << tup.first);
    for (const auto& ab : tup.second.getBitstreamMap()) {
      writeARTRegister(tup.first,
          static_cast<uint8_t>(std::stoi(ab.first) ),
          static_cast<uint8_t>(std::stoi(ab.second, nullptr, nsw::BASE_BIN) ));
    }
  }
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::maskART() const
{
  ERS_DEBUG(1, "ART mask");
  for (const auto& reg : m_ARTCoreregisters) {
    writeARTCoreRegister(reg, 0xFF);
  }
  ERS_DEBUG(1, " -> done");
}

void nsw::hw::ART::trainGBTx() const
{
  ERS_DEBUG(1, "Train GBTx");

  ERS_DEBUG(1, "ART pattern mode");
  for (std::size_t i=0; i < m_ARTregisters.size(); ++i) {
    writeARTCoreRegister(m_ARTregisters.at(i), 
        m_ARTregistervalues.at(i));
  }
  ERS_DEBUG(1, "-> done");

  ERS_DEBUG(1, "Set GBTx to training mode");
  writeGBTXRegister(REG_GBTX_TRAIN_MODE, GBTX_TRAIN_MODE_TRAINING);
  ERS_DEBUG(1, "-> done");

  ERS_DEBUG(1, "enable GBTx eport training");
  for (const auto& val : m_GBTx_eport_registers) {
    writeGBTXRegister(val, 0xff);
  }
  ERS_DEBUG(1, "-> done");

  // Pause
  std::this_thread::sleep_for(1000000us);

  ERS_DEBUG(1, "disable GBTx eport training");
  for (const auto& val : m_GBTx_eport_registers) {
    writeGBTXRegister(val, 0x00);
  }
  ERS_DEBUG(1, "-> done");

  ERS_DEBUG(1, "ART default mode");
  for (const auto& reg : m_ARTregisters) {
    const auto addr_bitstr = m_config.core.getBitstreamMap();
    for (const auto& ab : addr_bitstr) {
      if (reg == static_cast<std::uint8_t>(std::stoi(ab.first) )) {
        writeARTCoreRegister(static_cast<std::uint8_t>(std::stoi(ab.first)),
            static_cast<std::uint8_t>(std::stoi(ab.second, nullptr, 2))
            );
        break;
      }
    } // addr_bitstr
  } // ARTregisters
  ERS_DEBUG(1, " -> done (Train GBTx)");
}

void nsw::hw::ART::failsafeMode() const
{
  ERS_DEBUG(1, "ART flag mode (failsafe or no)");
  ERS_DEBUG(1, "Failsafe for: " << getName() << ": " << m_config.failsafe());
  writeARTCoreRegister(REG_FLAG_MASK,
      m_config.failsafe() ? FLAG_MASK_FAILSAFE    : FLAG_MASK_DEFAULT);
  writeARTCoreRegister(REG_FLAG_PATTERN,
      m_config.failsafe() ? FLAG_PATTERN_FAILSAFE : FLAG_PATTERN_DEFAULT);
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::unmaskART() const
{
  ERS_DEBUG(1, "ART unmask, according to config");
  for (const auto& reg: m_ARTCoreregisters) {
    const auto addr_bitstr = m_config.core.getBitstreamMap();
    for (const auto& ab : addr_bitstr) {
      if (reg == static_cast<uint8_t>( std::stoi(ab.first) )) {
        writeARTCoreRegister(static_cast<uint8_t>(std::stoi(ab.first)),
            static_cast<uint8_t>(std::stoi(ab.second, nullptr, 2))
            );
        break;
      }
    }
  }
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::adjustPhaseART() const
{
  ERS_DEBUG(1, "ART BCRCLK phase");
  constexpr static std::uint8_t phase_end = 4;
  std::uint8_t phase = 0;
  while (phase <= phase_end) {
    // coarse phase
    writeGBTXRegister(REG_GBTX_TTC_COARSE_DELAY, phase);
    phase = phase + 1;
  }
  ERS_DEBUG(1, "-> done");
}

void nsw::hw::ART::writeGBTXRegister(const std::uint32_t regAddress,
                                     const std::uint8_t value) const
{
  // TODO: once GBTx config is merged as a HW interface, this can be modified.
  // 2 for address (i), 1 for data
  // address is [1:0], 2 is data
  std::vector<std::uint8_t> gbtx_data({0x0, 0x0, 0x0});
  gbtx_data.at(1) = static_cast<uint8_t>((regAddress >> 8 ) & 0xff);
  gbtx_data.at(0) = static_cast<uint8_t>(regAddress         & 0xff);
  gbtx_data.at(2) = value;
  nsw::hw::SCA::sendI2c(getConnection(), 
      getAddressGbtx(), 
      gbtx_data); 
}

void nsw::hw::ART::writeARTRegister(const std::string& nodeName,
                                    const std::uint8_t regAddress,
                                    const std::uint8_t value) const
{
  // 0 is register slave address, 1 is value(all 8-bit)
  const std::vector<std::uint8_t> art_data({regAddress, value});
  nsw::hw::SCA::sendI2c(getConnection(), 
      getScaAddress() + "." + nodeName, 
      art_data);
}

void nsw::hw::ART::writeARTCoreRegister(const std::uint8_t regAddress, 
                                        const std::uint8_t value) const 
{
  writeARTRegister(
    getNameCore(),
    regAddress, value);
}

void nsw::hw::ART::writeARTPsRegister(const std::uint8_t regAddress, 
    const std::uint8_t value) const 
{
  writeARTRegister(
    getNamePs(),
    regAddress, value);
}

std::vector<std::uint8_t> nsw::hw::ART::readRegister(const std::string& nodeName,
                                                     const std::uint8_t regAddress,
                                                     const std::size_t addressSize,
                                                     const std::size_t numberOfBytes 
      ) const
{
  std::vector<std::uint8_t>  readback{};
  std::vector<std::uint8_t> art_data{regAddress, 0x0};
  readback = nsw::hw::SCA::readI2cAtAddress(getConnection(), 
      getScaAddress() + "." + nodeName, art_data.data(), 
      addressSize,
      numberOfBytes);
  return readback;
}
