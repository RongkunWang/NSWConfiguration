#ifndef NSWCONFIGURATION_HW_OPCMANAGER_H
#define NSWCONFIGURATION_HW_OPCMANAGER_H

#include <map>
#include <future>
#include <mutex>
#include <string>
#include <thread>

#include <ers/ers.h>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/CommandSender.h"
#include "NSWConfiguration/hw/ScaStatus.h"

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
    struct Identifier {
      std::string port{};
      std::string name{};
    };
    using ConnectionMap = std::map<std::string, std::map<std::string, std::unique_ptr<OpcClient>>>;
    using PingStatusMap = std::map<std::string, hw::ScaStatus::ScaStatus>;

  public:
    /**
     * @brief Constructor
     *
     * Start the background thread for pinging
     */
    OpcManager();

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

    /**
     * \brief Ping a connection and check if it is reachable
     *
     * \param name Name of the device
     * \param connection Corresponding OPC client
     */
    static hw::ScaStatus::ScaStatus testConnection(const std::string& name, const OpcClient* connection);

  private:
    /**
     * \brief Ping all connections to keep them open
     *
     * Executes a loop to ping them every \ref PING_INTERVAL seconds
     *
     * \param stopToken Token to request to stop background thread
     */
    void pingConnections(std::stop_token stopToken);

    /**
     * @brief Analyze the result of the pings of all devices of one server
     *
     * Add new devices with issues to tracker
     * Remove devices that had issues previously
     * Send command to SCA service in case of offline OPC server
     *
     * @param port OPC server port
     * @param result Result of the connection test
     */
    void analyzePingResults(const std::string& port, const PingStatusMap& result);

    /**
     * @brief Issue a message if a connection is requested for a device that is listed as bad
     *
     * @param id ID of the device
     */
    void warnBadConnection(const Identifier& id) const;

    /**
     * \brief Create a new connection to the OPC server (one per device)
     *
     * \param identifier ID of the device
     */
    void add(const Identifier& identifier);

    /**
     * \brief Check if a connection to a device exists
     *
     * \param identifier ID of the device
     * \return true Connection does exist
     * \return false Connection does not exist
     */
    bool exists(const Identifier& identifier) const;

    /**
     * \brief Check if any connection for a given port exists
     *
     * \param identifier ID of the device
     * \return true Connection to given port does exist
     * \return false Connection to given port does not exist
     */
    bool existsPort(const Identifier& identifier) const;

    /**
     * \brief Close all connections (implementation)
     */
    void doClear();

    /**
     * @brief Continously check if OPC server has come back online
     *
     * \param stopToken Token to request to stop thread
     * @param server OPC server
     * @param deviceNames Names of the devices connected to server
     */
    void testServerRestart(std::stop_token stopToken, const std::string& server, const std::vector<std::string>& deviceNames);

    /**
     * @brief Check if an OPC server has come back online
     *
     * @param server OPC server
     * @param deviceNames Names of the devices connected to server
     * @return true Server back online
     * @return false Server not online
     */
    static bool checkServerStatus(const std::string& server, const std::vector<std::string>& deviceNames);

    ConnectionMap m_connections{};        //<! opened connections
    std::map<std::string, PingStatusMap> m_badConnections{};  //<! opened connections which are not reachable
    constexpr static std::chrono::seconds PING_INTERVAL{10};           //<! Delay between two pings
    std::jthread m_backgroundThread{};     //<! Background thread to ping all connections
    std::map<std::string, std::jthread>
      m_reconnectThreads{};                //<! Threads to establish new connection to OPC
                                           // server when it went offline
    nsw::CommandSender m_commandSender{};  //<! Name of the application for recovery callback
    mutable std::mutex m_mutex{};          //<! Mutex for synchronization
  };
}  // namespace nsw

#endif
