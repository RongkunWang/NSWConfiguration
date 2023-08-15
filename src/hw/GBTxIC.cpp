#include "NSWConfiguration/hw/GBTxIC.h"

#include <algorithm>
#include <iterator>

#include "NSWConfiguration/GBTxConfig.h"
#include "NSWConfiguration/Issues.h"
#include "NSWConfiguration/L1DDCConfig.h"

nsw::hw::GBTxIC::GBTxIC(const L1DDCConfig& config) :
  ScaAddressBase(config.getOpcNodeId()),
  m_icConnection{config.getFidToFlx(), config.getFidToHost()},
  m_config(config.getGBTx(GBTX_IC_ID)),
  m_name(config.getName())
{}

void nsw::hw::GBTxIC::writeConfiguration() const
{
  writeConfiguration(m_config);
}

void nsw::hw::GBTxIC::writeConfiguration(const GBTxConfig& config) const
{
  const auto data = config.configAsVector();

  m_icConnection.sendCfg(data);
  m_tracker.update(data);
}

std::vector<std::uint8_t> nsw::hw::GBTxIC::readConfiguration() const
{
  auto data = m_icConnection.readCfg();
  m_tracker.validate(data);
  return data;
}

bool nsw::hw::GBTxIC::hasConfigurationErrors() const
{
  return not m_tracker.getErrors().empty();
}

void nsw::hw::GBTxIC::train(const bool trainEc, const std::chrono::microseconds& sleepTime) const
{
  return;
}
