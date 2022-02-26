#ifndef NSWCONFIGURATION_HW_OPCMANAGER_H
#define NSWCONFIGURATION_HW_OPCMANAGER_H

#include <map>
#include <future>
#include <mutex>
#include <string>

#include <ers/ers.h>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/CommandSender.h"

ERS_DECLARE_ISSUE(nsw,
                  OpcManagerPingFrequency,
                  "Cannot ping OPC connection with required frequency",
                  )

ERS_DECLARE_ISSUE(nsw,
                  OpcManagerPingIssue,
                  "Issue pinging a connection " << message << ". Sending recovery request.",
                  ((std::string) message)
                  )
namespace nsw {
  class OpcManager
  {
    using Identifier = std::pair<std::string, std::string>;

  public:
    /**
     * \brief Get a pointer containing the OPC client
     *
     * \param deviceName Name of the device
     * \return OpcClientPtr Object containing a pointer to the OPC client
     */
    OpcClientPtr getConnection(const std::string& ipPort, const std::string& deviceName);

    /**
     * \brief Destroy the OPC Manager object
     *
     * Close all existing connections and notify handed out pointers.
     */
    ~OpcManager();
    OpcManager() = default;
    OpcManager(const OpcManager&) = delete;
    OpcManager(OpcManager&&) = delete;
    OpcManager& operator=(OpcManager&&) = delete;
    OpcManager& operator=(const OpcManager&) = delete;

    /**
     * \brief Close all connections
     */
    void clear();

    /**
     * \brief Set the command sender to the RC application
     *
     * \param sender Command sender to RC application
     */
    void setCommandSender(nsw::CommandSender&& sender) { m_commandSender = std::move(sender); }

  private:
    /**
     * \brief Ping all connections to keep them open
     *
     * Executes a loop to ping them every \ref PING_INTERVAL seconds
     *
     * \param stop Future to stop thread
     */
    void pingConnections(std::future<void>&& stop) const;

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
     * \brief Close all connections (implementation)
     */
    void doClear();

    std::map<Identifier, std::unique_ptr<OpcClient>> m_connections{};  //<! opened connections
    constexpr static std::chrono::seconds PING_INTERVAL{10};           //<! Delay between two pings
    std::promise<void> m_stopBackgroundThread{};  //<! Background thread to ping all connections
    std::future<void> m_backgroundThread{};       //<! Background thread to ping all connections
    nsw::CommandSender m_commandSender{};         //<! Name of the application for recovery callback
    mutable std::mutex m_mutex{};                 //<! Mutex for synchronization
  };
}  // namespace nsw

#endif
