#ifndef NSWCONFIGURATION_HW_ROUTER_H
#define NSWCONFIGURATION_HW_ROUTER_H

#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/Constants.h"

namespace nsw::hw {
  /**
   * \brief Class representing a Router
   *
   * Provides methods to read/write individual Router registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * The SCA register connections are defined in:
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
     * \brief Select the control mode of the Router
     */
    enum ControlMode {
      SCA = 0x0,     //!< SCA
      UNUSED = 0x1,  //!< UNUSED
      CMDIF = 0x2,   //!< CMDIF?
      VIO = 0x3      //!< VIO
    };

    /**
     * \brief SoftReset level for the Router
     */
    enum SoftReset { LOW = 0x0, HI = 0x1 };

    /**
     * \brief Read the full TP address space
     *
     * \returns a map of address to register value
     */
    std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration();

    /**
     * \brief Write the full Router configuration
     */
    void writeConfiguration();

    /**
     * \brief Write a value to a Router register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister([[maybe_unused]] std::uint8_t registerId,
                       [[maybe_unused]] std::uint32_t value);

    /**
     * \brief Read an individual Router register by its address
     */
    std::vector<std::uint8_t> readRegister([[maybe_unused]] std::uint8_t registerId);

    /**
     * \brief Set the control mode of the Router
     *
     *  Sets Router control mode (line 17 in excel)
     *
     * \param mode \ref ControlMode for the Router
     * \param index Which ctrlModX register to set
     */
    void setControlMode(const OpcClientPtr& opcConnection,
                        const ControlMode& mode,
                        std::size_t index) const;

    /**
     * \brief Toggle the SoftReset on the Router
     *
     *  Sends Soft_RST (line 11 in excel)
     *
     * \param reset_hold time in seconds to hold the reset signal
     * \param reset_sleep time in seconds to sleep at the end of the reset
     */
    void toggleSoftReset(const OpcClientPtr& opcConnection,
                         const std::chrono::seconds& reset_hold = nsw::router::RESET_HOLD,
                         const std::chrono::seconds& reset_sleep = nsw::router::RESET_SLEEP) const;

    /**
     * \brief Perform a soft reset of the Router
     *
     *  Sets Router control mode to SCA mode (line 17 in excel)
     *  Sends Soft_RST (line 11 in excel)
     *
     * \param reset_hold time in seconds to hold the reset signal
     * \param reset_sleep time in seconds to sleep at the end of the reset
     */
    void sendSoftReset(const OpcClientPtr& opcConnection,
                       const std::chrono::seconds& reset_hold = nsw::router::RESET_HOLD,
                       const std::chrono::seconds& reset_sleep = nsw::router::RESET_SLEEP) const;

    /**
     * \brief Check the GPIOs of the Router
     *
     * Read SCA IO status back: Line 6 & 8 in excel
     * (only need to match with star mark bits)
     */
    void checkGPIO(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Set the SCA ID of the Router
     */
    void setSCAID(const OpcClientPtr& opcConnection) const;

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
