#include <memory>
#include <string>

#include "NSWConfiguration/ConfigSender.h"

nsw::ConfigSender::ConfigSender() {
}

void nsw::ConfigSender::addOpcClientIfNew(std::string opcserver_ipport) {
    if (m_clients.find(opcserver_ipport) == m_clients.end()) {
        m_clients.emplace(opcserver_ipport, std::make_unique<nsw::OpcClient>(opcserver_ipport));
    }
}

void nsw::ConfigSender::sendSpiRaw(std::string opcserver_ipport, std::string node, uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, data, data_size);
}

void nsw::ConfigSender::sendI2CRaw(std::string opcserver_ipport, std::string node, uint8_t* data, size_t data_size) {
    addOpcClientIfNew(opcserver_ipport);
    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, data, data_size);
}

void nsw::ConfigSender::sendVmmConfig(nsw::VMMConfig cfg) {
    auto data = cfg.getByteVector();
    sendSpiRaw(cfg.getOpcServerIp(), cfg.getAddress(), data.data(), data.size());
}

void nsw::ConfigSender::sendRocConfig(nsw::ROCConfig cfg) {
    // auto roc_registers = cfg.getRegisters();
    // Loop over each I2C config of the ROC
    // auto (auto register : registers) {
    //     data = register.getByteVector();
    //     sendI2CRaw(cfg.getOpcServerIp(), register.getAddress(), data.data(), data.size());
    // }
}
