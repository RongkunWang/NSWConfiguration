#include "NSWConfiguration/hw/TPCarrier.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <stdexcept>

nsw::hw::TPCarrier::TPCarrier(OpcManager& manager, const TPCarrierConfig& config) :
  m_opcManager{manager},
  m_config(config),
  m_opcserverIp(config.getOpcServerIp()),
  m_scaAddress(config.getAddress())
{}

void nsw::hw::TPCarrier::writeConfiguration() const
{
  const auto& regs = nsw::carrier::REGS;
  writeRegister(regs.at("RJOUT_SEL"), m_config.RJOutSel());
}

std::map<std::uint32_t, std::uint32_t> nsw::hw::TPCarrier::readConfiguration() const
{
  std::map<std::uint32_t, std::uint32_t> result;
  for (const auto& [name, reg]: nsw::carrier::REGS) {
    result.emplace(reg, readRegister(reg));
  }
  return result;
}

void nsw::hw::TPCarrier::writeRegister(const std::uint32_t regAddress,
                                       const std::uint32_t value) const
{
  ERS_LOG("Writing address, message =  " << static_cast<uint32_t>(regAddress) << ", " << value << " to "
          << m_opcserverIp << "/" << m_opcserverIp);
  const auto addr = nsw::intToByteVector(regAddress, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  const auto data = nsw::intToByteVector(value,      nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);

  std::vector<uint8_t> payload;
  payload.reserve(addr.size() + data.size());
  payload.insert(std::end(payload), std::begin(addr), std::end(addr));
  payload.insert(std::end(payload), std::begin(data), std::end(data));

  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);
  nsw::hw::SCA::sendI2cRaw(opcConnection, m_scaAddress, payload.data(), payload.size());
}

std::uint32_t nsw::hw::TPCarrier::readRegister(const std::uint32_t regAddress) const
{
  const auto addr = nsw::intToByteVector(regAddress, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);

  const auto& opcConnection = m_opcManager.get().getConnection(m_opcserverIp, m_scaAddress);
  const auto data = nsw::hw::SCA::readI2cAtAddress(opcConnection, m_scaAddress,
                                                   addr.data(), addr.size(), nsw::NUM_BYTES_IN_WORD32);

  return nsw::byteVectorToWord32(data, nsw::scax::SCAX_LITTLE_ENDIAN);
}
