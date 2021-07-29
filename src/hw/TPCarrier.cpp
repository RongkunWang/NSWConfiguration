#include "NSWConfiguration/TPCarrierInterface.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/SCAInterface.h"
#include "NSWConfiguration/Utility.h"

nsw::hw::TPCarrier::TPCarrier(const TPCarrierConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{}

void nsw::hw::TPCarrier::writeConfiguration()
{
  writeRegister(nsw::carrier::REG_RJOUT_SEL, m_config.RJOutSel());
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::TPCarrier::readConfiguration()
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::TPCarrier::writeRegister(const std::uint8_t registerId, const std::uint32_t value)
{
  const auto& opcconnection = OpcManager::getConnection(m_opcserverIp);
  const auto data =
    nsw::intToByteVector(value, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  const auto addr =
    nsw::intToByteVector(registerId, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  std::vector<std::uint8_t> payload(addr);
  payload.insert(payload.end(), data.begin(), data.end());

  ERS_DEBUG(3,
            "... writing to TPCarrier: address, message =  " << static_cast<int>(registerId) << ", "
                                                             << static_cast<int>(value));
  DeviceInterface::SCA::sendI2cRaw(opcconnection, m_scaAddress, payload.data(), payload.size());
}

std::vector<std::uint8_t> nsw::hw::TPCarrier::readRegister(
  [[maybe_unused]] const std::uint8_t registerId)
{
  throw std::logic_error("Not implemented");
}
