#include <memory>
#include <string>

#include "NSWConfiguration/ConfigSender.h"

nsw::ConfigSender::ConfigSender() {
}

void nsw::ConfigSender::sendRaw(std::string opcserver_ipport, std::string node, uint8_t* data, size_t data_size) {
    // Add OpcClient to the map if it doesn't exist yet
    if (m_clients.find(opcserver_ipport) == m_clients.end()) {
        m_clients.emplace(opcserver_ipport, std::make_unique<nsw::OpcClient>(opcserver_ipport));
    }

    m_clients[opcserver_ipport]->writeSpiSlaveRaw(node, data, data_size);
}

void nsw::ConfigSender::sendVmmConfig(nsw::VMMConfig cfg) {
    auto data = cfg.getByteArray();
    sendRaw(cfg.getOpcServerIp(), cfg.getVmmAddress(), data.data(), data.size());
}
