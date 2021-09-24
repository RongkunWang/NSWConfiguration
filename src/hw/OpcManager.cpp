#include "NSWConfiguration/hw/OpcManager.h"

#include <mutex>
#include <shared_mutex>

std::map<std::string, OpcClientPtr> nsw::OpcManager::m_clients;
std::shared_mutex nsw::OpcManager::m_mutex;

OpcClientPtr nsw::OpcManager::getConnection(const std::string& opcserver_ipport)
{
  return std::make_unique<nsw::OpcClient>(opcserver_ipport);

  // Code for one connection per IP (or board)
  // if (exists(opcserver_ipport)) {
  //   addConnection(opcserver_ipport);
  // }
  // std::shared_lock<std::shared_mutex> lock(m_mutex);
  // return m_clients.at(opcserver_ipport);
}

void nsw::OpcManager::addConnection([[maybe_unused]] const std::string& opcserver_ipport)
{
  // Code for one connection per IP (or board)
  // std::lock_guard<std::shared_mutex> lock(m_mutex);
  // m_clients.emplace(opcserver_ipport,
  //                   );
}

bool nsw::OpcManager::exists(const std::string& opcserver_ipport)
{
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  return m_clients.find(opcserver_ipport) == std::end(m_clients);
}
