#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>
#include <iomanip>
#include <fmt/core.h>

nsw::hw::Router::Router(OpcManager& manager, const RouterConfig& config) :
  m_opcManager{manager},
  m_config(config),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress())
{
  m_name = fmt::format("{}/{}", m_opcserverIp, m_scaAddress);
}

bool nsw::hw::Router::readGPIO(const std::string& name) const
{
  const auto addr = m_scaAddress + ".gpio." + name;
  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);
  return nsw::hw::SCA::readGPIO(opcConnection, addr);
}

void nsw::hw::Router::sendGPIO(const std::string& name, const bool val) const
{
  const auto addr = m_scaAddress + ".gpio." + name;
  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);
  nsw::hw::SCA::sendGPIO(opcConnection, addr, val);
}

void nsw::hw::Router::sendAndReadbackGPIO(const std::string& name, const bool val) const
{
  sendGPIO(name, val);
  if (readGPIO(name) != val) {
    const auto msg = fmt::format("{} readback wrong for {}", m_name, name);
    ers::warning(nsw::RouterConfigIssue(ERS_HERE, msg));
  }
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
  writeSoftResetAndCheckGPIO();
  writeSetSCAID();
}

void nsw::hw::Router::writeSoftResetAndCheckGPIO() const
{
  for (std::size_t rst = 0; rst < nsw::router::MAX_RESETS; ++rst) {
    writeSoftReset();
    if (checkGPIOsAFewTimes()) {
      return;
    }
    ERS_INFO(fmt::format("Trying another reset for {}", m_name));
    nsw::snooze(nsw::router::PAUSE_BETWEEN_RESETS);
  }

  const auto msg = fmt::format("{} configuration failed", m_name);
  ers::warning(nsw::RouterConfigIssue(ERS_HERE, msg));
}

void nsw::hw::Router::writeSoftReset(const std::chrono::seconds reset_hold,
                                     const std::chrono::seconds reset_sleep) const
{
  ERS_LOG(m_name << ": toggling soft reset");

  // Set Router control mode to SCA mode: Line 17 in excel
  sendAndReadbackGPIO("ctrlMod0", false);
  sendAndReadbackGPIO("ctrlMod1", false);

  // Enable soft reset: Line 11 in excel
  sendAndReadbackGPIO("softReset", true);
  nsw::snooze(reset_hold);

  // Disable soft reset
  sendAndReadbackGPIO("softReset", false);
  nsw::snooze(reset_sleep);
}

bool nsw::hw::Router::checkGPIOsAFewTimes() const
{
  for (std::size_t checks = 0; checks < nsw::router::MAX_GPIO_CHECKS; ++checks) {
    if (checkGPIOs()) {
      return true;
    }
    ERS_INFO(fmt::format("Waiting for {}", m_name));
    nsw::snooze(nsw::router::PAUSE_AFTER_RESET);
  }
  return false;
}

bool nsw::hw::Router::checkGPIOs() const
{
  bool ok = true;

  // Read SCA IO status back: Line 6 & 8 in excel
  // (only need to match with star mark bits)
  for (const auto& [name, exp] : m_gpio_checks) {
    const auto long_name = fmt::format("{}/{}", m_name, name);
    const bool obs = readGPIO(std::string(name));
    const bool yay = obs == exp;
    const auto msg = fmt::format("{:<54} :: Expected = {}, Observed = {} -> {}",
                                 long_name, exp, obs, (yay ? "Good" : "Bad"));
    if (yay) {
      ERS_LOG(msg);
    } else {
      ERS_INFO(msg);
      ok = false;
    }
  }

  return ok;
}

void nsw::hw::Router::writeSetSCAID() const
{
  // Get ID from config object
  const auto scaid = static_cast<unsigned>(getConfig().id());

  // Announce
  ERS_LOG (fmt::format("{}: ID (sector) = {:#06b}", m_name, getConfig().id_sector()));
  ERS_LOG (fmt::format("{}: ID (layer)  = {:#05b}", m_name, getConfig().id_layer()));
  ERS_LOG (fmt::format("{}: ID (endcap) = {:#03b}", m_name, getConfig().id_endcap()));
  ERS_INFO(fmt::format("{}: -> ID = {:#010b} = {:#x} = {}", m_name, scaid, scaid, scaid));

  // Set ID
  for (std::size_t bit = 0; bit < NUM_BITS_IN_BYTE; bit++) {
    const bool this_bit = ((scaid >> bit) & 0b1);
    const auto gpio = fmt::format("routerId{}", bit);
    sendAndReadbackGPIO(gpio, this_bit);
  }
}
