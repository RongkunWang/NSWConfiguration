#include "NSWConfiguration/hw/VMM.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/SCAInterface.h"

nsw::hw::VMM::VMM(const FEBConfig& config, const std::size_t numVmm) :
  m_config(config.getVmms().at(numVmm)),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_rocAnalogName(config.getRocAnalog().getName())
{}

void nsw::hw::VMM::writeConfiguration() const
{
  // Set Vmm Configuration Enable
  constexpr uint8_t VMM_ACC_DISABLE = 0xff;
  constexpr uint8_t VMM_ACC_ENABLE = 0x00;
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);

  // Set Vmm Acquisition Disable
  const auto scaRocVmmReadoutAddress =
    fmt::format("{}.{}.reg122vmmEnaInv", m_scaAddress, m_rocAnalogName);
  nsw::hw::SCA::sendI2c(opcConnection, scaRocVmmReadoutAddress, {VMM_ACC_DISABLE});

  const auto data = m_config.getByteVector();

  ERS_LOG("Sending configuration to " << m_scaAddress << ".spi." << m_config.getName());

  nsw::hw::SCA::sendSpiRaw(opcConnection,
                           fmt::format("{}.spi.{}", m_scaAddress, m_config.getName()),
                           data.data(),
                           data.size());

  ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(m_config.getBitString()));

  // Set Vmm Acquisition Enable
  nsw::hw::SCA::sendI2c(opcConnection, scaRocVmmReadoutAddress, {VMM_ACC_ENABLE});
}

std::map<std::uint8_t, std::vector<std::uint8_t>> nsw::hw::VMM::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}
