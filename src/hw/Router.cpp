#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>
#include <iomanip>
#include <fmt/core.h>

nsw::hw::Router::Router(const RouterConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{
  m_name = fmt::format("{}/{}", m_opcserverIp, m_scaAddress);
}

bool nsw::hw::Router::readGPIO(const std::string& name) const
{
  const auto addr = m_scaAddress + ".gpio." + name;
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  return nsw::hw::SCA::readGPIO(opcConnection, addr);
}

void nsw::hw::Router::sendGPIO(const std::string& name, const bool val) const
{
  const auto addr = m_scaAddress + ".gpio." + name;
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendGPIO(opcConnection, addr, val);
}

std::map<std::string, bool> nsw::hw::Router::readConfiguration() const
{
  std::map<std::string, bool> result;
  for (const auto& name: m_ordered_gpios) {
    result.emplace(name, readGPIO(std::string(name)));
  }
  return result;
}

void nsw::hw::Router::writeConfiguration() const
{
  writeSoftReset();
  writeWaitGPIO();
  writeSetSCAID();
}

void nsw::hw::Router::writeSoftReset(const std::chrono::seconds reset_hold,
                                     const std::chrono::seconds reset_sleep) const
{
  ERS_LOG(m_scaAddress << ": toggling soft reset");

  // Set Router control mode to SCA mode: Line 17 in excel
  sendGPIO("ctrlMod0", false);
  sendGPIO("ctrlMod1", false);
  ERS_LOG(m_scaAddress << "/ctrlMod0" << " " << readGPIO("ctrlMod0"));
  ERS_LOG(m_scaAddress << "/ctrlMod1" << " " << readGPIO("ctrlMod1"));

  // Enable soft reset: Line 11 in excel
  sendGPIO("softReset", true);
  ERS_LOG(m_scaAddress << "/softReset" << " " << readGPIO("softReset"));
  std::this_thread::sleep_for(reset_hold);

  // Disable soft reset
  sendGPIO("softReset", false);
  ERS_LOG(m_scaAddress << "/softReset" << " " << readGPIO("softReset"));
  std::this_thread::sleep_for(reset_sleep);
}

void nsw::hw::Router::writeWaitGPIO() const
{
  //
  // check GPIOs as many times as necessary,
  //   but no more times than allowed
  //
  for (size_t checks = 0; checks < nsw::router::MAX_GPIO_CHECKS; ++checks) {
    if (writeCheckGPIO()) {
      return;
    }
    ERS_INFO("Waiting for "
             << getConfig().getOpcServerIp()
             << "/"
             << getConfig().getAddress());
    std::this_thread::sleep_for(nsw::router::RESET_PAUSE);
  }

  //
  // you shouldnt be here!
  // crash if the user requests it
  //
  if (getConfig().CrashOnConfigFailure()) {
    const auto msg = getConfig().getOpcServerIp()
      + "/"
      + getConfig().getAddress()
      + " Configuration error. Crashing.";
    nsw::RouterConfigIssue issue(ERS_HERE, msg.c_str());
    ers::fatal(issue);
    throw issue;
  }

  //
  // otherwise, alert the user
  //
  const auto msg = "Giving up on "
    + getConfig().getOpcServerIp()
    + "/"
    + getConfig().getAddress();
  ERS_INFO(msg);
}

bool nsw::hw::Router::writeCheckGPIO() const
{
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  bool ok = true;

  // Read SCA IO status back: Line 6 & 8 in excel
  // (only need to match with star mark bits)
  for (const auto& [name, exp] : m_gpio_checks) {
    const auto long_name = m_scaAddress + "/" + std::string(name);
    const bool obs = readGPIO(std::string(name));
    const bool yay = obs == exp;
    std::stringstream msg;
    msg << std::left << std::setw(34) << long_name
        << " ::"
        << " Expected = " << exp
        << " Observed = " << obs
        << " -> " << (yay ? "Good" : "Bad");
    if (yay) {
      ERS_LOG(msg.str());
    } else {
      ERS_INFO(msg.str());
      ok = false;
    }
  }

  return ok;
}

void nsw::hw::Router::writeSetSCAID() const
{
  // Get ID from config object
  const auto scaid = static_cast<unsigned>(getConfig().id());
  const auto id_sector_str = nsw::bitString(static_cast<unsigned>(getConfig().id_sector()), 4);
  const auto id_layer_str  = nsw::bitString(static_cast<unsigned>(getConfig().id_layer()),  3);
  const auto id_endcap_str = nsw::bitString(static_cast<unsigned>(getConfig().id_endcap()), 1);

  // Announce
  ERS_LOG (m_scaAddress << ": ID (sector) = 0b" << id_sector_str);
  ERS_LOG (m_scaAddress << ": ID (layer)  = 0b" << id_layer_str);
  ERS_LOG (m_scaAddress << ": ID (endcap) = 0b" << id_endcap_str);
  ERS_INFO(m_scaAddress << ": -> ID"
           << " = 0b" << id_sector_str << id_layer_str << id_endcap_str
           << " = 0x" << std::hex << scaid << std::dec
           << " = "   << scaid);

  // Set ID
  for (std::size_t bit = 0; bit < NUM_BITS_IN_BYTE; bit++) {
    const bool this_bit = ((scaid >> bit) & 0b1);
    const auto gpio = "routerId" + std::to_string(bit);
    sendGPIO(gpio, this_bit);
    ERS_LOG(m_scaAddress
            << ": Set ID bit " << bit
            << " = " << this_bit
            << " => Readback = " << readGPIO(gpio));
  }
}
