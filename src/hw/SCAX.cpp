#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/SCAX.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include <fmt/core.h>


void nsw::hw::SCAX::writeRegister(const OpcClientPtr opcConnection,
                                  std::string_view node,
                                  const std::uint32_t value) {
  const auto data = nsw::intToByteVector(value,      nsw::NUM_BYTES_IN_WORD32, nsw::scax::SCAX_LITTLE_ENDIAN);
  // std::vector<uint8_t> payload;
  // payload.reserve(data.size());
  // payload.insert(std::end(payload), std::begin(data), std::end(data));
  // opcConnection->writeI2cRaw(node, payload.data(), payload.size());
  nsw::hw::SCA::sendI2c(opcConnection, std::string(node), data);

}

std::uint32_t nsw::hw::SCAX::readRegister(const OpcClientPtr opcConnection,
                                          std::string_view node)
{
  // const auto data = opcConnection->readI2c(node, nsw::NUM_BYTES_IN_WORD32);
  const auto data = nsw::hw::SCA::readI2c(opcConnection, std::string(node), nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(data, nsw::scax::SCAX_LITTLE_ENDIAN);
}

void nsw::hw::SCAX::writeAndReadbackRegister(const OpcClientPtr opcConnection,
                                             std::string_view  node,
                                             const std::uint32_t value)
{
  writeRegister(opcConnection, node, value);
  const auto val = readRegister(opcConnection, node);
  if (val != value) {
    const auto msg = fmt::format("Mismatch: wrote {}, readback {}", value, val);
    nsw::SCAXReadbackMismatch issue(ERS_HERE, msg.c_str());
    ers::error(issue);
  }
}


std::set<std::string_view> nsw::hw::SCAX::SkipRegisters(const boost::property_tree::ptree& config)
{
  std::set<std::string_view> skippedRegSet;
  const auto key = "SkipRegisters";
  if (config.count(key) != 0) {
    auto v = nsw::getSetFromPtree<std::string>(config, key);
    std::copy(v.begin(), v.end(),
                      std::inserter(skippedRegSet, skippedRegSet.begin()));
  } 
  return skippedRegSet;
}
