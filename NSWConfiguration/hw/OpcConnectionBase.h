#ifndef NSWCONFIGURATION_HW_OPCCONNECTIONBASE_H
#define NSWCONFIGURATION_HW_OPCCONNECTIONBASE_H

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ScaStatus.h"

namespace nsw::hw {
  /**
   * \brief Base class for HW interfaces that have a connection to the OPC server
   */
  class OpcConnectionBase
  {
  public:
    /**
     * \brief Constructor
     *
     * \param manager Link to OPC manager
     * \param opcServerIp IP address of server
     * \param scaAddress SCA address (OPC node ID)
     */
    OpcConnectionBase(nsw::OpcManager& manager, std::string opcServerIp, std::string scaAddress);

    /**
     * \brief Try to ping the device
     *
     * \return ScaStatus SCA reachable, unreachable, or server offline
     */
    [[nodiscard]] ScaStatus::ScaStatus ping() const;

    /**
     * \brief Check if the device is reachable
     *
     * \return reachable
     */
    [[nodiscard]] bool reachable() const;

  protected:
    /**
     * \brief Get the Opc Server IP
     *
     * \return std::string IP address of OPC server
     */
    [[nodiscard]] std::string getOpcServerIp() const { return m_opcServerIp; }

    /**
     * \brief Get a connection to the OPC server
     *
     * \return OpcClientPtr Pointer to connection
     */
    [[nodiscard]] OpcClientPtr getConnection() const
    {
      return m_opcManager.get().getConnection(m_opcServerIp, m_scaAddress);
    }

  private:
    std::string m_scaAddress;                                      //!< SCA address
    std::string m_opcServerIp;                                     //!< OPC server IP address
    mutable std::reference_wrapper<nsw::OpcManager> m_opcManager;  //!< Pointer to OpcManager
  };
}  // namespace nsw::hw

#endif
