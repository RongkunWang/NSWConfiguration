#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/Utility.h"

using boost::property_tree::ptree;

nsw::ARTConfig::ARTConfig(ptree config):
    m_config(config),
    core(config.get_child(ART_CORE_NAME), ART_CORE_NAME, ART_CORE_REGISTERS),
    pll (config.get_child(ART_PLL_NAME),  ART_PLL_NAME,  ART_PLL_REGISTERS)
{
    // std::cout << "ART Constructor!" << std::endl;
}

int nsw::ARTConfig::register0_test_00() { 
    return m_config.get_child("register0").get<int>("test_00");
}

int nsw::ARTConfig::art_core_cfg_deser_flagmask() {
    return core.getRegisterValue("03:registerMyName_3", "cfg_deser_flagmask");
}
