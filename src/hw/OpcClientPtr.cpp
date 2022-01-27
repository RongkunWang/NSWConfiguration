#include "NSWConfiguration/hw/OpcClientPtr.h"

#include <fmt/core.h>

#include <ers/ers.h>

#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/OpcManager.h"

nsw::internal::OpcClientPtr::OpcClientPtr(nsw::OpcManager* manager, nsw::OpcClient* client) :
  m_data{client}, m_manager{manager}
{}

nsw::internal::OpcClientPtr::~OpcClientPtr()
{
  if (m_manager != nullptr) {
    m_manager->startRemoval(this);
  } else {
    ERS_DEBUG(2, "Manager invalid");
  }
}

nsw::OpcClient& nsw::internal::OpcClientPtr::operator*()
{
  if (m_manager == nullptr) {
    throw std::runtime_error("Manager invalid. Connection dead");
  }
  return *m_data;
}

const nsw::OpcClient* nsw::internal::OpcClientPtr::operator->() const
{
  if (m_manager == nullptr) {
    throw std::runtime_error("Manager invalid. Connection dead");
  }
  return m_data;
}

void nsw::internal::OpcClientPtr::invalidateManager()
{
  m_manager = nullptr;
}
