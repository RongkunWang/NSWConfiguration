#include "NSWConfiguration/hw/OpcManager.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <mutex>
#include <ranges>
#include <stop_token>
#include <thread>
#include <utility>
#include <execution>

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ers/ers.h>

#include "NSWConfiguration/CommandNames.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/ScaStatus.h"

using namespace std::chrono_literals;

nsw::OpcManager::OpcManager() :
  m_backgroundThread{[this](const std::stop_token stopToken) { pingConnections(stopToken); }}
{}

nsw::OpcClientPtr nsw::OpcManager::getConnection(const std::string& ipPort, const std::string& deviceName)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ERS_DEBUG(2, "Get connection to " << deviceName);
  const auto identifier = Identifier{ipPort, deviceName};
  if (not exists(identifier)) {
    ERS_DEBUG(2, "Create new connection to " << deviceName);
    add(identifier);
  }
  warnBadConnection(identifier);
  return m_connections.at(identifier.port).at(identifier.name).get();
}

nsw::OpcManager::~OpcManager()
{
  ERS_DEBUG(2, "Destructing manager");
  doClear();
}

void nsw::OpcManager::clear()
{
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
  ERS_DEBUG(2, fmt::format("Create connection for {}:{}", identifier.port, identifier.name));
  if (not existsPort(identifier)) {
    m_connections.try_emplace(identifier.port);
  }
  m_connections.at(identifier.port).try_emplace(identifier.name, std::make_unique<OpcClient>(identifier.port));
}

bool nsw::OpcManager::exists(const Identifier& identifier) const
{
  return existsPort(identifier) and m_connections.at(identifier.port).contains(identifier.name);
}

bool nsw::OpcManager::existsPort(const Identifier& identifier) const
{
  return m_connections.contains(identifier.port);
}

void nsw::OpcManager::pingConnections(std::stop_token stopToken)
{
  while (not stopToken.stop_requested()) {
    const auto timeBefore = std::chrono::high_resolution_clock::now();
    ERS_DEBUG(2, "Pinging all connections");
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      for (const auto& [port, connectionsPerServer] : m_connections) {
        std::map<std::string, hw::ScaStatus::ScaStatus> status{};
        std::transform(std::cbegin(connectionsPerServer),
                      std::cend(connectionsPerServer),
                      std::inserter(status, std::end(status)),
                      [](const auto& pair) -> decltype(status)::value_type {
                        return {pair.first, testConnection(pair.first, pair.second.get())};
                      });
        analyzePingResults(port, status);
      }
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
    std::mutex mutex;
    std::unique_lock lock(mutex);
    std::condition_variable_any().wait_for(lock, stopToken, sleepTime, [] { return false; });
  }
}

void nsw::OpcManager::analyzePingResults(const std::string& port, const std::map<std::string, hw::ScaStatus::ScaStatus>& result)
{
  // Add port if not yet added
  if (not m_badConnections.contains(port)) {
    m_badConnections.try_emplace(port);
  }
  auto& badConnections = m_badConnections.at(port);

  // All good: Remove any bad devices
  if (std::ranges::all_of(result, [] (const auto& pair) { return pair.second == hw::ScaStatus::REACHABLE; })) {
    if (not badConnections.empty()) {
      ERS_LOG(fmt::format("Previously bad connections {} are recovered", badConnections | std::views::keys));
      badConnections.clear();
    }
    return;
  }

  // Add bad devices to list if not already there
  for (const auto& [name, status] : result) {
    if (not badConnections.contains(name) and status != hw::ScaStatus::REACHABLE) {
      ERS_LOG(fmt::format("Device {} is added to list of bad devices because {}", name, getRepresentation(status)));
      badConnections.try_emplace(name, status);
    }
    if (badConnections.contains(name) and status == hw::ScaStatus::REACHABLE) {
      ERS_LOG(fmt::format("Previously bad connection {} is recovered", name));
      badConnections.erase(name);
    }
    if (badConnections.contains(name) and status != badConnections.at(name)) {
      ERS_LOG(fmt::format("Status of device {} is updated from {} to {}",
                          name,
                          hw::ScaStatus::getRepresentation(badConnections.at(name)),
                          hw::ScaStatus::getRepresentation(status)));
      badConnections.at(name) = status;
    }
  }

  // Server offline, start checking for reconnect
  if (std::ranges::all_of(
        result, [](const auto& pair) { return pair.second == hw::ScaStatus::SERVER_OFFLINE; }) and
      not m_reconnectThreads.contains(port)) {
    ERS_INFO("Detected offline OPC server. Testing for restart");
    std::vector<std::string> deviceNames{};
    deviceNames.reserve(std::size(m_connections.at(port)));
    std::ranges::copy(m_connections.at(port) | std::views::keys, std::back_inserter(deviceNames));
    m_reconnectThreads.try_emplace(port, [this, port, deviceNames](const std::stop_token stopToken) { testServerRestart(stopToken, port, deviceNames); });
  }
}

void nsw::OpcManager::warnBadConnection(const Identifier& id) const
{
  if (m_badConnections.contains(id.port) and m_badConnections.at(id.port).contains(id.name)) {
    ERS_LOG(fmt::format("Received request for a connection to {}.{} that was identified as bad due "
                        "to {}. Operation might fail",
                        id.port,
                        id.name,
                        getRepresentation(m_badConnections.at(id.port).at(id.name))));
  }
}

void nsw::OpcManager::doClear()
{
  m_backgroundThread.request_stop();
  if (m_backgroundThread.joinable()) {
    m_backgroundThread.join();
  }
  for (auto& [port, thread] : m_reconnectThreads) {
    thread.request_stop();
  }
  for (auto& [port, thread] : m_reconnectThreads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  m_connections.clear();
  m_badConnections.clear();
}

void nsw::OpcManager::testServerRestart(const std::stop_token stopToken, const std::string& server, const std::vector<std::string>& deviceNames)
{
  const auto sleepTime{30s};
  while (not checkServerStatus(server, deviceNames) and not stopToken.stop_requested()) {
    std::mutex mutex;
    std::unique_lock lock(mutex);
    std::condition_variable_any().wait_for(lock, stopToken, sleepTime, [] { return false; });
  }
  if (stopToken.stop_requested()) {
    return;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  m_connections.at(server).clear();
  m_badConnections.at(server).clear();
  if (m_commandSender.valid()) {
    ERS_LOG("Detected restart of OPC server. Sending command to application (disabled at the moment)");
    // m_commandSender.send(nsw::commands::RECOVER_OPC_MESSAGE);
  }
}

bool nsw::OpcManager::checkServerStatus(const std::string& server, const std::vector<std::string>& deviceNames)
{
  const auto tryConnect = [&server](const std::string& deviceName) {
    try {
      auto connection = OpcClient(server);
      return OpcManager::testConnection(deviceName, &connection);
    } catch (const nsw::OpcConnectionIssue&) {
      return hw::ScaStatus::SERVER_OFFLINE;
    }
  };
  return std::ranges::none_of(deviceNames | std::views::transform(tryConnect), [] (const auto result) { return result == hw::ScaStatus::SERVER_OFFLINE; });
}
