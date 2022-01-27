#ifndef NSWCONFIGURATION_HW_OPCMANAGER_H
#define NSWCONFIGURATION_HW_OPCMANAGER_H

#include <map>
#include <future>
#include <mutex>
#include <string>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/OpcClientPtr.h"

namespace nsw {
  class OpcManager
  {
    using Identifier = std::pair<std::string, std::string>;

  public:
    /**
     * \brief Get a pointer containing the OPC client
     *
     * \param deviceName Name of the device
     * \return internal::OpcClientPtr Object containing a pointer to the OPC client
     */
    internal::OpcClientPtr getConnection(const std::string& ipPort, const std::string& deviceName);

    /**
     * \brief Destroy the OPC Manager object
     *
     * Close all existing connections and notify handed out pointers.
     */
    ~OpcManager();
    OpcManager(const OpcManager&) = delete;
    OpcManager(OpcManager&&) = delete;
    OpcManager& operator=(OpcManager&&) = delete;
    OpcManager& operator=(const OpcManager&) = delete;
    OpcManager() = default;

  private:
    /**
     * \brief Start the timer for closing the connection
     *
     * \param pointer Pointer object handed out by \ref getConnection
     */
    void startRemoval(internal::OpcClientPtr* pointer);

    /**
     * \brief Create a new connection to the OPC server (one per device)
     *
     * \param identifier Name of the device
     */
    void add(const Identifier& identifier);

    /**
     * \brief Check if a connection to a device exists
     *
     * \param identifier Name of the device
     * \return true Connection does exist
     * \return false Connection does not exist
     */
    bool exists(const Identifier& identifier) const;

    /**
     * \brief Close a connection to a device
     *
     * \param identifier Name of the device
     */
    void remove(const Identifier& identifier);

    /**
     * \brief Stop the timer for closing a connection
     *
     * \param identifier Name of the device
     */
    void stopRemoval(const Identifier& identifier);

    /**
     * \brief Clean up finished removal threads
     */
    void cleanupThreads();

    friend internal::OpcClientPtr;
    std::map<internal::OpcClientPtr*, Identifier>
      m_pointers{};  //<! non-owning map of provided pointers
    std::map<Identifier, std::unique_ptr<OpcClient>> m_connections{};  //<! opened connections
    std::map<Identifier, std::promise<void>> m_promisesStop{};         //<! Promises to stop closing
    std::map<Identifier, std::promise<void>>
      m_promisesKill{};  //<! Promises to force stopping threads
    constexpr static std::chrono::seconds MAX_TIME_UNSUSED{
      10};  //<! Maximum amount of time a connection may remain open while not being used
    std::vector<std::future<void>> m_threads{};  //<! Container holding all removal threads
    mutable std::mutex m_mutex{};                //<! Mutex for synchronization
  };
}  // namespace nsw

#endif
