#include <memory>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <utility>

#include "ers/ers.h"

#include "NSWConfiguration/OpcClient.h"


nsw::OpcClient::OpcClient(std::string server_ip_port): m_server_ipport(server_ip_port) {
    // TODO(cyildiz): Does this need to be moved to a higher level?
    // Can we have multiple init() in the same application?
    // Do we need this at all? It's an empty function
    // UaPlatformLayer::init();

    std::string opc_connection = "opc.tcp://" + server_ip_port;

    m_session = std::make_unique<UaClientSdk::UaSession>();
    m_sessionConnectInfo.internalServiceCallTimeout = 60000;

    // TODO(cyildiz): Handle connection exceptions
    UaStatus status = m_session->connect(
            opc_connection.c_str(),
            m_sessionConnectInfo,
            m_security,
            new MyCallBack ());

    if (status.isBad()) {
        std::cout << "Bad status for : " << m_server_ipport
                  << " due to : " << status.toString().toUtf8() << std::endl;
        exit(0);
    }
}

nsw::OpcClient::~OpcClient() {
  ServiceSettings sessset = ServiceSettings();
  m_session->disconnect(sessset, OpcUa_True);
}

void nsw::OpcClient::writeSpiSlave(std::string node, std::vector<uint8_t> cdata) {
    auto data = cdata.data();  // get pointer to array
    writeSpiSlaveRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeSpiSlaveRaw(std::string node, uint8_t* data, size_t number_of_bytes) {
    UaoClientForOpcUaSca::SpiSlave ss(m_session.get(), UaNodeId(node.c_str(), 2));

    UaByteString bs;
    bs.setByteString(number_of_bytes, data);
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << number_of_bytes
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    try {
        ss.writeSlave(bs);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write SpiSlave: " <<  e.what() << std::endl;
    }
}


uint8_t nsw::OpcClient::readRocRaw(std::string node, unsigned int scl, unsigned int sda,
                                    uint8_t registerAddress, unsigned int i2cDelay) {

    UaoClientForOpcUaSca::IoBatch ioBatch(m_session.get(), UaNodeId( node.c_str(), 2));

    ioBatch.addSetPins( { { scl, true }, { sda, true } } );
    ioBatch.addSetPinsDirections( { { scl, UaoClientForOpcUaSca::IoBatch::OUTPUT }, { sda, UaoClientForOpcUaSca::IoBatch::OUTPUT } }, 10 );

    ioBatch.addSetPins( { { scl, true }, { sda, true } }, i2cDelay );
    ioBatch.addSetPins( { { sda, false } }, i2cDelay );
    ioBatch.addSetPins( { { scl, false } }, i2cDelay );

    uint8_t byte = 0xF1;

    for (auto i = 0; i < 8; ++i) {

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

    for (auto i = 0; i < 8; ++i) {
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

    for (auto i = 0; i < 8; ++i) {
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

    auto interestingPinSda = UaoClientForOpcUaSca::repliesToPinBits( ioBatch.dispatch(), sda );

    std::bitset<8> registerValue;

    for ( auto i = 0; i < 8; ++i ) {
        registerValue[7-i] = interestingPinSda[i+2];
    }

    return (uint8_t)(registerValue.to_ulong());
}


std::vector<uint8_t> nsw::OpcClient::readSpiSlave(std::string node, size_t number_of_chunks) {
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
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't read SpiSlave: " <<  e.what() << std::endl;
    }
}

void nsw::OpcClient::writeI2c(std::string node, std::vector<uint8_t> cdata) {
    auto data = cdata.data();  // get pointer to array
    writeI2cRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeI2cRaw(std::string node, uint8_t* data, size_t number_of_bytes) {
    UaoClientForOpcUaSca::I2cSlave i2cnode(m_session.get(), UaNodeId(node.c_str(), 2));

    UaByteString bs;
    bs.setByteString(number_of_bytes, data);
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << number_of_bytes
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    SUCCESS    = 0;
    THIS_RETRY = 0;
    while (!SUCCESS && THIS_RETRY < MAX_RETRY) {
        try {
            i2cnode.writeSlave(bs);
            SUCCESS = 1;
        } 
        catch (const std::exception& e) {
            ERS_LOG("writeI2cRaw " << THIS_RETRY << " failed. " << e.what() 
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            THIS_RETRY++;
            sleep(1);
        }
    }
}

void nsw::OpcClient::writeGPIO(std::string node, bool data) {
    UaoClientForOpcUaSca::DigitalIO gpio(m_session.get(), UaNodeId(node.c_str(), 2));
    ERS_DEBUG(4, "Node: " << node << ", Data: " << data);

    try {
        gpio.writeValue(data);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write GPIO: " <<  e.what() << std::endl;
    }
}

bool nsw::OpcClient::readGPIO(std::string node) {
    UaoClientForOpcUaSca::DigitalIO gpio(m_session.get(), UaNodeId(node.c_str(), 2));
    bool value = false;

    try {
        value = gpio.readValue();
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't read GPIO: " <<  e.what() << std::endl;
    }
    return value;
}

std::vector<uint8_t> nsw::OpcClient::readI2c(std::string node, size_t number_of_bytes) {
    UaoClientForOpcUaSca::I2cSlave i2cnode(m_session.get(), UaNodeId(node.c_str(), 2));

    std::vector<uint8_t> result;
    try {
        UaByteString output;
        i2cnode.readSlave(number_of_bytes, output);
        ERS_DEBUG(4, "node: " << node << ", bytes to read: " << number_of_bytes);
        // copy array contents in a vector
        result.assign(output.data(), output.data() + number_of_bytes);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't read I2c: " <<  e.what() << std::endl;
    }

    return result;
}

float nsw::OpcClient::readAnalogInput(std::string node) {
    UaoClientForOpcUaSca::AnalogInput ainode(m_session.get(), UaNodeId(node.c_str(), 2));
    return ainode.readValue();
}

std::vector<short unsigned int> nsw::OpcClient::readAnalogInputConsecutiveSamples(std::string node, size_t n_samples) {
    UaoClientForOpcUaSca::AnalogInput ainode(m_session.get(), UaNodeId(node.c_str(), 2));

    std::vector<short unsigned int> values;

    SUCCESS    = 0;
    THIS_RETRY = 0;
    while (!SUCCESS && THIS_RETRY < MAX_RETRY) {
        try {
            ainode.getConsecutiveRawSamples(n_samples, values);
            SUCCESS = 1;
        }
        catch (const std::exception& e) {
            values.clear();
            ERS_LOG("readAnalogInputConsecutiveSamples " << THIS_RETRY << " failed. " << e.what() 
                    << " Next attempt. Maximum " << MAX_RETRY << " attempts.");
            THIS_RETRY++;
            sleep(1);
        }
    }
    return values;
}

// TODO(cyildiz): Set a parameter: number_of_retries, so each action is tried multiple times

