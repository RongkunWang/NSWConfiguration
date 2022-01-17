#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/SCAX.h"
#include <fmt/core.h>

void nsw::hw::SCAX::writeRegister(const OpcClientPtr& opcConnection,
                                  const std::string& node,
                                  const std::uint32_t regAddress,
                                  const std::uint32_t value)
{
  const auto addr = nsw::intToByteVector(regAddress, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  const auto data = nsw::intToByteVector(value,      nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  std::vector<uint8_t> payload;
  payload.reserve(addr.size() + data.size());
  payload.insert(std::end(payload), std::begin(addr), std::end(addr));
  payload.insert(std::end(payload), std::begin(data), std::end(data));
  nsw::hw::SCA::sendI2cRaw(opcConnection,
                           node,
                           payload.data(),
                           payload.size());
}

std::uint32_t nsw::hw::SCAX::readRegister(const OpcClientPtr& opcConnection,
                                          const std::string& node,
                                          const std::uint32_t regAddress)
{
  const auto addr = nsw::intToByteVector(regAddress, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  const auto data = nsw::hw::SCA::readI2cAtAddress(opcConnection,
                                                   node,
                                                   addr.data(),
                                                   addr.size(),
                                                   nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(data, nsw::scax::SCAX_LITTLE_ENDIAN);
}

void nsw::hw::SCAX::writeAndReadbackRegister(const OpcClientPtr& opcConnection,
                                             const std::string& node,
                                             const std::uint32_t regAddress,
                                             const std::uint32_t value)
{
  writeRegister(opcConnection, node, regAddress, value);
  const auto val = readRegister(opcConnection, node, regAddress);
  if (val != value) {
    const auto msg = fmt::format("Mismatch: wrote {}, readback {}", value, val);
    nsw::SCAXReadbackMismatch issue(ERS_HERE, msg.c_str());
    ers::error(issue);
  }
}
