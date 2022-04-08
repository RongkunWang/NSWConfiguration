#include "NSWConfiguration/monitoring/RocConfigurationRegisters.h"

#include "NSWConfiguration/monitoring/Helper.h"

nsw::mon::RocConfigurationRegisters::RocConfigurationRegisters(const nsw::hw::DeviceManager& deviceManager):
  m_devices{deviceManager.getFebs()}
{}

void nsw::mon::RocConfigurationRegisters::monitor(ISInfoDictionary* isDict, const std::string_view serverName) const
{
  nsw::mon::internal::monitorAndPublish(
    m_devices.get(), isDict, serverName, NAME, nsw::mon::RocConfigurationRegisters::getData);
}

nsw::mon::is::RocConfiguration nsw::mon::RocConfigurationRegisters::getData(const nsw::hw::FEB& feb)
{
  auto isObject = nsw::mon::is::RocConfiguration{};
  isObject.reg115 = feb.getRoc().readRegister(115);
  return isObject;
}