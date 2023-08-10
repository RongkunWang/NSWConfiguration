#include "NSWConfiguration/hw/MMTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::MMTP::MMTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_busAddress(fmt::format("{}.I2C_0.bus0", getScaAddress())),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress()))
{
}

void nsw::hw::MMTP::writeConfiguration(bool doAlignArtGbtx) const
{
  std::vector<std::pair<uint8_t, uint32_t> > list_of_messages = {
    {nsw::mmtp::REG_ADDC_EMU_DISABLE,         nsw::mmtp::ADDC_EMU_DISABLE},
    {nsw::mmtp::REG_L1A_OPENING_OFFSET,       L1AOpeningOffset()},
    {nsw::mmtp::REG_L1A_REQUEST_OFFSET,       L1ARequestOffset()},
    {nsw::mmtp::REG_L1A_CLOSING_OFFSET,       L1AClosingOffset()},
    {nsw::mmtp::REG_L1A_TIMEOUT_WINDOW,       L1ATimeoutWindow()},
    {nsw::mmtp::REG_L1A_BUSY_THRESHOLD,       m_config.get<std::uint32_t>("L1ABusyThreshold", 7)},
    {nsw::mmtp::REG_LOCAL_BCID_OFFSET,        m_config.get<std::uint32_t>("LocalBcidOffset", 37)},
    {nsw::mmtp::REG_L1A_CONTROL,              nsw::mmtp::L1A_RESET_ENABLE},
    {nsw::mmtp::REG_L1A_CONTROL,              nsw::mmtp::L1A_RESET_DISABLE},
    {nsw::mmtp::REG_FIBER_BC_OFFSET,          FiberBCOffset()},
    {nsw::mmtp::REG_INPUT_PHASE,              GlobalInputPhase()},
    {nsw::mmtp::REG_INPUT_PHASEOFFSET,        GlobalInputOffset()},
    {nsw::mmtp::REG_INPUT_PHASEL1OFFSET,      m_config.get<std::uint32_t>("gbtL1ddPhaseOffset", 0)},
    {nsw::mmtp::REG_SELFTRIGGER_DELAY,        SelfTriggerDelay()},
    {nsw::mmtp::REG_VMM_MASK_HOT_THRESH,      VmmMaskHotThresh()},
    {nsw::mmtp::REG_VMM_MASK_HOT_THRESH_HYST, VmmMaskHotThreshHyst()},
    {nsw::mmtp::REG_VMM_MASK_DRAIN_PERIOD,    VmmMaskDrainPeriod()},
    {nsw::mmtp::REG_GLO_SYNC_IDLE_STATE,      gloSyncIdleState()},
    {nsw::mmtp::REG_GLO_SYNC_BCID_OFFSET,     gloSyncBcidOffset()},
    {nsw::mmtp::REG_LAT_TX_IDLE_STATE,        1},
    // FIXME remove this once Nathan put it a pulse
    {nsw::mmtp::REG_LAT_TX_IDLE_STATE,        0},
    {nsw::mmtp::REG_LAT_TX_BCID_OFFSET,       m_config.get<std::uint32_t>("latTxBcidOffset", 100)},
    {nsw::mmtp::REG_FIBER_REMAP_SEL,          fiberRemapSel()},
  };

  const auto skippedReg = SkipRegisters();
  for (const auto& [addr, value]: list_of_messages) {
    if (skippedReg.contains(addr)) {
      continue;
    }
    try {
      writeRegister(addr, value);
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to write reg 0x{:08x}: {}", addr, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }

  if (doAlignArtGbtx) {
    alignArtGbtx();
  }
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::MMTP::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  const auto skippedReg = SkipRegisters();
  for (const auto& reg: nsw::mmtp::REGS) {
    if (skippedReg.contains(reg)) {
      continue;
    }
    try {
      result.emplace(reg, readRegister(reg));
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to read reg 0x{:08x}: {}", reg, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }
  return result;
}

void nsw::hw::MMTP::writeRegister(const std::uint32_t regAddress,
                                  const std::uint32_t value) const
{
  nsw::hw::SCAX::writeRegister(getConnection(), m_busAddress, regAddress, value);
}

std::uint32_t nsw::hw::MMTP::readRegister(const std::uint32_t regAddress) const
{
  return nsw::hw::SCAX::readRegister(getConnection(), m_busAddress, regAddress);
}

void nsw::hw::MMTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                             const std::uint32_t value) const
{
  ERS_LOG(fmt::format("{}: writing to {:#04x} with {:#10x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), m_busAddress, regAddress, value);
}

void nsw::hw::MMTP::toggleIdleStateHigh() const
{
  writeRegister(nsw::mmtp::REG_GLO_SYNC_IDLE_STATE, 1);
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 1);
  // FIXME remove this once Nathan put it a pulse
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 0);
}

std::set<std::uint8_t> nsw::hw::MMTP::SkipRegisters() const
{
  const auto key = "SkipRegisters";
  if (m_config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(m_config, key);
}

std::set<std::uint8_t> nsw::hw::MMTP::SkipFibers() const
{
  const auto key = "SkipFibers";
  if (m_config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(m_config, key);
}

std::vector<std::uint32_t> nsw::hw::MMTP::readAlignment(const size_t n_reads) const
{
  auto aligned = std::vector<std::uint32_t>(nsw::mmtp::NUM_FIBERS);
  for (std::size_t read = 0; read < n_reads; read++) {
    const auto word = readRegister(nsw::mmtp::REG_FIBER_ALIGNMENT);
    for (std::size_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
      const bool align = ((word >> fiber) & 1);
      if (align) {
        aligned.at(fiber)++;
      }
    }
  }
  return aligned;
}

void nsw::hw::MMTP::setHorxEnvMonAddr(const bool tx, const std::uint8_t microPod, const bool temp, const bool loss, const std::uint8_t fiber) const {
  int val = (microPod - 1 + (tx ? 0 : 3)) << 4;
  if (temp) {
  } else if (loss) {
    val += 1;
  } else {
    val += nsw::mmtp::NUM_FIBERS_PER_MICROPOD + 1 - fiber;
  }
  // not allowed to add this because it's a const function. Will need update in Monitoring Helper
  // m_config.put("HorxEnvMonAddr", val);
  writeRegister(nsw::mmtp::REG_HORX_ENV_MON_ADDR, val);
}

void nsw::hw::MMTP::alignArtGbtx() const
{
  // if no ARTs want to be aligned: exit
  const auto skipFibs = SkipFibers();
  if (skipFibs.size() == nsw::mmtp::NUM_FIBERS) {
    return;
  }

  // check ART/MMTP communication
  ERS_INFO(fmt::format("Checking ART communication for {}", m_name));
  std::size_t n_resets = 0;
  while (true) {

    // admit defeat
    if (n_resets > nsw::mmtp::FIBER_ALIGN_ATTEMPTS) {
      ers::warning(MMTPFiberAlignIssue(ERS_HERE, fmt::format("Failed to stabilize input to ADDC {}. Skipping.", m_name)));
      break;
    }

    // allow communication to settle after previous reset
    nsw::snooze(nsw::mmtp::FIBER_ALIGN_SLEEP);

    // read MMTP alignment register
    const auto aligned = readAlignment(nsw::mmtp::FIBER_ALIGN_N_READS);

    // announce
    for (std::uint32_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
      if (skipFibs.contains(fiber)) {
        continue;
      }
      const auto align = aligned.at(fiber);
      const auto msg = fmt::format("Fiber {}: {} aligned out of {}",
                                   fiber, align, nsw::mmtp::FIBER_ALIGN_N_READS);
      if (align < nsw::mmtp::FIBER_ALIGN_N_READS) {
        ERS_INFO(msg);
      } else {
        ERS_LOG(msg);
      }
    }

    // build the reset
    // if any fiber of a quad has any misalignments,
    // reset that QPLL
    std::uint32_t reset = 0;
    for (std::uint32_t qpll = 0; qpll < nsw::mmtp::NUM_QPLL; qpll++) {
      for (std::uint32_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
        if (skipFibs.contains(fiber)) {
          continue;
        }
        if (fiber / nsw::mmtp::NUM_FIBERS_PER_QPLL != qpll) {
          continue;
        }
        if (aligned.at(fiber) < nsw::mmtp::FIBER_ALIGN_N_READS) {
          reset += (1 << qpll);
          break;
        }
      }
    }
    ERS_INFO(fmt::format("{} reset word = {}", m_name, reset));

    // the moment of truth
    if (reset == 0) {
      ERS_INFO(fmt::format("alignArtGbtxMmtp success for {}!", m_name));
      break;
    }

    // or, set/unset the reset
    writeRegister(nsw::mmtp::REG_FIBER_QPLL_RESET, reset);
    writeRegister(nsw::mmtp::REG_FIBER_QPLL_RESET, nsw::mmtp::FIBER_QPLL_RESET_DISABLE);

    // and take note
    n_resets++;
  }
}
