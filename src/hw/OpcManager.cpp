#include "NSWConfiguration/hw/OpcManager.h"

#include <algorithm>
#include <chrono>
#include <ers/ers.h>
#include <thread>
#include <utility>
#include <execution>

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/OpcClient.h"

using namespace std::chrono_literals;


nsw::OpcClientPtr nsw::OpcManager::getConnection(const std::string& ipPort, const std::string& deviceName)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ERS_DEBUG(2, "Get connection to " << deviceName);
  const auto identifier = std::make_pair(ipPort, deviceName);
  if (m_connections.empty()) {
    m_backgroundThread = std::async(std::launch::async,
                              &OpcManager::pingConnections,
                              this,
                              m_stopBackgroundThread.get_future());
  }
  if (not exists(identifier)) {
    ERS_DEBUG(2, "Create new connection to " << deviceName);
    add(identifier);
  }
  return m_connections.at(identifier).get();
}

nsw::OpcManager::~OpcManager()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ERS_DEBUG(2, "Destructing manager");
  doClear();
}

void nsw::OpcManager::clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ERS_DEBUG(2, "Clear");
  doClear();
}

nsw::hw::ScaStatus::ScaStatus nsw::OpcManager::testConnection(const std::string& name, const OpcClient* connection)
{
  try {
    const auto status = connection->readScaOnline(name);
    if (not status) {
      return hw::ScaStatus::UNREACHABLE;
    }
    return hw::ScaStatus::REACHABLE;
  } catch (const nsw::OpcConnectionIssue&) {
    return hw::ScaStatus::SERVER_OFFLINE;
  } catch (const nsw::OpcReadWriteIssue&) {
    return hw::ScaStatus::SERVER_OFFLINE;
  }
}

void nsw::OpcManager::add(const Identifier& identifier)
{
  if (not exists(identifier)) {
    ERS_DEBUG(2, fmt::format("Create connection for {}", identifier.second));
    m_connections.try_emplace(identifier, std::make_unique<OpcClient>(identifier.first));
  }
}

bool nsw::OpcManager::exists(const Identifier& identifier) const
{
  return m_connections.find(identifier) != std::end(m_connections);
}

void nsw::OpcManager::pingConnections(std::future<void>&& stop) const
{
  while (stop.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    const auto timeBefore = std::chrono::high_resolution_clock::now();
    ERS_DEBUG(2, "Pinging all connections");
    bool crashed{false};
    std::for_each(std::execution::par, std::cbegin(m_connections), std::cend(m_connections), [this, &crashed] (const auto& pair) {
      const auto& name = pair.first.second;
      const auto& connection = pair.second;
      try {
        static_cast<void>(connection->readScaOnline(name));
      } catch (const nsw::OpcReadWriteIssue& ex) {
        ers::warning(OpcManagerPingIssue(ERS_HERE, ex.what()));
        if (m_commandSender.valid()) {
          crashed = true;
        } else {
          throw;
        }
      }
    });
    if (crashed) {
      if (m_commandSender.valid()) {
        m_commandSender.send(nsw::commands::RECOVER_OPC_MESSAGE);
      }
      break;
    }
    ERS_DEBUG(2, "Done pinging all connections");
    const auto sleepTime =
      std::max(0ms,
               std::chrono::duration_cast<std::chrono::milliseconds>(
                 PING_INTERVAL - (std::chrono::high_resolution_clock::now() - timeBefore)));
    ERS_DEBUG(2, fmt::format("Sleeping for {}", std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime)));
    if (sleepTime == 0s) {
      ers::log(OpcManagerPingFrequency(ERS_HERE));
    }
    stop.wait_for(sleepTime);
  }
}

void nsw::OpcManager::doClear()
{
  if (m_backgroundThread.valid()) {
    m_stopBackgroundThread.set_value();
  }
  if (m_backgroundThread.valid()) {
    m_backgroundThread.wait();
  }
  m_connections.clear();
  m_stopBackgroundThread = std::promise<void>();
}
