#include "NSWConfiguration/hw/Router.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Utility.h"

using namespace std::chrono_literals;

nsw::hw::Router::Router(const RouterConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{}

void nsw::hw::Router::writeConfiguration()
{
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  sendSoftReset(opcConnection);
  checkGPIO(opcConnection);
  setSCAID(opcConnection);
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::Router::readConfiguration()
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::Router::writeRegister([[maybe_unused]] const std::uint8_t registerId,
                                    [[maybe_unused]] const std::uint32_t value)
{}

std::vector<std::uint8_t> nsw::hw::Router::readRegister(
  [[maybe_unused]] const std::uint8_t registerId)
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::Router::setControlMode(const OpcClientPtr& opcConnection,
                                     const ControlMode& mode,
                                     const std::size_t index) const
{
  const auto ctrlModReg = fmt::format("{}.gpio.ctrlMod{}", m_scaAddress, index);
  DeviceInterface::SCA::sendGPIO(opcConnection, ctrlModReg, mode);
  ERS_LOG(ctrlModReg << " " << DeviceInterface::SCA::readGPIO(opcConnection, ctrlModReg));
}

void nsw::hw::Router::toggleSoftReset(const OpcClientPtr& opcConnection,
                                      const std::chrono::seconds& reset_hold,
                                      const std::chrono::seconds& reset_sleep) const
{
  const auto soft_reset = fmt::format("{}.gpio.softReset", m_scaAddress);
  DeviceInterface::SCA::sendGPIO(opcConnection, soft_reset, SoftReset::HI);
  ERS_LOG(soft_reset << " " << DeviceInterface::SCA::readGPIO(opcConnection, soft_reset));
  std::this_thread::sleep_for(reset_hold);
  DeviceInterface::SCA::sendGPIO(opcConnection, soft_reset, SoftReset::LOW);
  ERS_LOG(soft_reset << " " << DeviceInterface::SCA::readGPIO(opcConnection, soft_reset));
  std::this_thread::sleep_for(reset_sleep);
}

void nsw::hw::Router::sendSoftReset(const OpcClientPtr& opcConnection,
                                    const std::chrono::seconds& reset_hold,
                                    const std::chrono::seconds& reset_sleep) const
{
  ERS_LOG(m_scaAddress << ": toggling soft reset");

  setControlMode(opcConnection, ControlMode::SCA, 0);
  setControlMode(opcConnection, ControlMode::SCA, 1);

  toggleSoftReset(opcConnection, reset_hold, reset_sleep);

  // FIXME TODO still in control mode here, desired?
}

void nsw::hw::Router::checkGPIO(const OpcClientPtr& opcConnection) const
{
  bool all_ok = true;
  const std::vector<std::pair<std::string, bool>> check = {{"fpgaConfigOK", true},
                                                           {"mmcmBotLock", true},
                                                           {"fpgaInit", true},
                                                           {"rxClkReady", true},
                                                           {"txClkReady", true},
                                                           {"cpllTopLock", true},
                                                           {"cpllBotLock", true},
                                                           {"mmcmTopLock", true},
                                                           {"semFatalError", false},
                                                           {"masterChannel0", true}};
  for (const auto& [key, exp] : check) {
    const auto bit = fmt::format("{}.gpio.{}", m_scaAddress, key);
    const bool obs = DeviceInterface::SCA::readGPIO(opcConnection, bit);
    bool yay = obs == exp;
    std::stringstream msg;
    msg << std::left << std::setw(34) << bit << " ::"
        << " Expected = " << exp << " Observed = " << obs << " -> " << (yay ? "Good" : "Bad");
    ERS_INFO(msg.str());
    if (!m_config.CrashOnConfigFailure()) {
      yay = true;
    }
    if (key.find("ClkReady") != std::string::npos && !m_config.CrashOnClkReadyFailure()) {
      yay = true;
    }
    if (!yay) {
      all_ok = false;
    }
    if (key == "fpgaConfigOK") {
      std::this_thread::sleep_for(1s);
    }
  }

  // Complain if bad config
  if (!all_ok) {
    const auto msg =
      fmt::format("{}/{} Configuration error. Crashing.", m_opcserverIp, m_scaAddress);
    ERS_INFO(msg);
    throw std::runtime_error(msg);
  }
  std::this_thread::sleep_for(1s);
}

void nsw::hw::Router::setSCAID(const OpcClientPtr& opcConnection) const
{
  // Set ID
  const auto scaid = static_cast<uint>(m_config.id());
  const auto id_sector_str = nsw::bitString(static_cast<uint>(m_config.id_sector()), 4);
  const auto id_layer_str = nsw::bitString(static_cast<uint>(m_config.id_layer()), 3);
  const auto id_endcap_str = nsw::bitString(static_cast<uint>(m_config.id_endcap()), 1);
  ERS_LOG(m_scaAddress << ": ID (sector) = 0b" << id_sector_str);
  ERS_LOG(m_scaAddress << ": ID (layer)  = 0b" << id_layer_str);
  ERS_LOG(m_scaAddress << ": ID (endcap) = 0b" << id_endcap_str);
  ERS_INFO(m_scaAddress << ": -> ID"
                        << " = 0b" << id_sector_str << id_layer_str << id_endcap_str << " = 0x"
                        << std::hex << scaid << std::dec << " = " << scaid);
  for (std::size_t bit = 0; bit < NUM_BITS_IN_BYTE; bit++) {
    const bool this_bit = ((scaid >> bit) & 0b1);
    const auto gpio = fmt::format("{}.gpio.routerId{}", m_scaAddress, bit);
    DeviceInterface::SCA::sendGPIO(opcConnection, gpio, this_bit);
    ERS_LOG(m_scaAddress << ": Set ID bit " << bit << " = " << this_bit << " => Readback = "
                         << DeviceInterface::SCA::readGPIO(opcConnection, gpio));
  }
}
