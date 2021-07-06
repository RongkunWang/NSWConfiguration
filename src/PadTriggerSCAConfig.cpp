//
// conf_ro_bc_offset   <= control_reg(6 downto 0);   -- DEFAULT VALUE "000 1000"
// conf_ro_window_size <= control_reg(10 downto 7);  -- DEFAULT VALUE "0010"
// conf_ro_en          <= control_reg(11);           -- DEFAULT VALUE '0'
// conf_bcid_offset    <= control_reg(23 downto 12); -- DEFAULT VALUE "0000 0000 0111"
// conf_startIdleState <= control_reg(24);           -- DEFAULT VALUE '1'
// conf_ocr_en         <= control_reg(25);           -- DEFAULT VALUE ‘0'
// conf_ttc_calib      <= control_reg(29 downto 26); -- DEFAULT VALUE ‘0000' HYPOTHESIS
//

#include "NSWConfiguration/PadTriggerSCAConfig.h"
#include "NSWConfiguration/Constants.h"

using boost::property_tree::ptree;

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
  m_LatencyScanStart = -1;
  m_LatencyScanNBC = -1;
}

void nsw::PadTriggerSCAConfig::dump() {
    // std::cout << std::endl;
}

int nsw::PadTriggerSCAConfig::ControlRegister() const {
  int reg = 0;
  if (firmware() == "Pad_ro_ilaro_20200610.bit") {
    reg += (L1AReadoutLatency() <<  0);
    reg += (L1AReadoutNBCMode() <<  7);
    reg += (pFEBBCIDOffset()    << 11);
    reg += (L1AReadoutEnable()  << 15);
  } else {
    reg += (L1AReadoutLatency() <<  0);
    reg += (L1AReadoutNBCMode() <<  7);
    reg += (L1AReadoutEnable()  << 11);
    reg += (pFEBBCIDOffset()    << 12);
    reg += (StartIdleState()    << 24);
    reg += (OCREnable()         << 25);
    reg += (TTCCalib()          << 26);
  }
  return reg;
}

std::vector<uint32_t> nsw::PadTriggerSCAConfig::PFEBBCIDs(uint32_t val_07_00,
                                                          uint32_t val_15_08,
                                                          uint32_t val_23_16
                                                          ) const {
  std::vector<uint32_t> bcids_07_00 = {};
  std::vector<uint32_t> bcids_15_08 = {};
  std::vector<uint32_t> bcids_23_16 = {};
  std::vector<uint32_t> bcids  = {};
  size_t bit_position   = 0;
  while (bit_position < nsw::NUM_BITS_IN_WORD32) {
    bcids_07_00.push_back( (val_07_00 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bcids_15_08.push_back( (val_15_08 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bcids_23_16.push_back( (val_23_16 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bit_position += nsw::padtrigger::NUM_BITS_PER_PFEB_BCID;
  }
  for (const auto bcid: bcids_07_00)
    bcids.push_back(bcid);
  for (const auto bcid: bcids_15_08)
    bcids.push_back(bcid);
  for (const auto bcid: bcids_23_16)
    bcids.push_back(bcid);
  return bcids;
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

int nsw::PadTriggerSCAConfig::LatencyScanStart() const {
  if (m_LatencyScanStart == -1)
    return m_config.get<int>("LatencyScanStart");
  return m_LatencyScanStart;
}

int nsw::PadTriggerSCAConfig::LatencyScanNBC() const {
  if (m_LatencyScanNBC == -1)
    return m_config.get<int>("LatencyScanNBC");
  return m_LatencyScanNBC;
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

std::string nsw::PadTriggerSCAConfig::firmware() const {
  return m_config.get<std::string>("firmware");
}
