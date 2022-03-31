#include "NSWConfiguration/hw/VMM.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/SCAInterface.h"

nsw::hw::VMM::VMM(OpcManager& manager, const FEBConfig& config, const std::size_t numVmm) :
  m_opcManager{manager},
  m_config(config.getVmms().at(numVmm)),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress()),
  m_rocAnalogName(config.getRocAnalog().getName()),
  m_vmmId{numVmm + config.getFirstVmmIndex()}
{}

void nsw::hw::VMM::writeConfiguration(const bool resetVmm) const
{
  writeConfiguration(m_config, resetVmm);
}

void nsw::hw::VMM::writeConfiguration(const VMMConfig& config, bool resetVmm) const
{
  // Set Vmm Configuration Enable
  constexpr std::uint8_t VMM_ACC_DISABLE = 0xff;
  constexpr std::uint8_t VMM_ACC_ENABLE = 0x00;
  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);

  // Set Vmm Acquisition Disable
  const auto scaRocVmmReadoutAddress =
    fmt::format("{}.{}.reg122vmmEnaInv", m_scaAddress, m_rocAnalogName);
  nsw::hw::SCA::sendI2c(opcConnection, scaRocVmmReadoutAddress, {VMM_ACC_DISABLE});

  const auto writeVmmConfig = [this, &opcConnection](const auto& conf) {
    const auto data = conf.getByteVector();

    setVmmConfigurationStatusInfoDcs(opcConnection, conf);
    ERS_DEBUG(4, "Sending configuration to " << m_scaAddress << ".spi." << conf.getName());

    nsw::hw::SCA::sendSpiRaw(opcConnection,
                             fmt::format("{}.spi.{}", m_scaAddress, conf.getName()),
                             data.data(),
                             data.size());
  };

  if (resetVmm) {
    auto configCopy{config};
    constexpr std::uint32_t RESET_VMM = 3;
    configCopy.setGlobalRegister("reset", RESET_VMM);
    writeVmmConfig(configCopy);
  }

  writeVmmConfig(config);

  ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(config.getBitString()));

  // Set Vmm Acquisition Enable
  nsw::hw::SCA::sendI2c(opcConnection, scaRocVmmReadoutAddress, {VMM_ACC_ENABLE});
}

std::map<std::uint8_t, std::vector<std::uint8_t>> nsw::hw::VMM::readConfiguration() const
{
  throw std::logic_error("Not implemented");
}

std::vector<std::uint16_t> nsw::hw::VMM::samplePdoMonitoringOutput(const std::size_t nSamples) const
{
  return samplePdoMonitoringOutput(m_config, nSamples);
}

std::vector<std::uint16_t> nsw::hw::VMM::samplePdoMonitoringOutput(VMMConfig config,
                                                                   const std::size_t nSamples) const
{
  config.setGlobalRegister("sbmx", 1);  // Route analog monitor to pdo output
  config.setGlobalRegister("sbfp", 1);  // Enable PDO output buffers (more stable reading)

  writeConfiguration(config);

  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);
  return nsw::hw::SCA::readAnalogInputConsecutiveSamples(
    opcConnection, fmt::format("{}.ai.vmmPdo{}", m_scaAddress, m_vmmId), nSamples);
}

void nsw::hw::VMM::setVmmConfigurationStatusInfoDcs(const OpcClientPtr& opcConnection,
                                                    const nsw::VMMConfig& config) const
{
  ERS_DEBUG(
    3,
    fmt::format("[{}, {}] Write VMMConfigurationStatusInfo FreeVariable parameter for DCS use.",
                m_scaAddress,
                config.getName()));

  // VMM registers for temperature monitoring
  const auto scmx = config.getGlobalRegister("scmx");
  const auto sbmx = config.getGlobalRegister("sbmx");
  const auto sbfp = config.getGlobalRegister("sbfp");
  const auto sm = config.getGlobalRegister("sm");
  const auto reset = config.getGlobalRegister("reset");

  // 1 : VMM temperature monitoring mode
  // 0 : Baseline mode (No VMM temperature output)
  const bool isVMMTemperatureModeEnabled =
    (scmx == 0 && sbmx == 1 && sbfp == 1 && sm == 4 && reset != 3);

  nsw::hw::SCA::writeFreeVariable(
    opcConnection,
    fmt::format("{}.spi.{}.configurationStatus", m_scaAddress, config.getName()),
    isVMMTemperatureModeEnabled);
}
