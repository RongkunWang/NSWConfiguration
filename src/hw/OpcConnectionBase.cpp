#include "NSWConfiguration/hw/OpcConnectionBase.h"

nsw::hw::OpcConnectionBase::OpcConnectionBase(nsw::OpcManager& manager,
                                              std::string opcServerIp,
                                              std::string scaAddress) :
  m_scaAddress{std::move(scaAddress)}, m_opcServerIp{std::move(opcServerIp)}, m_opcManager{manager}
{}
