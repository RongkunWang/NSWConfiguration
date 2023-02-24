#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_CONFIG_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_CONFIG_H

#include <chrono>
#include <string>

namespace nsw::mon {
  /**
   * \brief Configuration for one monitoring group containing name and frequency
   *
   * A monitoring group is a set of parameters of one device type that is monitored with the same
   * frequency.
   */
  struct Config {
    std::string m_name;
    std::chrono::seconds m_frequency;
  };
}  // namespace nsw::mon

#endif
