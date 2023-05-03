#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ScaStatus.h"
#include "fmt/core.h"

nsw::hw::OpcConnectionBase::OpcConnectionBase(nsw::OpcManager& manager,
                                              std::string opcServerIp,
                                              std::string scaAddress) :
  m_scaAddress{std::move(scaAddress)}, m_opcServerIp{std::move(opcServerIp)}, m_opcManager{manager}
{}

nsw::hw::ScaStatus::ScaStatus nsw::hw::OpcConnectionBase::ping() const
{
  try {
    auto connection = OpcClient(m_opcServerIp);
    return OpcManager::testConnection(m_scaAddress, &connection);
  } catch (const nsw::OpcConnectionIssue&) {
    return ScaStatus::SERVER_OFFLINE;
  }
}

bool nsw::hw::OpcConnectionBase::reachable() const
{
  return ping() == ScaStatus::REACHABLE;
}
