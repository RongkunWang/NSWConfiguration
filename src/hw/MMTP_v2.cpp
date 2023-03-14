#include "NSWConfiguration/hw/MMTP_v2.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::MMTP_v2::MMTP_v2(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress())),
  m_skippedReg(nsw::hw::SCAX::SkipRegisters(m_config))
{
}

void nsw::hw::MMTP_v2::writeConfiguration(bool doAlignArtGbtx) const
{
  for (const auto& [reg, defVal]: nsw::mmtp::registersToWrite) {
    if (m_skippedReg.contains(reg)) {
      ERS_LOG(fmt::format("{}: skip writing to {}", m_name, reg));
      continue;
    }
    try {
      writeRegister(reg, m_config.get(std::string(reg), std::uint32_t{defVal}));
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to write reg {}: {}", reg, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }
  toggleIdleStateHigh();
  writeRegister(nsw::mmtp::REG_L1A_CONTROL, nsw::mmtp::L1A_RESET_ENABLE);
  writeRegister(nsw::mmtp::REG_L1A_CONTROL, nsw::mmtp::L1A_RESET_DISABLE);
  if (doAlignArtGbtx) {
    alignArtGbtx();
  }
}

std::map<std::string, std::uint32_t> nsw::hw::MMTP_v2::readConfiguration() const
{
  std::map<std::string, std::uint32_t> result;
  for (const auto& reg : nsw::mmtp::registersToRead) {
    if (m_skippedReg.contains(reg)) {
      ERS_LOG(fmt::format("{}: skip reading from {}", m_name, reg));
      continue;
    }
    // matching register number
    try {
      result.emplace(reg, readRegister(reg));
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to read reg {}: {}",  reg, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }
  return result;
}

void nsw::hw::MMTP_v2::writeRegister(std::string_view regAddress,
                                  const std::uint32_t value) const
{
  ERS_LOG(fmt::format("About to write register {} = {}", regAddress, value));
  nsw::hw::SCAX::writeRegister(getConnection(),
      fmt::format("{}.{}", getScaAddress(), regAddress),
      value);
}

std::uint32_t nsw::hw::MMTP_v2::readRegister(std::string_view regAddress) const
{
  ERS_LOG(fmt::format("About to read register {}", regAddress));
  return nsw::hw::SCAX::readRegister(getConnection(),
      fmt::format("{}.{}", getScaAddress(), regAddress));
}

void nsw::hw::MMTP_v2::writeAndReadbackRegister(std::string_view regAddress,
                                             const std::uint32_t value) const
{
  ERS_LOG(fmt::format("About to write&read register {} = {}", regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(),
      fmt::format("{}.{}", getScaAddress(), regAddress),
      value);
}

void nsw::hw::MMTP_v2::toggleIdleStateHigh() const
{
  writeRegister(nsw::mmtp::REG_GLO_SYNC_IDLE_STATE, 1);
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 1);
  // FIXME remove this once Nathan put it a pulse
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 0);
}


std::set<std::uint8_t> nsw::hw::MMTP_v2::SkipFibers() const
{
  const auto key = "SkipFibers";
  if (m_config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(m_config, key);
}

std::vector<std::uint32_t> nsw::hw::MMTP_v2::readAlignment(const size_t n_reads) const
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

void nsw::hw::MMTP_v2::setHorxEnvMonAddr(const bool tx, const std::uint8_t microPod, const bool temp, const bool loss, const std::uint8_t fiber) const {
  int val = (microPod - 1 + (tx ? 0 : 3)) << 4;
  if (temp) {
  } else if (loss) {
    val += 1;
  } else {
    val += nsw::mmtp::NUM_FIBERS_PER_MICROPOD + 1 - fiber;
  }
  writeRegister(nsw::mmtp::REG_HORX_ENV_MON_ADDR, val);
}

void nsw::hw::MMTP_v2::alignArtGbtx() const
{
  // if no ARTs want to be aligned: exit
  const auto skipFibs = SkipFibers();
  if (skipFibs.size() == nsw::mmtp::NUM_FIBERS) {
    return;
  }

  // check ART/MMTP_v2 communication
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

    // read MMTP_v2 alignment register
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