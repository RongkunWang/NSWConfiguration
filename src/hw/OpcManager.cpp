#include "NSWConfiguration/hw/OpcManager.h"

#include <utility>

#include <fmt/core.h>
#include <fmt/format.h>

#include <ers/ers.h>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/OpcClientPtr.h"

nsw::internal::OpcClientPtr nsw::OpcManager::getConnection(const std::string& ipPort, const std::string& deviceName)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  const auto identifier = std::make_pair(ipPort, deviceName);
  cleanupThreads();
  if (not exists(identifier)) {
    add(identifier);
  } else {
    stopRemoval(identifier);
  }
  auto pointer = internal::OpcClientPtr(this, m_connections.at(identifier).get());
  m_pointers.insert_or_assign(&pointer, identifier);
  return pointer;
}

nsw::OpcManager::~OpcManager()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ERS_INFO("Destructing manager");
  for (auto& pair : m_promisesKill) {
    pair.second.set_value();
  }
  for (auto& pair : m_promisesStop) {
    pair.second.set_value();
  }
  for (auto& thread : m_threads) {
    thread.wait();
  }
  for (auto pair : m_pointers) {
    pair.first->invalidateManager();
  }
}

void nsw::OpcManager::startRemoval(nsw::internal::OpcClientPtr* pointer)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  const auto identifier = m_pointers.at(pointer);
  ERS_INFO(fmt::format("Starting removal for {}", identifier.second));
  m_pointers.erase(pointer);
  m_promisesStop.insert_or_assign(identifier, std::promise<void>());
  m_promisesKill.insert_or_assign(identifier, std::promise<void>());
  m_threads.push_back(std::async(
    std::launch::async,
    [this, identifier](std::future<void>&& stop, std::future<void>&& kill) {
      if (stop.wait_for(MAX_TIME_UNSUSED) != std::future_status::ready) {
        if (kill.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
          return;
        }
        std::lock_guard<std::mutex> lockRemoval(m_mutex);
        remove(identifier);
      }
    },
    m_promisesStop.at(identifier).get_future(),
    m_promisesKill.at(identifier).get_future()));
}

void nsw::OpcManager::add(const Identifier& identifier)
{
  if (not exists(identifier)) {
    ERS_INFO(fmt::format("Create connection for {}", identifier.second));
    m_connections.try_emplace(identifier, std::make_unique<OpcClient>(identifier.first));
  }
}

bool nsw::OpcManager::exists(const Identifier& identifier) const
{
  return m_connections.find(identifier) != std::end(m_connections);
}

void nsw::OpcManager::remove(const Identifier& identifier)
{
  if (exists(identifier)) {
    ERS_INFO(fmt::format("Close connection for {}", identifier.second));
    m_connections.erase(identifier);
    m_promisesStop.erase(identifier);
    m_promisesKill.erase(identifier);
  }
}

void nsw::OpcManager::stopRemoval(const Identifier& identifier)
{
  ERS_INFO(fmt::format("Stopping removal for {}", identifier.second));
  m_promisesStop.at(identifier).set_value();
}

void nsw::OpcManager::cleanupThreads()
{
  m_threads.erase(std::remove_if(std::begin(m_threads),
                                 std::end(m_threads),
                                 [](auto&& future) {
                                   return future.wait_for(std::chrono::seconds(0)) ==
                                          std::future_status::ready;
                                 }),
                  std::end(m_threads));
}
