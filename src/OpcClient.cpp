#include <memory>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <utility>
#include <fstream>
#include <bitset>

#include <fmt/core.h>

#include <ers/ers.h>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/Constants.h"

// Generated (UaoClientForOpcUaSca/include) files
#include "SpiSlave.h"
#include "I2cSlave.h"
#include "DigitalIO.h"
#include "AnalogInput.h"
#include "SCA.h"
#include "IoBatch.h"
#include "XilinxFpga.h"

nsw::OpcClient::OpcClient(const std::string& server_ip_port): m_server_ipport(server_ip_port) {
    // TODO(cyildiz): Does this need to be moved to a higher level?
    // Can we have multiple init() in the same application?
    // Do we need this at all? It's an empty function
    // UaPlatformLayer::init();

    std::string opc_connection = "opc.tcp://" + server_ip_port;

    m_session = std::make_unique<UaClientSdk::UaSession>();
    m_sessionConnectInfo.internalServiceCallTimeout = OPC_SERVICE_TIMEOUT;

    // TODO(cyildiz): Handle connection exceptions
    UaStatus status = m_session->connect(
            opc_connection.c_str(),
            m_sessionConnectInfo,
            m_security,
            nullptr);

    if (status.isBad()) {  // Can't establish initial connection with Opc Server
        nsw::OpcConnectionIssue issue(ERS_HERE, m_server_ipport, status.toString().toUtf8());
        ers::error(issue);
        throw issue;
    }
}

nsw::OpcClient::~OpcClient() {
  ServiceSettings sessset = ServiceSettings();
  m_session->disconnect(sessset, OpcUa_True);
}

void nsw::OpcClient::writeSpiSlave(const std::string& node, const std::vector<uint8_t>& cdata) const {
    auto data = cdata.data();  // get pointer to array
    writeSpiSlaveRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeSpiSlaveRaw(const std::string& node, const uint8_t* data, size_t number_of_bytes) const {
    UaoClientForOpcUaSca::SpiSlave ss(m_session.get(), UaNodeId(node.c_str(), 2));

    UaByteString bs;
    // UaByteString::setByteString does not modify its buffer argument.
    bs.setByteString(number_of_bytes, const_cast<uint8_t*>(data));
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << number_of_bytes
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    bool success{ false };
    size_t retry{ 0 };
    while (!success && retry < MAX_RETRY) {
        try {
            ss.writeSlave(bs);
            success = true;
        } catch (const std::exception& e) {
            ERS_LOG("writeSpiSlaveRaw " << retry << " failed. " << e.what()
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            retry++;
            sleep(1);
        }
    }
    if (!success) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, "writeSpiSlaveRaw failed");
        ers::warning(issue);
        throw issue;
    }
}


uint8_t nsw::OpcClient::readRocRaw(const std::string& node, unsigned int scl, unsigned int sda,
                                    uint8_t registerAddress, unsigned int i2cDelay) const {
    UaoClientForOpcUaSca::IoBatch ioBatch(m_session.get(), UaNodeId( node.c_str(), 2));

    ioBatch.addSetPins( { { scl, true }, { sda, true } } );
    ioBatch.addSetPinsDirections( { { scl, UaoClientForOpcUaSca::IoBatch::OUTPUT }, { sda, UaoClientForOpcUaSca::IoBatch::OUTPUT } }, GPIO_PIN_DELAY );

    ioBatch.addSetPins( { { scl, true }, { sda, true } }, i2cDelay );
    ioBatch.addSetPins( { { sda, false } }, i2cDelay );
    ioBatch.addSetPins( { { scl, false } }, i2cDelay );

    uint8_t byte = 0xF1;

    for (auto i = 0; i < ROC_REGISTER_SIZE; ++i) {

      if ( byte & 0x80 ) {
        ioBatch.addSetPins( { { sda, true } } );
      } else {
        ioBatch.addSetPins( { { sda, false } } );
      }

      byte <<= 1;

      ioBatch.addSetPins( { { scl, true } }, i2cDelay );
      ioBatch.addSetPins( { { scl, false } }, i2cDelay );
    }

    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::INPUT } } );
    ioBatch.addSetPins( { { sda, false } } );
    ioBatch.addSetPins( { { scl, true } }, i2cDelay );
    ioBatch.addGetPins();
    ioBatch.addSetPins( { { scl, false } }, i2cDelay );
    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::OUTPUT } } );

    for (auto i = 0; i < ROC_REGISTER_SIZE; ++i) {
      if ( registerAddress & 0x80 ) {
        ioBatch.addSetPins( { { sda, true } } );
      } else {
        ioBatch.addSetPins( { { sda, false } } );
      }

      registerAddress <<= 1;

      ioBatch.addSetPins( { { scl, true } }, i2cDelay );
      ioBatch.addSetPins( { { scl, false } }, i2cDelay );
    }

    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::INPUT } } );
    ioBatch.addSetPins( { { sda, false } } );
    ioBatch.addSetPins( { { scl, true } }, i2cDelay );
    ioBatch.addGetPins();
    ioBatch.addSetPins( { { scl, false } }, i2cDelay );
    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::OUTPUT } } );

    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::INPUT } } );

    for (auto i = 0; i < ROC_REGISTER_SIZE; ++i) {
        ioBatch.addSetPins( { { scl, true } }, i2cDelay );
        ioBatch.addGetPins();
        ioBatch.addSetPins( { { scl, false } }, i2cDelay );
    }

    ioBatch.addSetPins( { { sda, true } }, i2cDelay );

    ioBatch.addSetPinsDirections( { { sda, UaoClientForOpcUaSca::IoBatch::OUTPUT } } );
    ioBatch.addSetPins( { { scl, true } }, i2cDelay );
    ioBatch.addSetPins( { { scl, false } }, i2cDelay );

    ioBatch.addSetPins( { { sda, false } }, i2cDelay );
    ioBatch.addSetPins( { { scl, true } }, i2cDelay );
    ioBatch.addSetPins( { { sda, true } }, i2cDelay );

    const auto interestingPinSda = UaoClientForOpcUaSca::repliesToPinBits( ioBatch.dispatch(), sda );

    std::bitset<ROC_REGISTER_SIZE> registerValue;

    for ( auto i = 0; i < ROC_REGISTER_SIZE; ++i ) {
        registerValue[ROC_REGISTER_SIZE-1-i] = interestingPinSda[i+2];
    }

    return static_cast<uint8_t>(registerValue.to_ulong());
}


std::vector<uint8_t> nsw::OpcClient::readSpiSlave(const std::string& node, size_t number_of_chunks) const {
    UaoClientForOpcUaSca::SpiSlave ss(m_session.get(), UaNodeId(node.c_str(), 2));

    try {
        UaByteString bsread;
        ss.readSlave(number_of_chunks, bsread);
        std::vector<uint8_t> result;
        auto array = bsread.data();
        auto length = bsread.length();
        ERS_DEBUG(4, "node: " << node << ", read bytes: " << length);
        result.assign(array, array + length);
        return result;
    } catch (const std::exception& e) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, e.what());
        ers::warning(issue);
        throw issue;
    }
}

void nsw::OpcClient::writeI2c(const std::string& node, const std::vector<uint8_t>& cdata) const {
    auto data = cdata.data();  // get pointer to array
    writeI2cRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeI2cRaw(const std::string& node, const uint8_t* data, size_t number_of_bytes)  const {
    UaoClientForOpcUaSca::I2cSlave i2cnode(m_session.get(), UaNodeId(node.c_str(), 2));

    UaByteString bs;
    // UaByteString::setByteString does not modify its buffer argument.
    bs.setByteString(number_of_bytes, const_cast<uint8_t*>(data));
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << number_of_bytes
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    bool success{ false };
    size_t retry{ 0 };
    while (!success && retry < MAX_RETRY) {
        try {
            i2cnode.writeSlave(bs);
            success = true;
        } 
        catch (const std::exception& e) {
            ERS_LOG("writeI2cRaw " << retry << " failed. " << e.what() 
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            retry++;
            sleep(1);
        }
    }
    if (!success) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, "writeI2c failed");
        ers::warning(issue);
        throw issue;
    }
}

void nsw::OpcClient::writeGPIO(const std::string& node, bool data) const {
    UaoClientForOpcUaSca::DigitalIO gpio(m_session.get(), UaNodeId(node.c_str(), 2));
    ERS_DEBUG(4, "Node: " << node << ", Data: " << data);

    bool success{ false };
    size_t retry{ 0 };
    while (!success && retry < MAX_RETRY) {
        try {
            gpio.writeValue(data);
            success = true;
        } catch (const std::exception& e) {
            ERS_LOG("writeGPIO " << retry << " to " << node << " failed. " << e.what()
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            retry++;
            sleep(1);
        }
    }
    if (!success) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, "writeGPIO failed");
        ers::warning(issue);
        throw issue;
    }
}

bool nsw::OpcClient::readGPIO(const std::string& node) const {
    UaoClientForOpcUaSca::DigitalIO gpio(m_session.get(), UaNodeId(node.c_str(), 2));
    bool value = false;

    bool success{ false };
    size_t retry{ 0 };
    while (!success && retry < MAX_RETRY) {
        try {
            value = gpio.readValue();
            success = true;
        } catch (const std::exception& e) {
            ERS_LOG("readGPIO " << retry << " to " << node << " failed. " << e.what()
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            retry++;
            sleep(1);
        }
    }
    if (!success) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, "readGPIO failed");
        ers::warning(issue);
        throw issue;
    }
    return value;
}

std::vector<uint8_t> nsw::OpcClient::readI2c(const std::string& node, size_t number_of_bytes) const {
    UaoClientForOpcUaSca::I2cSlave i2cnode(m_session.get(), UaNodeId(node.c_str(), 2));

    std::vector<uint8_t> result;
    try {
        UaByteString output;
        i2cnode.readSlave(number_of_bytes, output);
        ERS_DEBUG(4, "node: " << node << ", bytes to read: " << number_of_bytes);
        // copy array contents in a vector
        result.assign(output.data(), output.data() + number_of_bytes);
    } catch (const std::exception& e) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, e.what());
        ers::warning(issue);
        throw issue;
    }

    return result;
}

float nsw::OpcClient::readAnalogInput(const std::string& node) const {
    UaoClientForOpcUaSca::AnalogInput ainode(m_session.get(), UaNodeId(node.c_str(), 2));
    return ainode.readValue();
}

std::vector<short unsigned int> nsw::OpcClient::readAnalogInputConsecutiveSamples(const std::string& node, size_t n_samples) const {
    UaoClientForOpcUaSca::AnalogInput ainode(m_session.get(), UaNodeId(node.c_str(), 2));

    std::vector<short unsigned int> values;

    bool success{ false };
    size_t retry{ 0 };
    while (!success && retry < MAX_RETRY) {
        try {
            ainode.getConsecutiveRawSamples(n_samples, values);
            success = true;
        }
        catch (const std::exception& e) {
            values.clear();
            ERS_LOG("readAnalogInputConsecutiveSamples " << retry << " failed. " << e.what() 
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            retry++;
            sleep(1);
        }
    }
    if (!success) {
        nsw::OpcReadWriteIssue issue(ERS_HERE, m_server_ipport, node, "readAnalogInputConsecutiveSamples failed");
        ers::warning(issue);
        throw issue;
    }
    return values;
}

int nsw::OpcClient::readScaID(const std::string& node) const {
    UaoClientForOpcUaSca::SCA scanode(m_session.get(), UaNodeId(node.c_str(), 2));
    return scanode.readId();
}

std::string nsw::OpcClient::readScaAddress(const std::string& node) const {
    UaoClientForOpcUaSca::SCA scanode(m_session.get(), UaNodeId(node.c_str(), 2));
    return scanode.readAddress().toUtf8();
}

bool nsw::OpcClient::readScaOnline(const std::string& node) const {
    UaoClientForOpcUaSca::SCA scanode(m_session.get(), UaNodeId(node.c_str(), 2));
    return scanode.readOnline();
}

void nsw::OpcClient::writeXilinxFpga(const std::string& node, const std::string& bitfile_path) const {
    UaoClientForOpcUaSca::XilinxFpga fpga(m_session.get(), UaNodeId(node.c_str(), 2));

    // Open file in binary mode and immediately go to end
    std::ifstream input(bitfile_path, std::ios::binary| std::ios::ate);

    if (input.is_open()) {
        auto size = input.tellg();  // Current position, which is end of file
        ERS_DEBUG(4, "File size in bytes: " << size);
        auto bytes = std::make_unique<char[]>(size);
        input.seekg(0, std::ios::beg);  // Go to beginning of file
        input.read(bytes.get(), size);  // Read the whole file into memory block
        input.close();

        // Convert to UaByteString
        UaByteString bs;
        // FIXME: unnecessary cast?
        bs.setByteString(size, reinterpret_cast<uint8_t*>(bytes.get()));
        ERS_DEBUG(4, "Node: " << node << ", Data size: " << size
                  << ", data[0]: " << static_cast<unsigned>(bytes.get()[0]));

        try {
            fpga.program(bs);
        } catch (const std::exception& e) {
            // TODO(cyildiz) handle exception properly
            nsw::OpcClientIssue issue(ERS_HERE, fmt::format("Can't program FPGA: {}", e.what()));
            ers::warning(issue);
            // throw issue;
        }
    } else {  // File doesn't exist?
      // TODO(cyildiz) handle exception properly
      nsw::OpcClientIssue issue(ERS_HERE, fmt::format("Can't open bitfile: {}", bitfile_path));
      ers::warning(issue);
      // throw issue;
    }
}


// TODO(cyildiz): Set a parameter: number_of_retries, so each action is tried multiple times
