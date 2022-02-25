#include "NSWConfiguration/hw/SCAInterface.h"

#include "NSWConfiguration/Utility.h"

#include <fmt/core.h>

void nsw::hw::SCA::sendI2cRaw(const nsw::OpcClientPtr& opcConnection,
                              const std::string& node,
                              const std::uint8_t* data,
                              const size_t dataSize)
{
  opcConnection->writeI2cRaw(node, data, dataSize);
}

void nsw::hw::SCA::sendI2c(const nsw::OpcClientPtr& opcConnection,
                           const std::string& node,
                           const std::vector<std::uint8_t>& vdata)
{
  opcConnection->writeI2cRaw(node, vdata.data(), vdata.size());
}

void nsw::hw::SCA::sendGPIO(const nsw::OpcClientPtr& opcConnection,
                            const std::string& node,
                            const bool data)
{
  opcConnection->writeGPIO(node, data);
}

bool nsw::hw::SCA::readGPIO(const nsw::OpcClientPtr& opcConnection, const std::string& node)
{
  return opcConnection->readGPIO(node);
}

std::vector<std::uint8_t> nsw::hw::SCA::readI2c(const nsw::OpcClientPtr& opcConnection,
                                                const std::string& node,
                                                const size_t numberOfBytes)
{
  return opcConnection->readI2c(node, numberOfBytes);
}

std::vector<std::uint8_t> nsw::hw::SCA::readI2cAtAddress(const nsw::OpcClientPtr& opcConnection,
                                                         const std::string& node,
                                                         const std::uint8_t* address,
                                                         const size_t addressSize,
                                                         const size_t numberOfBytes)
{
  // Write only the address without data
  nsw::hw::SCA::sendI2cRaw(opcConnection, node, address, addressSize);

  // Read back data into the vector readdata
  std::vector<std::uint8_t> readdata = nsw::hw::SCA::readI2c(opcConnection, node, numberOfBytes);
  return readdata;
}

void nsw::hw::SCA::sendI2cAtAddress(const nsw::OpcClientPtr& opcConnection,
                                    const std::string& node,
                                    const std::vector<std::uint8_t>& address,
                                    std::vector<std::uint8_t> data)
{
  // Insert the address in the beginning of data vector
  for (const auto& addressByte : address) {
    data.insert(data.begin(), addressByte);
  }
  nsw::hw::SCA::sendI2cRaw(opcConnection, node, data.data(), data.size());
}

void nsw::hw::SCA::sendI2cMasterSingle(const nsw::OpcClientPtr& opcConnection,
                                       const std::string& topnode,
                                       const nsw::I2cMasterConfig& cfg,
                                       const std::string& regAddress)
{
  const auto addrBitstream = cfg.getBitstreamMap();
  const auto& bitstr = addrBitstream.at(regAddress);
  const auto data = nsw::stringToByteVector(bitstr);
  nsw::hw::SCA::sendI2cMasterSingle(
    opcConnection, fmt::format("{}.{}", topnode, cfg.getName()), data, regAddress);
}

void nsw::hw::SCA::sendI2cMasterSingle(const nsw::OpcClientPtr& opcConnection,
                                       const std::string& node,
                                       const std::vector<std::uint8_t>& data,
                                       const std::string& regAddress)
{
  ERS_LOG("Sending I2c configuration to " << node << "." << regAddress);
  const auto address = fmt::format("{}.{}", node, regAddress);  // Full I2C address
  for (const auto d : data) {
    ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
  }
  sendI2cRaw(opcConnection, address, data.data(), data.size());
}

void nsw::hw::SCA::sendI2cMasterConfig(const nsw::OpcClientPtr& opcConnection,
                                       const std::string& topnode,
                                       const nsw::I2cMasterConfig& cfg)
{
  ERS_LOG("Sending I2c configuration to " << topnode << "." << cfg.getName());
  const auto addrBitstream = cfg.getBitstreamMap();
  for (const auto& ab : addrBitstream) {
    const auto address =
      fmt::format("{}.{}.{}", topnode, cfg.getName(), ab.first);  // Full I2C address
    const auto bitstr = ab.second;
    const auto data = nsw::stringToByteVector(bitstr);
    for (const auto d : data) {
      ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
    }
    sendI2cRaw(opcConnection, address, data.data(), data.size());
  }
}

void nsw::hw::SCA::sendSpiRaw(const nsw::OpcClientPtr& opcConnection,
                              const std::string& node,
                              const std::uint8_t* data,
                              const size_t dataSize)
{
  opcConnection->writeSpiSlaveRaw(node, data, dataSize);
}

std::vector<std::uint8_t> nsw::hw::SCA::readSpi(const nsw::OpcClientPtr& opcConnection,
                                                const std::string& node,
                                                size_t dataSize)
{
  return opcConnection->readSpiSlave(node, dataSize);
}

void nsw::hw::SCA::sendSpi(const nsw::OpcClientPtr& opcConnection,
                           const std::string& node,
                           const std::vector<std::uint8_t>& vdata)
{
  opcConnection->writeSpiSlaveRaw(node, vdata.data(), vdata.size());
}

void nsw::hw::SCA::writeXilinxFpga(const OpcClientPtr& opcConnection,
                                   const std::string& node,
                                   const std::string& bitfile_path)
{
  opcConnection->writeXilinxFpga(node, bitfile_path);
}
