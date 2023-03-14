#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/SCAX.h"
#include <fmt/core.h>

void nsw::hw::SCAX::writeRegister(const OpcClientPtr opcConnection,
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

std::uint32_t nsw::hw::SCAX::readRegister(const OpcClientPtr opcConnection,
                                          const std::string& node,
                                          const std::uint32_t regAddress,
                                          const std::uint32_t mask)
{
  const auto addr = nsw::intToByteVector(regAddress, nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  const auto data = nsw::hw::SCA::readI2cAtAddress(opcConnection,
                                                   node,
                                                   addr.data(),
                                                   addr.size(),
                                                   nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(data, nsw::scax::SCAX_LITTLE_ENDIAN) & mask;
}

void nsw::hw::SCAX::writeAndReadbackRegister(const OpcClientPtr opcConnection,
                                             const std::string& node,
                                             const std::uint32_t regAddress,
                                             const std::uint32_t value,
                                             const std::uint32_t mask)
{
  writeRegister(opcConnection, node, regAddress, value);
  const auto val = readRegister(opcConnection, node, regAddress, mask);
  if (val != value) {
    const auto msg = fmt::format("Mismatch: wrote {}, readback {}", value, val);
    nsw::SCAXReadbackMismatch issue(ERS_HERE, msg.c_str());
    ers::error(issue);
  }
}




void nsw::hw::SCAX::writeRegister(const OpcClientPtr opcConnection,
                                  const std::string& node,
                                  const std::uint32_t value) {
  const auto data = nsw::intToByteVector(value,      nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  std::vector<uint8_t> payload;
  payload.reserve(data.size());
  payload.insert(std::end(payload), std::begin(data), std::end(data));
  opcConnection->writeI2cRaw(node, payload.data(), payload.size());
}

std::uint32_t nsw::hw::SCAX::readRegister(const OpcClientPtr opcConnection,
                                          const std::string& node)
{
  const auto data = opcConnection->readI2c(node, nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(data, nsw::scax::SCAX_LITTLE_ENDIAN);
}

void nsw::hw::SCAX::writeAndReadbackRegister(const OpcClientPtr opcConnection,
                                             const std::string& node,
                                             const std::uint32_t value)
{
  writeRegister(opcConnection, node, {value});
  const auto val = readRegister(opcConnection, node);
  if (val != value) {
    const auto msg = fmt::format("Mismatch: wrote {}, readback {}", value, val);
    nsw::SCAXReadbackMismatch issue(ERS_HERE, msg.c_str());
    ers::error(issue);
  }
}

std::set<std::uint8_t> nsw::hw::SCAX::SkipRegisters(const boost::property_tree::ptree& config)
{
  const auto key = "SkipRegisters";
  if (config.count(key) == 0) {
    return std::set<std::uint8_t>();
  }
  return nsw::getSetFromPtree<std::uint8_t>(config, key);
}

std::set<std::string> nsw::hw::SCAX::SkipRegistersStr(const boost::property_tree::ptree& config)
{
  const auto key = "SkipRegisters";
  if (config.count(key) == 0) {
    return std::set<std::string>();
  }
  return nsw::getSetFromPtree<std::string>(config, key);
}
