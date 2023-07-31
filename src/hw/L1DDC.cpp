#include "NSWConfiguration/hw/L1DDC.h"

nsw::hw::L1DDC::L1DDC(OpcManager& manager, const L1DDCConfig& config) :
  m_gbtxIc{config},
  m_gbtxI2c{std::to_array({GBTxI2c{manager, config, 1}, GBTxI2c{manager, config, 2}})},
  m_config{config}
{}

void nsw::hw::L1DDC::writeConfiguration()
{
  if (m_config.getConfigureGBTx(0)) {
    m_gbtxIc.writeConfiguration();
  }
  if (m_config.getConfigureGBTx(1)) {
    m_gbtxI2c.at(0).writeConfiguration();
  }
  if (m_config.getConfigureGBTx(2)) {
    m_gbtxI2c.at(1).writeConfiguration();
  }
}
