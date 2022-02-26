#include "NSWConfiguration/hw/OpcConnectionBase.h"
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
    const auto status = connection.readScaOnline(m_scaAddress);
    if (not status) {
      return ScaStatus::UNREACHABLE;
    }
    return ScaStatus::REACHABLE;
  } catch (const nsw::OpcConnectionIssue&) {
    ERS_INFO("Reconnection failed");
    return ScaStatus::SERVER_OFFLINE;
  } catch (const nsw::OpcReadWriteIssue&) {
    ERS_INFO(fmt::format("Reconnection succeeded but SCA ({}) unreachable", m_scaAddress));
    return ScaStatus::SERVER_OFFLINE;
  }
}
