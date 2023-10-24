#include "NSWConfiguration/hw/MMTP.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::MMTP::MMTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_config(config),
  m_name(fmt::format("{}/{}", getOpcServerIp(), getScaAddress())),
  m_skippedReg(nsw::hw::SCAX::SkipRegisters(m_config))
{
  for (const auto& [ name, bus, addr, def, rw ] : nsw::mmtp::REGS) {
    if(std::find(nsw::mmtp::registersToRemember.begin(), 
          nsw::mmtp::registersToRemember.end(), name) != nsw::mmtp::registersToRemember.end()) {
      m_registersToRemember.emplace(name, std::make_pair(bus, addr));
    }
    
    if(m_skippedReg.contains(addr)) continue;
    if(std::find(nsw::mmtp::registersToRead.begin(), 
          nsw::mmtp::registersToRead.end(), name) != nsw::mmtp::registersToRead.end()) {
      m_registersToRead.emplace_back(name, bus, addr);
    }
    if(std::find(nsw::mmtp::registersToWrite.begin(), 
          nsw::mmtp::registersToWrite.end(), name) != nsw::mmtp::registersToWrite.end()) {
      m_registersToWrite.emplace_back(name, bus, addr, def);
    }
  }
}

void nsw::hw::MMTP::writeConfiguration(bool doAlignArtGbtx) const
{
  for (const auto& [name, bus, addr, defVal]: m_registersToWrite) {
    try {
      writeRegister(addr, m_config.get(name, std::uint32_t{defVal}), bus);
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to write reg 0x{:08x}: {}", addr, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }
  toggleIdleStateHigh();
  writeRegister("REG_L1A_CONTROL", nsw::mmtp::L1A_RESET_ENABLE);
  writeRegister("REG_L1A_CONTROL", nsw::mmtp::L1A_RESET_DISABLE);
  if (doAlignArtGbtx) {
    alignArtGbtx();
  }
}

std::map<std::uint32_t, std::pair<std::string, std::uint32_t>> nsw::hw::MMTP::readConfiguration() const
{
  std::map<std::uint32_t, std::pair<std::string, std::uint32_t>> result;
  for (const auto& [ name, bus, addr ] : m_registersToRead) {
    // matching register number
    try {
      result.emplace(addr, std::make_pair(name, readRegister(addr, bus)));
    } catch (const std::exception& ex) {
      const auto msg = fmt::format("Failed to read reg {} at bus {} reg 0x{:08x}: {}", name, bus, addr, ex.what());
      ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
      throw;
    }
  }
  return result;
}

void nsw::hw::MMTP::writeRegister(const std::uint32_t regAddress,
                                  const std::uint32_t value, 
                                  const std::uint8_t bus) const
{
  nsw::hw::SCAX::writeRegister(getConnection(), 
      getBusAddress(bus), 
      regAddress, 
      value);
}

void nsw::hw::MMTP::writeRegister(const std::string regName, 
                                  const std::uint32_t value) const
{
  try {
    const auto & val = m_registersToRemember.at(regName);
    writeRegister(val.second, value, val.first);
  } catch (const std::exception& ex) {
    const auto msg = fmt::format("Failed to write reg by Name {}: {}", regName, ex.what());
    ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
    throw;
  }
}


std::uint32_t nsw::hw::MMTP::readRegister(const std::uint32_t regAddress, std::uint8_t bus) const
{
  return nsw::hw::SCAX::readRegister(getConnection(),
      getBusAddress(bus),
      regAddress
      );
}

std::uint32_t nsw::hw::MMTP::readRegister(const std::string regName) const
{
  try {
    const auto & val = m_registersToRemember.at(regName);
    return readRegister(val.second, val.first);
  } catch (const std::exception& ex) {
    const auto msg = fmt::format("Failed to read reg by Name {}: {}", regName, ex.what());
    ers::error(nsw::MMTPReadWriteIssue(ERS_HERE, msg));
    throw;
  }
}

void nsw::hw::MMTP::writeAndReadbackRegister(const std::uint32_t regAddress,
                                             const std::uint32_t value,
                                             std::uint8_t bus) const
{
  ERS_LOG(fmt::format("{}: writing to {:#04x} with {:#10x}", m_name, regAddress, value));
  nsw::hw::SCAX::writeAndReadbackRegister(getConnection(), 
      getBusAddress(bus), 
      regAddress, 
      value);
}

void nsw::hw::MMTP::toggleIdleStateHigh() const
{
  writeRegister(nsw::mmtp::REG_GLO_SYNC_IDLE_STATE, 1);
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 1);
  // FIXME remove this once Nathan put it a pulse
  writeRegister(nsw::mmtp::REG_LAT_TX_IDLE_STATE, 0);
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
    const auto word = readRegister("FIBER_ALIGNMENT");
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
