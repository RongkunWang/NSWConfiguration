#include <memory>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

#include "ers/ers.h"

#include "NSWConfiguration/OpcClient.h"

nsw::OpcClient::OpcClient(std::string server_ip_port): m_server_ipport(server_ip_port) {
    // TODO(cyildiz): Does this need to be moved to a higher level?
    // Can we have multiple init() in the same application?
    // Do we need this at all?
    // UaPlatformLayer::init();

    std::string opc_connection = "opc.tcp://" + server_ip_port;

    // TODO(cyildiz): Handle connection exceptions
    m_session = ClientSessionFactory::connect(opc_connection.c_str());
    if (!m_session) {
        std::cout << "Connection error!" << std::endl;
        exit(0);
    }
}

void nsw::OpcClient::writeSpiSlave(std::string node, std::vector<uint8_t> cdata) {
    auto data = cdata.data();  // get pointer to array
    writeSpiSlaveRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeSpiSlaveRaw(std::string node, uint8_t* data, size_t data_size) {
    SpiSlave ss(m_session, UaNodeId(node.c_str(), 2));

    UaByteString bs;
    bs.setByteString(data_size, data);
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << data_size
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    try {
        ss.writeWrite(bs);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write SpiSlave: " <<  e.what() << std::endl;
    }
}

void nsw::OpcClient::writeI2c(std::string node, std::vector<uint8_t> cdata) {
    auto data = cdata.data();  // get pointer to array
    writeI2cRaw(node, data, cdata.size());
}

void nsw::OpcClient::writeI2cRaw(std::string node, uint8_t* data, size_t data_size) {
    I2cSlave i2cnode(m_session, UaNodeId(node.c_str(), 2));

    UaByteString bs;
    bs.setByteString(data_size, data);
    ERS_DEBUG(4, "Node: " << node << ", Data size: " << data_size
              << ", data[0]: " << static_cast<unsigned>(data[0]));

    try {
        i2cnode.writeValue(bs);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write I2c: " <<  e.what() << std::endl;
    }
}

void nsw::OpcClient::writeGPIO(std::string node, bool data) {
    DigitalIO gpio(m_session, UaNodeId(node.c_str(), 2));
    ERS_DEBUG(4, "Node: " << node << ", Data: " << data);

    try {
        gpio.writeValue(data);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write GPIO: " <<  e.what() << std::endl;
    }
}

bool nsw::OpcClient::readGPIO(std::string node) {
    DigitalIO gpio(m_session, UaNodeId(node.c_str(), 2));
    bool value = false;

    try {
        value = gpio.readValue();
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't read GPIO: " <<  e.what() << std::endl;
        throw;
    }
    return value;
}

std::vector<uint8_t> nsw::OpcClient::readI2c(std::string node) {
    I2cSlave i2cnode(m_session, UaNodeId(node.c_str(), 2));

    std::vector<uint8_t> result;
    try {
        auto bytestring = i2cnode.readValue();
        auto array = bytestring.data();
        auto length = bytestring.length();
        // copy array contents in a vector
        result.assign(array, array + length);
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't read I2c: " <<  e.what() << std::endl;
    }

    return result;
}

double nsw::OpcClient::readAnalogOutput(std::string node) {
    AnalogInput ainode(m_session, UaNodeId(node.c_str(), 2));
    return ainode.readValue();
}
