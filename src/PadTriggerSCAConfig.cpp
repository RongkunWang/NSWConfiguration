//
// "User": information which is supplied by the user via config file/db
// "Real": information which is gathered from the board itself
//
// conf_ro_bc_offset   <= control_reg(6 downto 0);   -- DEFAULT VALUE "0001000"
// conf_ro_window_size <= control_reg(10 downto 7);  -- DEFAULT VALUE "0010"
// conf_bcid_offset    <= control_reg(14 downto 11); -- DEFAULT VALUE "0111"
//
#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/PadTriggerSCAConfig.h"

#include "boost/property_tree/json_parser.hpp"

nsw::PadTriggerSCAConfig::PadTriggerSCAConfig(ptree config):
    SCAConfig(config)
{
    m_UserL1AReadoutLatency = -1;
    m_UserL1AReadoutNBC     = -1;
    m_UserL1AReadoutEnable  = -1;
    m_UserpFEBBCIDOffset    = -1;
    m_RealControlRegister   = -1;
    m_RealL1AReadoutLatency = -1;
    m_RealL1AReadoutNBCMode = -1;
    m_RealL1AReadoutEnable  = -1;
    m_RealpFEBBCIDOffset    = -1;
}

void nsw::PadTriggerSCAConfig::dump() {
    // std::cout << std::endl;
}

int nsw::PadTriggerSCAConfig::UserControlRegister() {
  if (UserL1AReadoutLatency() == -1 ||
      UserL1AReadoutNBCMode() == -1 ||
      UserL1AReadoutEnable()  == -1 ||
      UserpFEBBCIDOffset()    == -1)
      return -1;
  int reg = 0;
  reg += (UserL1AReadoutLatency() <<  0);
  reg += (UserL1AReadoutNBCMode() <<  7);
  reg += (UserpFEBBCIDOffset()    << 11);
  reg += (UserL1AReadoutEnable()  << 15);
  return reg;
}

int nsw::PadTriggerSCAConfig::UserL1AReadoutLatency() {
  return (m_UserL1AReadoutLatency != -1) ? m_UserL1AReadoutLatency : m_config.get<int>("L1AReadoutLatency");
}

int nsw::PadTriggerSCAConfig::UserL1AReadoutNBC() {
  return (m_UserL1AReadoutNBC != -1) ? m_UserL1AReadoutNBC : m_config.get<int>("L1AReadoutNBC");
}

int nsw::PadTriggerSCAConfig::UserL1AReadoutNBCMode() {
  if (UserL1AReadoutNBC() == -1)
    return -1;
  else if (UserL1AReadoutNBC() == 1)
    return 0;
  else if (UserL1AReadoutNBC() == 2)
    return 1;
  else if (UserL1AReadoutNBC() == 3)
    return 2;
  auto msg = "UserL1AReadoutNBCMode is confused by NBC = " + std::to_string(UserL1AReadoutNBC());
  throw std::runtime_error(msg);
}

int nsw::PadTriggerSCAConfig::UserL1AReadoutEnable() {
  return (m_UserL1AReadoutEnable != -1) ? m_UserL1AReadoutEnable : m_config.get<int>("L1AReadoutEnable");
}

int nsw::PadTriggerSCAConfig::UserpFEBBCIDOffset() {
  return (m_UserpFEBBCIDOffset != -1) ? m_UserpFEBBCIDOffset : m_config.get<int>("pFEBBCIDOffset");
}

void nsw::PadTriggerSCAConfig::SetRealControlRegister(int val) {
  m_RealControlRegister = val;
  SetRealL1AReadoutLatency( (val >>  0) & 0x7f);
  SetRealL1AReadoutNBCMode( (val >>  7) & 0x0f);
  SetRealpFEBBCIDOffset(    (val >> 11) & 0x0f);
  SetRealL1AReadoutEnable(  (val >> 15) & 0x01);
}


int nsw::PadTriggerSCAConfig::RealL1AReadoutNBC() {
  if (RealL1AReadoutNBCMode() == -1)
    return -1;
  else if (RealL1AReadoutNBCMode() == 0)
    return 1;
  else if (RealL1AReadoutNBCMode() == 1)
    return 2;
  else if (RealL1AReadoutNBCMode() == 2)
    return 3;
  auto msg = "RealL1AReadoutNBCMode is confused by NBCMode = " + std::to_string(RealL1AReadoutNBCMode());
  throw std::runtime_error(msg);
}
