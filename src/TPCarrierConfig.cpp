#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/TPCarrierConfig.h"

nsw::TPCarrierConfig::TPCarrierConfig(const boost::property_tree::ptree& config) :
  SCAConfig(config)
{
}

uint32_t nsw::TPCarrierConfig::RJOutSel() const {
  return m_config.get("RJOutSel", nsw::carrier::RJOUT_SEL_STGC);
}

