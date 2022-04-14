#include "NSWConfiguration/hw/Router.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>
#include <iomanip>
#include <fmt/core.h>

nsw::hw::Router::Router(OpcManager& manager, const RouterConfig& config) :
  ScaAddressBase(config.getAddress()),
  OpcConnectionBase(manager, config.getOpcServerIp(), config.getAddress()),
  m_config(config)
{
  m_name = fmt::format("{}/{}", getOpcServerIp(), getScaAddress());
}

bool nsw::hw::Router::readGPIO(const std::string& name) const
{
  const auto addr = fmt::format("{}.gpio.{}", getScaAddress(), name);
  return nsw::hw::SCA::readGPIO(getConnection(), addr);
}

void nsw::hw::Router::sendGPIO(const std::string& name, const bool val) const
{
  const auto addr = fmt::format("{}.gpio.{}", getScaAddress(), name);
  nsw::hw::SCA::sendGPIO(getConnection(), addr, val);
}

void nsw::hw::Router::sendAndReadbackGPIO(const std::string& name, const bool val) const
{
  sendGPIO(name, val);
  if (readGPIO(name) != val) {
    const auto msg = fmt::format("{} readback wrong for {}", m_name, name);
    ers::warning(nsw::RouterHWIssue(ERS_HERE, msg));
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
  ers::warning(nsw::RouterHWIssue(ERS_HERE, msg));
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
  const auto scaid = static_cast<unsigned>(id());

  // Announce
  ERS_LOG (fmt::format("{}: ID (sector) = {:#06b}", m_name, idSector()));
  ERS_LOG (fmt::format("{}: ID (layer)  = {:#05b}", m_name, idLayer()));
  ERS_LOG (fmt::format("{}: ID (endcap) = {:#03b}", m_name, idEndcap()));
  ERS_INFO(fmt::format("{}: -> ID = {:#010b} = {:#x} = {}", m_name, scaid, scaid, scaid));

  // Set ID
  for (std::size_t bit = 0; bit < NUM_BITS_IN_BYTE; bit++) {
    const bool this_bit = ((scaid >> bit) & 0b1);
    const auto gpio = fmt::format("routerId{}", bit);
    sendAndReadbackGPIO(gpio, this_bit);
  }
}

std::uint8_t nsw::hw::Router::id() const
{
  idCheck();
  constexpr static std::uint8_t shiftSector = 4;
  constexpr static std::uint8_t shiftLayer = 1;
  return static_cast<std::uint8_t>((idSector() << shiftSector) + (idLayer() << shiftLayer) + idEndcap());
}

std::uint8_t nsw::hw::Router::idEndcap() const
{
  const auto endcap = std::string{Sector().front()};
  if (endcap != "A" && endcap != "C") {
    idCrash();
  }
  return endcap == "A" ? 0 : 1;
}

std::uint8_t nsw::hw::Router::idSector() const
{
  const auto sect = std::stoul(Sector().substr(1, 2));
  if (sect < nsw::MIN_SECTOR_ID || sect > nsw::MAX_SECTOR_ID) {
    idCrash();
  }
  return static_cast<std::uint8_t>(sect - 1);
}

std::uint8_t nsw::hw::Router::idLayer() const
{
  const auto layer = std::stoul(std::string{getScaAddress().back()});
  if (layer < nsw::MIN_LAYER_ID || layer > nsw::MAX_LAYER_ID) {
    idCrash();
  }
  return static_cast<std::uint8_t>(layer);
}

void nsw::hw::Router::idCheck() const
{
  const auto len = getScaAddress().size();
  if (len != m_old_convention.size() && 
      len != m_convention.size()) {
    idCrash();
  }
  if (std::string{getScaAddress().at(len-2)} != "L") {
    idCrash();
  }
}

void nsw::hw::Router::idCrash() const
{
  const auto msg = fmt::format("{} ({} or {}): {}",
    m_name_error, m_old_convention, m_convention, getScaAddress());
  ers::error(nsw::RouterHWIssue(ERS_HERE, msg));
}
