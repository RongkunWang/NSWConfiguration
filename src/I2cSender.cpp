#include "NSWConfiguration/I2cInterface.h"

#include "NSWConfiguration/Utility.h"

void nsw::DeviceInterface::I2c::sendI2cRaw(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const uint8_t*                         data,
  const size_t                           data_size) {
  opc_connection->writeI2cRaw(node, data, data_size);
}

void nsw::DeviceInterface::I2c::sendI2c(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const std::vector<uint8_t>&            vdata) {
  opc_connection->writeI2cRaw(node, vdata.data(), vdata.size());
}

void nsw::DeviceInterface::I2c::sendGPIO(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const bool                             data) {
  opc_connection->writeGPIO(node, data);
}

bool nsw::DeviceInterface::I2c::readGPIO(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node) {
  return opc_connection->readGPIO(node);
}

std::vector<uint8_t> nsw::DeviceInterface::I2c::readI2c(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const size_t                           number_of_bytes) {
  return opc_connection->readI2c(node, number_of_bytes);
}

std::vector<uint8_t> nsw::DeviceInterface::I2c::readI2cAtAddress(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const uint8_t*                         address,
  const size_t                           address_size,
  const size_t                           number_of_bytes) {
  // Write only the address without data
  nsw::DeviceInterface::I2c::sendI2cRaw(
    opc_connection, node, address, address_size);

  // Read back data into the vector readdata
  std::vector<uint8_t> readdata =
    nsw::DeviceInterface::I2c::readI2c(opc_connection, node, number_of_bytes);
  return readdata;
}

void nsw::DeviceInterface::I2c::sendI2cAtAddress(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const std::vector<uint8_t>&            address,
  std::vector<uint8_t>                   data) {
  // Insert the address in the beginning of data vector
  for (const auto& address_byte : address) {
    data.insert(data.begin(), address_byte);
  }
  nsw::DeviceInterface::I2c::sendI2cRaw(
    opc_connection, node, data.data(), data.size());
}

void nsw::DeviceInterface::I2c::sendI2cMasterSingle(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     topnode,
  const nsw::I2cMasterConfig&            cfg,
  const std::string&                     reg_address) {
  const auto  addr_bitstr = cfg.getBitstreamMap();
  const auto& bitstr      = addr_bitstr.at(reg_address);
  const auto  data        = nsw::stringToByteVector(bitstr);
  nsw::DeviceInterface::I2c::sendI2cMasterSingle(
    opc_connection, topnode + "." + cfg.getName(), data, reg_address);
}

void nsw::DeviceInterface::I2c::sendI2cMasterSingle(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     node,
  const std::vector<uint8_t>&            data,
  const std::string&                     reg_address) {
  ERS_LOG("Sending I2c configuration to " << node << "." << reg_address);
  const auto address = node + "." + reg_address;  // Full I2C address
  for (auto d : data) {
    ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
  }
  sendI2cRaw(opc_connection, address, data.data(), data.size());
}

void nsw::DeviceInterface::I2c::sendI2cMasterConfig(
  const std::unique_ptr<nsw::OpcClient>& opc_connection,
  const std::string&                     topnode,
  const nsw::I2cMasterConfig&            cfg) {
  ERS_LOG("Sending I2c configuration to " << topnode << "." << cfg.getName());
  const auto addr_bitstr = cfg.getBitstreamMap();
  for (const auto& ab : addr_bitstr) {
    const auto address =
      topnode + "." + cfg.getName() + "." + ab.first;  // Full I2C address
    const auto bitstr = ab.second;
    const auto data   = nsw::stringToByteVector(bitstr);
    for (const auto d : data) {
      ERS_DEBUG(5, "data: " << static_cast<unsigned>(d));
    }
    sendI2cRaw(opc_connection, address, data.data(), data.size());
  }
}