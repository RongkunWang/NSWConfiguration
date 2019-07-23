#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::ARTConfig::ARTConfig(ptree config): FEConfig(config) {
    // std::cout << "ART Constructor!" << std::endl;
}

int nsw::ARTConfig::register0_test_00() { 
    return m_config.get_child("register0").get<int>("test_00");
}
