//
// conf_ro_bc_offset   <= control_reg(6 downto 0);   -- DEFAULT VALUE "000 1000"
// conf_ro_window_size <= control_reg(10 downto 7);  -- DEFAULT VALUE "0010"
// conf_ro_en          <= control_reg(11);           -- DEFAULT VALUE '0'
// conf_bcid_offset    <= control_reg(23 downto 12); -- DEFAULT VALUE "0000 0000 0111"
// conf_startIdleState <= control_reg(24);           -- DEFAULT VALUE '1'
// conf_ocr_en         <= control_reg(25);           -- DEFAULT VALUE ‘0'
// conf_ttc_calib      <= control_reg(29 downto 26); -- DEFAULT VALUE ‘0000' HYPOTHESIS
//

#include <string>
#include "boost/optional.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "NSWConfiguration/PadTriggerSCAConfig.h"

nsw::PadTriggerSCAConfig::PadTriggerSCAConfig(const ptree& config):
    SCAConfig(config)
{
  m_L1AReadoutLatency = -1;
  m_L1AReadoutNBC = -1;
  m_L1AReadoutEnable = -1;
  m_pFEBBCIDOffset = -1;
  m_StartIdleState = -1;
  m_OCREnable = -1;
  m_TTCCalib = -1;
}

void nsw::PadTriggerSCAConfig::dump() {
    // std::cout << std::endl;
}

int nsw::PadTriggerSCAConfig::ControlRegister() const {
  int reg = 0;
  reg += (L1AReadoutLatency() <<  0);
  reg += (L1AReadoutNBCMode() <<  7);
  reg += (L1AReadoutEnable()  << 11);
  reg += (pFEBBCIDOffset()    << 12);
  reg += (StartIdleState()    << 24);
  reg += (OCREnable()         << 25);
  reg += (TTCCalib()          << 26);
  return reg;
}

int nsw::PadTriggerSCAConfig::L1AReadoutLatency() const {
  if (m_L1AReadoutLatency == -1)
    return m_config.get<int>("L1AReadoutLatency");
  return m_L1AReadoutLatency;
}

int nsw::PadTriggerSCAConfig::L1AReadoutNBC() const {
  if (m_L1AReadoutNBC == -1)
    return m_config.get<int>("L1AReadoutNBC");
  return m_L1AReadoutNBC;
}

int nsw::PadTriggerSCAConfig::L1AReadoutNBCMode() const {
  if (L1AReadoutNBC() == -1)
    return -1;
  else if (L1AReadoutNBC() == 1)
    return 0;
  else if (L1AReadoutNBC() == 2)
    return 1;
  else if (L1AReadoutNBC() == 3)
    return 2;
  auto msg = "L1AReadoutNBCMode is confused by NBC = " + std::to_string(L1AReadoutNBC());
  throw std::runtime_error(msg);
}

int nsw::PadTriggerSCAConfig::L1AReadoutEnable() const {
  if (m_L1AReadoutEnable == -1)
    return m_config.get<int>("L1AReadoutEnable");
  return m_L1AReadoutEnable;
}

int nsw::PadTriggerSCAConfig::pFEBBCIDOffset() const {
  if (m_pFEBBCIDOffset == -1)
    return m_config.get<int>("pFEBBCIDOffset");
  return m_pFEBBCIDOffset;
}

int nsw::PadTriggerSCAConfig::StartIdleState() const {
  if (m_StartIdleState == -1)
    return m_config.get<int>("StartIdleState");
  return m_StartIdleState;
}

int nsw::PadTriggerSCAConfig::OCREnable() const {
  if (m_OCREnable == -1)
    return m_config.get<int>("OCREnable");
  return m_OCREnable;
}

int nsw::PadTriggerSCAConfig::TTCCalib() const {
  if (m_TTCCalib == -1)
    return m_config.get<int>("TTCCalib");
  return m_TTCCalib;
}

bool nsw::PadTriggerSCAConfig::ConfigRepeaters() const {
  return m_config.get<bool>("ConfigRepeaters");
}

bool nsw::PadTriggerSCAConfig::ConfigVTTx() const {
  return m_config.get<bool>("ConfigVTTx");
}

bool nsw::PadTriggerSCAConfig::ConfigControlRegister() const {
  return m_config.get<bool>("ConfigControlRegister");
}
