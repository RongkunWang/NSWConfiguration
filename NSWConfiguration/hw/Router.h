#ifndef NSWCONFIGURATION_HW_ROUTER_H
#define NSWCONFIGURATION_HW_ROUTER_H

#include "NSWConfiguration/RouterConfig.h"

namespace nsw::hw {
  /**
   * \brief Class representing a Router
   *
   * The SCA GPIO connections are defined in:
   *  - http://cern.ch/go/p8jJ
   * The SCA connections on the Router are defined in:
   *  - http://cern.ch/go/Mm6j
   */
  class Router
  {
  public:
    /**
     * \brief Constrctor from a \ref RouterConfig object
     */
    explicit Router(const RouterConfig& config);

    /**
     * \brief Write the full Router configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Get the \ref RouterConfig object associated with this Router object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] RouterConfig& getConfig() { return m_config; }
    [[nodiscard]] const RouterConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    RouterConfig m_config;
    std::string m_opcserverIp;  //!< address and port of the Opc Server
    std::string m_scaAddress;   //!< SCA address of Router item in the Opc address space
  };
}  // namespace nsw::hw

#endif
