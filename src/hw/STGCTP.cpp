#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/Utility.h"
using namespace std::chrono_literals;

nsw::hw::STGCTP::STGCTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress())),
  m_skippedReg(nsw::hw::SCAX::SkipRegistersStr(m_config))
{
}

void nsw::hw::STGCTP::writeConfiguration() const
{
  doReset();
  // writeAndReadbackRegister(nsw::stgctp::REG_SECTOR,                   getSector(),               nsw::stgctp::MASK_SECTOR);
  // writeAndReadbackRegister(nsw::stgctp::REG_IGNORE_PADS,              getIgnorePads(),           nsw::stgctp::MASK_IGNORE_PADS);
  // writeAndReadbackRegister(nsw::stgctp::REG_IGNORE_MM,                getIgnoreMM(),             nsw::stgctp::MASK_IGNORE_MM);
  // writeAndReadbackRegister(nsw::stgctp::REG_DISABLE_NSWMON,           getDisableNSWMON(),        nsw::stgctp::MASK_DISABLE_NSWMON);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_OPENING_OFFSET,       getL1AOpeningOffset(),     nsw::stgctp::MASK_L1A_OPENING_OFFSET);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_REQUEST_OFFSET,       getL1ARequestOffset(),     nsw::stgctp::MASK_L1A_REQUEST_OFFSET);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_CLOSING_OFFSET,       getL1AClosingOffset(),     nsw::stgctp::MASK_L1A_CLOSING_OFFSET);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_TIMEOUT_WINDOW,       getL1ATimeoutWindow(),     nsw::stgctp::MASK_L1A_TIMEOUT_WINDOW);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_PAD_EN,               getL1APadEnable(),         nsw::stgctp::MASK_L1A_PAD_EN);
  // writeAndReadbackRegister(nsw::stgctp::REG_L1A_MERGE_EN,             getL1AMergeEnable(),       nsw::stgctp::MASK_L1A_MERGE_EN);
  // writeAndReadbackRegister(nsw::stgctp::REG_STGC_GLOSYNC_BCID_OFFSET, getGlobalSyncBcidOffset(), nsw::stgctp::MASK_STGC_GLOSYNC_BCID_OFFSET);
  // writeAndReadbackRegister(nsw::stgctp::REG_BUSY,                     getBusy(),                 nsw::stgctp::MASK_BUSY);
  // writeAndReadbackRegister(nsw::stgctp::REG_MON_DISABLE,              getMonitoringDisable(),    nsw::stgctp::MASK_MON_DISABLE);
  // writeAndReadbackRegister(nsw::stgctp::REG_NSW_MON_LIMIT,            getNSWMONLimit(),          nsw::stgctp::MASK_NSW_MON_LIMIT);
  // writeAndReadbackRegister(nsw::stgctp::REG_MON_LIMIT,                getMonitoringLimit(),      nsw::stgctp::MASK_MON_LIMIT);
  // writeAndReadbackRegister(nsw::stgctp::REG_MM_NSW_MON_EN,            getMMNSWMONEnable(),       nsw::stgctp::MASK_MM_NSW_MON_EN);
  // writeAndReadbackRegister(nsw::stgctp::REG_SMALL_SECTOR,             getSmallSector(),          nsw::stgctp::MASK_SMALL_SECTOR);
  // writeAndReadbackRegister(nsw::stgctp::REG_NO_STRETCH,               getNoStretch(),            nsw::stgctp::MASK_NO_STRETCH);
  writeAndReadbackRegister(nsw::stgctp::REG_SECTOR,         getSector());

  for (const auto & [reg, defVal] : nsw::stgctp::registersToWrite) {
    writeAndReadbackRegister(reg, m_config.get(reg, defVal));
  }
  
  for (const auto& [reg, val]: readConfiguration()) {
    ERS_LOG(fmt::format("{} Reg {}: val = {:#010x}", m_name, reg, val));
  }
}

std::map<std::string, std::uint32_t> nsw::hw::STGCTP::readConfiguration() const
{
  std::map<std::string, std::uint32_t> result;
  for (const auto& reg: nsw::stgctp::registersToRead) {
    try {
      result.emplace(reg, readRegister(reg));
    } catch (std::exception & ex) {
      ERS_INFO(fmt::format("{}: failed to read reg {}", m_name, reg));
      break;
    }
  }
  return result;
}

void nsw::hw::STGCTP::writeRegister(const std::string regAddress,
                                    const std::uint32_t value) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {}", m_name, regAddress));
    return;
  }
  nsw::hw::SCAX::writeRegister(getConnection(),
      regAddress,
      value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::string regAddress) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip reading {}, return dummy value {:#x}", m_name, regAddress, nsw::DEADBEEF));
    return nsw::DEADBEEF;
  }
  return nsw::hw::SCAX::readRegister(getConnection(),
      regAddress);
}

void nsw::hw::STGCTP::writeAndReadbackRegister(const std::string regAddress,
                                               const std::uint32_t value) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {}", m_name, regAddress));
    return;
  }
  ERS_LOG(fmt::format("{}: writing to {} with {:#010x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(),
      regAddress,
      value);
}


std::uint32_t nsw::hw::STGCTP::getSector() const
{
  const auto hasGeog = nsw::contains(getScaAddress(), "/");
  const auto hasJson = m_config.count("sector") > 0;
  if (hasGeog and hasJson) {
    const auto msg = "Dont give sector in json if using Geo IDs!";
    ers::error(nsw::STGCTPIssue(ERS_HERE, msg));
  }
  if (hasGeog) {
    return getGeoInfo().sector();
  }
  return m_config.get<std::uint32_t>("sector");
}

void nsw::hw::STGCTP::doReset() const
{
  if (getDoReset()) {
    ERS_LOG(fmt::format("{}: toggling RX resets", getName()));
    nsw::hw::SCAX::writeRegister(getConnection(), nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_ENABLE);
    nsw::snooze(1s);
    nsw::hw::SCAX::writeRegister(getConnection(), nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_DISABLE);
    nsw::snooze(2s);
    ERS_LOG(fmt::format("{}: toggling TX resets", getName()));
    nsw::hw::SCAX::writeRegister(getConnection(), nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_ENABLE);
    nsw::snooze(1s);
    nsw::hw::SCAX::writeRegister(getConnection(), nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_DISABLE);
    nsw::snooze(2s);
  }
}
