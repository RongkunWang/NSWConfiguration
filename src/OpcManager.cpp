#include "NSWConfiguration/OpcManager.h"

#include <mutex>
#include <shared_mutex>

std::map<std::string, std::unique_ptr<nsw::OpcClient>>
                  nsw::OpcManager::m_clients;
std::shared_mutex nsw::OpcManager::m_mutex;

std::unique_ptr<nsw::OpcClient>& nsw::OpcManager::getConnection(
  const std::string& opcserver_ipport) {
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  if (m_clients.find(opcserver_ipport) == std::end(m_clients)) {
    addConnection(opcserver_ipport);
  }
  return m_clients.at(opcserver_ipport);
}

void nsw::OpcManager::addConnection(const std::string& opcserver_ipport) {
  std::lock_guard<std::shared_mutex> lock(m_mutex);
  m_clients.emplace(opcserver_ipport,
                    std::make_unique<nsw::OpcClient>(opcserver_ipport));
}