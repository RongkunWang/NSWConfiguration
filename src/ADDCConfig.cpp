#include <string>
#include "boost/optional.hpp"
#include "NSWConfiguration/ADDCConfig.h"

nsw::ADDCConfig::ADDCConfig(ptree config):
    FEConfig(config) {

    // std::cout << "ADDCConfig constructor!" << std::endl;

    /// ADDC has 2 ART ASICs
    for (int i = 0; i < 2; i++) {
        std::string artname = "art" + std::to_string(i);
        if (config.find(artname) != config.not_found()) {
            // std::cout << "Found ART " << artname << std::endl;
            // ERS_DEBUG(3, "ART id:" << artname);
            m_arts.emplace_back(config.get_child(artname));
            m_arts.back().setName(artname);
        }
    }
}

void nsw::ADDCConfig::dump() {
    // std::cout << "Number of ART: " << m_arts.size() << std::endl;
}
