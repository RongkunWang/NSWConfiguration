//
// conf_ro_bc_offset   <= control_reg(6 downto 0);   -- DEFAULT VALUE "000 1000"
// conf_ro_window_size <= control_reg(10 downto 7);  -- DEFAULT VALUE "0010"
// conf_ro_en          <= control_reg(11);           -- DEFAULT VALUE '0'
// conf_bcid_offset    <= control_reg(23 downto 12); -- DEFAULT VALUE "0000 0000 0111"
// conf_startIdleState <= control_reg(24);           -- DEFAULT VALUE '1'
// conf_ocr_en         <= control_reg(25);           -- DEFAULT VALUE ‘0'
// conf_ttc_calib      <= control_reg(29 downto 26); -- DEFAULT VALUE ‘0000' HYPOTHESIS
//

#include <regex>
#include <ers/ers.h>
#include "NSWConfiguration/PadTriggerSCAConfig.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"

nsw::PadTriggerSCAConfig::PadTriggerSCAConfig(const boost::property_tree::ptree& config):
  SCAConfig(config),
  m_padtriggerfpga(config.get_child(PADTRIGGER_NAME), PADTRIGGER_NAME, PADTRIGGER_REGISTERS),
  m_LatencyScanStart(-1),
  m_LatencyScanNBC(-1)
{}

void nsw::PadTriggerSCAConfig::dump() {
    // std::cout << std::endl;
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

std::string nsw::PadTriggerSCAConfig::firmware() const {
  return m_config.get<std::string>("firmware");
}

uint32_t nsw::PadTriggerSCAConfig::firmware_dateword() const {

  uint32_t dateword = 0;
  const std::string& fw = firmware();

  // extract YYYY*MM*DD from firmware()
  //   [0-9]{N} means "N digits"
  //   .?       means "zero or one characters"
  const std::regex yyyy_mm_dd("[0-9]{4}.?[0-9]{2}.?[0-9]{2}");
  std::smatch matches;
  std::regex_search(fw, matches, yyyy_mm_dd);

  // check result
  if (matches.size() != 1) {
    ERS_INFO("Cannot extract YYYYMMDD from "
             << fw
             << ", returning "
             << dateword);
    return dateword;
  }

  // convert YYYY*MM*DD to YYYYMMDD
  for (const auto& match: matches) {
    const std::regex non_numeric("[^0-9]");
    const std::string datestr =
      std::regex_replace(std::string(match), non_numeric, "");
    dateword = static_cast<uint32_t>(std::stoul(datestr));
    break;
  }

  // return
  return dateword;
}
