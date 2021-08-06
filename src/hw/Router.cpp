#include "NSWConfiguration/hw/Router.h"

#include <stdexcept>

nsw::hw::Router::Router(const RouterConfig& config) :
  m_config(config), m_opcserverIp(config.getOpcServerIp()), m_scaAddress(config.getAddress())
{}

void nsw::hw::Router::writeConfiguration() const
{
  throw std::logic_error("Not implemented");
}
