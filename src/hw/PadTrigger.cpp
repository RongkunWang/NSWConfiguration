#include "NSWConfiguration/PadTriggerInterface.h"

#include <iterator>
#include <stdexcept>
#include <string>
#include <chrono>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/SCAInterface.h"
#include "NSWConfiguration/Utility.h"

using namespace std::chrono_literals;

nsw::hw::PadTrigger::PadTrigger(const PadTriggerSCAConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{}

void nsw::hw::PadTrigger::writeConfiguration()
{
  const auto& opcconnection = OpcManager::getConnection(m_opcserverIp);
  // Repeaters
  if (m_config.ConfigRepeaters()) {
    const std::vector<std::tuple<std::string, uint8_t, uint8_t>> repeater_sequence_of_commands = {
      // clang-format off
      // ENABLE SMBUS REGISTERS ON ALL REPEATER CHIPS
      {"1", 0x07, 0x01}, {"1", 0x07, 0x11}, {"1", 0x07, 0x21}, {"1", 0x07, 0x31},
      {"2", 0x07, 0x01}, {"2", 0x07, 0x11}, {"2", 0x07, 0x21}, {"2", 0x07, 0x31},
      {"3", 0x07, 0x01}, {"3", 0x07, 0x11}, {"3", 0x07, 0x21}, {"3", 0x07, 0x31},
      {"4", 0x07, 0x01}, {"4", 0x07, 0x11}, {"4", 0x07, 0x21}, {"4", 0x07, 0x31},
      {"5", 0x07, 0x01}, {"5", 0x07, 0x11}, {"5", 0x07, 0x21}, {"5", 0x07, 0x31},
      {"6", 0x07, 0x01}, {"6", 0x07, 0x11}, {"6", 0x07, 0x21}, {"6", 0x07, 0x31},
      // SET REPEATER CHIPS EQUALISER SETTINGS
      {"1", 0x14, 0x03}, {"1", 0x16, 0x03}, {"1", 0x18, 0x03}, {"1", 0x1A, 0x03},
      {"2", 0x14, 0x03}, {"2", 0x16, 0x03}, {"2", 0x18, 0x03}, {"2", 0x1A, 0x03},
      {"3", 0x14, 0x03}, {"3", 0x16, 0x03}, {"3", 0x18, 0x03}, {"3", 0x1A, 0x03},
      {"4", 0x14, 0x03}, {"4", 0x16, 0x03}, {"4", 0x18, 0x03}, {"4", 0x1A, 0x03},
      {"5", 0x14, 0x03}, {"5", 0x16, 0x03}, {"5", 0x18, 0x03}, {"5", 0x1A, 0x03},
      {"6", 0x14, 0x03}, {"6", 0x16, 0x03}, {"6", 0x18, 0x03}, {"6", 0x1A, 0x03},
      // DISABLE SMBUS REGISTERS ON ALL REPEATER CHIPS
      // (not really necessary but it’s a protection against occasional commands sent by mistake)
      {"1", 0x07, 0x00}, {"1", 0x07, 0x10}, {"1", 0x07, 0x20}, {"1", 0x07, 0x30},
      {"2", 0x07, 0x00}, {"2", 0x07, 0x10}, {"2", 0x07, 0x20}, {"2", 0x07, 0x30},
      {"3", 0x07, 0x00}, {"3", 0x07, 0x10}, {"3", 0x07, 0x20}, {"3", 0x07, 0x30},
      {"4", 0x07, 0x00}, {"4", 0x07, 0x10}, {"4", 0x07, 0x20}, {"4", 0x07, 0x30},
      {"5", 0x07, 0x00}, {"5", 0x07, 0x10}, {"5", 0x07, 0x20}, {"5", 0x07, 0x30},
      {"6", 0x07, 0x00}, {"6", 0x07, 0x10}, {"6", 0x07, 0x20}, {"6", 0x07, 0x30},
      // clang-format on
    };

    for (const auto& [rep, address, value] : repeater_sequence_of_commands) {
      // values
      const auto node = ".repeaterChip" + rep + ".repeaterChip" + rep;
      const std::vector<uint8_t> data = {value};

      // GPIO enable
      DeviceInterface::SCA::sendGPIO(
        opcconnection, m_scaAddress + ".gpio.gpio-repeaterChip" + rep, 1);

      // Repeater I2C: write
      ERS_INFO(m_opcserverIp << " " << m_scaAddress << ": Repeater I2C. "
                             << "Writing 0x" << std::hex << static_cast<unsigned>(value)
                             << " to address 0x" << static_cast<unsigned>(address) << std::dec);
      writeRegister(address, node, data);
      std::this_thread::sleep_for(10ms);

      // Repeater I2C: readback
      // FIXME TODO 1 is magic, number of bytes to read back from a Repeater address
      const auto val = readRegister(address, node, 1).at(0);
      ERS_INFO(m_opcserverIp << " " << m_scaAddress << ": Repeater I2C. "
                             << " Readback " << rep << ": 0x" << std::hex << val);
      std::this_thread::sleep_for(10ms);

      // GPIO disable
      DeviceInterface::SCA::sendGPIO(
        opcconnection, m_scaAddress + ".gpio.gpio-repeaterChip" + rep, 0);
    }
  } else {
    ERS_INFO("Skipping configuration of repeaters of " << m_opcserverIp << " " << m_scaAddress);
  }

  // VTTX
  if (m_config.ConfigVTTx()) {
    constexpr uint8_t address = 0x0;           // FIXME TODO address is magic
    const std::vector<uint8_t> data = {0xC7};  // FIXME TODO configuration data is magic
    const std::vector<std::string> vttxs = {"1", "2"};

    // 2.0 VTTX
    ERS_INFO("VTTx I2C: Writing " << std::hex << static_cast<std::uint32_t>(data.at(0))
                                  << " to address 0x" << static_cast<std::uint32_t>(address)
                                  << std::dec);
    for (const auto& vttx : vttxs) {
      const std::string node = ".vttx" + vttx + ".vttx" + vttx;
      writeRegister(address, node, data);
      std::this_thread::sleep_for(100ms);
    }

    // 2.1 Read them
    for (const auto& vttx : vttxs) {
      const std::string node = ".vttx" + vttx + ".vttx" + vttx;
      // FIXME 2 is a magic number, bytes to read from a VTTx config register
      const auto val =
        nsw::byteVectorToWord32(readRegister(address, node, 2), nsw::padtrigger::SCA_LITTLE_ENDIAN);
      ERS_INFO(m_opcserverIp << " " << m_scaAddress << " Readback VTTx" << vttx << ": " << std::hex
                             << val << std::dec);
      std::this_thread::sleep_for(100ms);
    }
  } else {
    ERS_INFO("Skipping configuration of VTTx of " << m_opcserverIp << " " << m_scaAddress);
  }

  // Control register
  if (m_config.ConfigControlRegister()) {
    constexpr std::uint8_t address = 0x0;
    const std::string node = ".fpga.fpga";
    const auto i2c_val_32 = static_cast<std::uint32_t>(m_config.ControlRegister());
    const auto data = nsw::intToByteVector(
      i2c_val_32, nsw::NUM_BYTES_IN_WORD32, nsw::padtrigger::SCA_LITTLE_ENDIAN);

    std::stringstream msg;
    msg << " Writing  " << node << " reg " << address << " val " << std::hex
        << m_config.ControlRegister() << " -> msg = ";
    for (const auto& val : data) {
      msg << std::hex << static_cast<unsigned>(val) << " " << std::dec;
    }

    writeRegister(address, node, data);

    // FIXME 4 magic number, size of control register
    const auto val =
      nsw::byteVectorToWord32(readRegister(address, node, 4), nsw::padtrigger::SCA_LITTLE_ENDIAN);
    ERS_INFO(m_opcserverIp << " " << m_scaAddress << " Readback ControlRegister: " << std::hex
                           << val << std::dec);
  } else {
    ERS_INFO("Skipping configuration of control register " << m_opcserverIp << " " << m_scaAddress);
  }
}

std::map<std::uint8_t, std::vector<std::uint32_t>> nsw::hw::PadTrigger::readConfiguration()
{
  throw std::logic_error("Not implemented");
}

void nsw::hw::PadTrigger::writeRegister(const std::uint8_t regAddress,
                                        const std::string& scaNode,
                                        const std::vector<std::uint8_t>& data)
{
  const auto& opcconnection = OpcManager::getConnection(m_opcserverIp);
  const auto addr = m_scaAddress + scaNode;

  // address and data
  const std::vector<std::uint8_t> address = {regAddress};

  std::vector<std::uint8_t> payload = address;
  payload.insert(payload.end(), data.begin(), data.end());

  std::stringstream msg;
  msg << " Writing  " << addr << " register " << std::hex << static_cast<std::size_t>(regAddress)
      << std::dec << " -> msg = ";

  for (const auto& val : payload) {
    msg << std::hex << static_cast<unsigned>(val) << " " << std::dec;
  }

  ERS_DEBUG(3, msg.str());

  DeviceInterface::SCA::sendI2cRaw(opcconnection, scaNode, payload.data(), payload.size());
}

std::vector<std::uint8_t> nsw::hw::PadTrigger::readRegister(const std::uint8_t regAddress,
                                                            const std::string& scaNode,
                                                            const std::size_t bytesToRead)
{
  const auto& opcconnection = OpcManager::getConnection(m_opcserverIp);
  const auto addr = m_scaAddress + scaNode;

  const std::vector<std::uint8_t> address = {regAddress};

  return DeviceInterface::SCA::readI2cAtAddress(
    opcconnection, m_scaAddress, address.data(), address.size(), bytesToRead);
}
