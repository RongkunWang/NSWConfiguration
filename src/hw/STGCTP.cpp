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
  writeRegister(nsw::stgctp::REG_SECTOR,         getSector());

  for (const auto & [reg, defVal] : nsw::stgctp::registersToWrite) {
    if(reg.find("_WO_") != std::string::npos) {
      if (defVal.type() == typeid(unsigned int)) {
        writeRegister(reg, m_config.get(reg, std::any_cast<unsigned int>(defVal)));
      } else if (defVal.type() == typeid(bool)) {
        writeRegister(reg, m_config.get(reg, std::any_cast<bool>(defVal)));
      }
    } else {
      if (defVal.type() == typeid(unsigned int)) {
        writeAndReadbackRegister(reg, m_config.get(reg, std::any_cast<unsigned int>(defVal)));
      } else if (defVal.type() == typeid(bool)) {
        writeAndReadbackRegister(reg, m_config.get(reg, std::any_cast<bool>(defVal)));
      }
    }
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
  ERS_LOG(fmt::format("About to write register {} = {}", regAddress, value));
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip writing to {}", m_name, regAddress));
    return;
  }
  nsw::hw::SCAX::writeRegister(getConnection(),
      fmt::format("{}.{}", getScaAddress(), regAddress),
      value);
}

std::uint32_t nsw::hw::STGCTP::readRegister(const std::string regAddress) const
{
  ERS_LOG(fmt::format("About to read register {}", regAddress));
  if (m_skippedReg.contains(regAddress)) {
    ERS_LOG(fmt::format("{}: skip reading {}, return dummy value {:#x}", m_name, regAddress, nsw::DEADBEEF));
    return nsw::DEADBEEF;
  }
  return nsw::hw::SCAX::readRegister(getConnection(),
      fmt::format("{}.{}", getScaAddress(), regAddress));
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
      fmt::format("{}.{}", getScaAddress(), regAddress),
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
