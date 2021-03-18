#include "NSWConfiguration/ADDCConfig.h"

#include "NSWConfiguration/Constants.h"

#include <string>
#include <iostream>

#include "ers/ers.h"

using boost::property_tree::ptree;

nsw::ADDCConfig::ADDCConfig(const ptree& config):
    SCAConfig(config) {
    /// ADDC has 2 ART ASICs
  for (size_t i = 0; i < nsw::NUM_ART_PER_ADDC; i++) {
        std::string artname = "art" + std::to_string(i);
        if (config.find(artname) != config.not_found()) {
            // std::cout << "Found ART " << artname << std::endl;
            // ERS_DEBUG(3, "ART id:" << artname);
            m_arts.emplace_back(config.get_child(artname));
            m_arts.back().setName(artname);
            m_arts.back().setIndex(i);
        }
    }
}

void nsw::ADDCConfig::dump() {
    // std::cout << "Number of ART: " << m_arts.size() << std::endl;
}

