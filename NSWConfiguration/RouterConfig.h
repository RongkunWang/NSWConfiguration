/// Router Configuration class

#ifndef NSWCONFIGURATION_ROUTER_H_
#define NSWCONFIGURATION_ROUTER_H_

#include <vector>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "NSWConfiguration/SCAConfig.h"

using boost::property_tree::ptree;

namespace nsw {

class RouterConfig: public SCAConfig {
 private:
    bool dummy;

 public:
    explicit RouterConfig(const ptree& config);
    ~RouterConfig() {}

    bool CrashOnClkReadyFailure() const;
    bool CrashOnConfigFailure() const;
    std::string Sector() const;
    void dump() const;

    uint8_t id() const;
    uint8_t id_sector() const;
    uint8_t id_layer() const;
    uint8_t id_endcap() const;
    void id_check() const;
    void id_crash() const;

 private:
    // NAMING CONVENTION:
    // Name: Router_LZ
    // Sector: XYY
    // ---------------
    // Endcap: X = A,C
    // Sector: YY = 01, 02, ..., 16
    // Layer:  Z = 0, 1, ..., 7
    std::string convention = "Router_LZ";
    std::string name_error = "This Router doesnt follow the naming convention";

};

}

#endif  // NSWCONFIGURATION_ROUTER_H_
