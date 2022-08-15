#include "NSWConfiguration/hw/MMTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::MMTP::MMTP(OpcManager& manager, const TPConfig& config):
  m_opcManager{manager},
  m_config(config.getConfig()),
  m_opcserverIp(m_config.get<std::string>("OpcServerIp")),
  m_scaAddress(m_config.get<std::string>("OpcNodeId")),
  m_busAddress(fmt::format("{}.I2C_0.bus0", m_scaAddress)),
  m_name(fmt::format("{}/{}", m_opcserverIp, m_scaAddress))
{
}

void nsw::hw::MMTP::writeConfiguration() const
{
  std::vector<std::pair<uint8_t, uint32_t> > list_of_messages = {
    {nsw::mmtp::REG_ADDC_EMU_DISABLE,         nsw::mmtp::ADDC_EMU_DISABLE},
    {nsw::mmtp::REG_L1A_LATENCY,              ARTWindowCenter()},
    {nsw::mmtp::REG_L1A_WIN_UPPER,            ARTWindowLeft()},
    {nsw::mmtp::REG_L1A_WIN_LOWER,            ARTWindowRight()},
    {nsw::mmtp::REG_L1A_CONTROL,              nsw::mmtp::L1A_RESET_ENABLE},
    {nsw::mmtp::REG_L1A_CONTROL,              nsw::mmtp::L1A_RESET_DISABLE},
    {nsw::mmtp::REG_FIBER_BC_OFFSET,          FiberBCOffset()},
    {nsw::mmtp::REG_INPUT_PHASE,              GlobalInputPhase()},
    {nsw::mmtp::REG_HORX_ENV_MON_ADDR,        HorxEnvMonAddr()},
    {nsw::mmtp::REG_INPUT_PHASEOFFSET,        GlobalInputOffset()},
    {nsw::mmtp::REG_SELFTRIGGER_DELAY,        SelfTriggerDelay()},
    {nsw::mmtp::REG_VMM_MASK_HOT_THRESH,      VmmMaskHotThresh()},
    {nsw::mmtp::REG_VMM_MASK_HOT_THRESH_HYST, VmmMaskHotThreshHyst()},
    {nsw::mmtp::REG_VMM_MASK_DRAIN_PERIOD,    VmmMaskDrainPeriod()},
    {nsw::mmtp::REG_GLO_SYNC_IDLE_STATE,      gloSyncIdleState()},
    {nsw::mmtp::REG_GLO_SYNC_BCID_OFFSET,     gloSyncBcidOffset()},
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

  alignArtGbtx();
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
      throw std::runtime_error(fmt::format("Failed to stabilize input to {}. Crashing", m_name));
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
