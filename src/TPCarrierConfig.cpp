#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/TPCarrierConfig.h"

nsw::TPCarrierConfig::TPCarrierConfig(const boost::property_tree::ptree& config) :
  SCAConfig(config)
{
}

uint32_t nsw::TPCarrierConfig::RJOutSel() const {
  if (m_config.get("RJOutSel_STGC", false)) {
    return nsw::carrier::RJOUT_SEL_STGC;
  } else if (m_config.get("RJOutSel_MM", false)) {
    return nsw::carrier::RJOUT_SEL_MM;
  } else {
    return m_config.get("RJOutSel", nsw::carrier::RJOUT_SEL_STGC);
  }
}

