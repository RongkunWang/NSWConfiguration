#include "NSWConfiguration/hw/STGCTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"
using namespace std::chrono_literals;

nsw::hw::STGCTP::STGCTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_scaAddressFPGA(fmt::format("{}.I2C_0.bus0", getScaAddress())),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress())),
  m_skippedReg(SkipRegisters())
{
}

void nsw::hw::STGCTP::writeConfiguration() const
{
  doReset();
  writeAndReadbackRegister(nsw::stgctp::REG_SECTOR,                   getSector(),                      nsw::stgctp::MASK_SECTOR);
  writeAndReadbackRegister(nsw::stgctp::REG_IGNORE_PADS,              getIgnorePads(),                  nsw::stgctp::MASK_IGNORE_PADS);
  writeAndReadbackRegister(nsw::stgctp::REG_IGNORE_MM,                getIgnoreMM(),                    nsw::stgctp::MASK_IGNORE_MM);
  writeAndReadbackRegister(nsw::stgctp::REG_STGC_MM_DISABLE,          m_config.get("MMDisable", false), nsw::stgctp::MASK_STGC_MM_DISABLE);
  writeAndReadbackRegister(nsw::stgctp::REG_DISABLE_NSWMON,           getDisableNSWMON(),               nsw::stgctp::MASK_DISABLE_NSWMON);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_OPENING_OFFSET,       getL1AOpeningOffset(),            nsw::stgctp::MASK_L1A_OPENING_OFFSET);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_REQUEST_OFFSET,       getL1ARequestOffset(),            nsw::stgctp::MASK_L1A_REQUEST_OFFSET);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_CLOSING_OFFSET,       getL1AClosingOffset(),            nsw::stgctp::MASK_L1A_CLOSING_OFFSET);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_TIMEOUT_WINDOW,       getL1ATimeoutWindow(),            nsw::stgctp::MASK_L1A_TIMEOUT_WINDOW);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_PAD_EN,               getL1APadEnable(),                nsw::stgctp::MASK_L1A_PAD_EN);
  writeAndReadbackRegister(nsw::stgctp::REG_L1A_MERGE_EN,             getL1AMergeEnable(),              nsw::stgctp::MASK_L1A_MERGE_EN);
  writeRegister           (nsw::stgctp::REG_STGC_GLOSYNC_BCID_OFFSET, getGlobalSyncBcidOffset());
  writeAndReadbackRegister(nsw::stgctp::REG_BUSY,                     getBusy(),                 nsw::stgctp::MASK_BUSY);
  writeAndReadbackRegister(nsw::stgctp::REG_MON_DISABLE,              getMonitoringDisable(),    nsw::stgctp::MASK_MON_DISABLE);
  writeAndReadbackRegister(nsw::stgctp::REG_NSW_MON_LIMIT,            getNSWMONLimit(),          nsw::stgctp::MASK_NSW_MON_LIMIT);
  writeAndReadbackRegister(nsw::stgctp::REG_MON_LIMIT,                getMonitoringLimit(),      nsw::stgctp::MASK_MON_LIMIT);
  writeAndReadbackRegister(nsw::stgctp::REG_MM_NSW_MON_EN,            getMMNSWMONEnable(),       nsw::stgctp::MASK_MM_NSW_MON_EN);
  writeAndReadbackRegister(nsw::stgctp::REG_SMALL_SECTOR,             getSmallSector(),          nsw::stgctp::MASK_SMALL_SECTOR);
  writeAndReadbackRegister(nsw::stgctp::REG_NO_STRETCH,               getNoStretch(),            nsw::stgctp::MASK_NO_STRETCH);
  for (const auto& [reg, val]: readConfiguration()) {
    ERS_LOG(fmt::format("{} Reg {:#04x}: val = {:#010x}", m_name, reg, val));
  }
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::STGCTP::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  for (const auto& [reg, mask]: nsw::stgctp::REGS) {
    try {
      result.emplace(reg, readRegister(reg, mask));
    } catch (std::exception & ex) {
      ERS_INFO(fmt::format("{}: failed to read reg {:#04x}", m_name, reg));
      break;
    }
  }
  return result;
}

void nsw::hw::STGCTP::writeRegister(const std::uint32_t regAddress,
                                    const std::uint32_t value) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {:#04x}", m_name, regAddress));
    return;
  }
  ERS_LOG(fmt::format("{}: writing to {:#04x} with {:#010x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeRegister(getConnection(), m_scaAddressFPGA, regAddress, value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::uint32_t regAddress,
                                            const std::uint32_t mask) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip reading {:#04x}, return dummy value {:#x}", m_name, regAddress, nsw::DEADBEEF));
    return nsw::DEADBEEF;
  }
  return nsw::hw::SCAX::readRegister(getConnection(), m_scaAddressFPGA, regAddress, mask);
}

void nsw::hw::STGCTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                               const std::uint32_t value,
                                               const std::uint32_t mask) const
{
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {:#04x}", m_name, regAddress));
    return;
  }
  ERS_LOG(fmt::format("{}: writing to {:#04x} with {:#010x} (mask: {:#010x})", m_name, regAddress, value, mask));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), m_scaAddressFPGA, regAddress, value, mask);
}

std::set<std::uint8_t> nsw::hw::STGCTP::SkipRegisters() const
{
  const auto key = "SkipRegisters";
  if (m_config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(m_config, key);
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
    writeRegister(nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_ENABLE);
    nsw::snooze(1s);
    writeRegister(nsw::stgctp::REG_RST_RX, nsw::stgctp::RST_RX_DISABLE);
    nsw::snooze(2s);
    ERS_LOG(fmt::format("{}: toggling TX resets", getName()));
    writeRegister(nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_ENABLE);
    nsw::snooze(1s);
    writeRegister(nsw::stgctp::REG_RST_TX, nsw::stgctp::RST_TX_DISABLE);
    nsw::snooze(2s);
  }
}
