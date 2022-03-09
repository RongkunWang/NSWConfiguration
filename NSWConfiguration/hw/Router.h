#ifndef NSWCONFIGURATION_HW_ROUTER_H
#define NSWCONFIGURATION_HW_ROUTER_H

#include "NSWConfiguration/RouterConfig.h"
#include "NSWConfiguration/Constants.h"
#include <chrono>

ERS_DECLARE_ISSUE(nsw,
                  RouterHWIssue,
                  message,
                  ((std::string)message)
                  )

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
     * \brief Name of router object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Send a value to a particular Router GPIO
     */
    void sendGPIO(const std::string& name, bool val) const;

    /**
     * \brief Send a value to a particular Router GPIO, and read back
     */
    void sendAndReadbackGPIO(const std::string& name, bool val) const;

    /**
     * \brief Read a value from a particular Router GPIO
     */
    bool readGPIO(const std::string& name) const;

    /**
     * \brief Read all Router GPIOs
     */
    std::map<std::string, bool> readConfiguration() const;

    /**
     * \brief Write the full Router configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Send a soft reset to the Router, and check GPIOs
     */
    void writeSoftResetAndCheckGPIO() const;

    /**
     * \brief Send a soft reset to the Router
     */
    void writeSoftReset(const std::chrono::seconds reset_hold  = nsw::router::RESET_HOLD,
                        const std::chrono::seconds reset_sleep = nsw::router::RESET_SLEEP) const;

    /**
     * \brief Check the status of Router GPIOs a few times
     */
    bool checkGPIOsAFewTimes() const;

    /**
     * \brief Check the status of Router GPIOs
     */
    bool checkGPIOs() const;

    /**
     * \brief Configure the Router GPIOs which control the SCA ID
     */
    void writeSetSCAID() const;

    [[nodiscard]]
    std::string Sector() const
    { return m_config.getConfig().get<std::string>("Sector"); }

    /**
     * \brief Convert name and sector to unique 8-bit ID
     *
     * 4 bits: Sector (0x0 - 0xf)
     * 3 bits: Layer (0x0 - 0x7)
     * 1 bits: Endcap (A=0, C=1)
     * NB: "Sector" = Sector-1,
     *     since ATLAS sectors 01-16
     *
     * e.g. Router A14 L5
     * 4 bits: 14-1 = 13 = 0b1101
     * 3 bits: L5   =  5 = 0b101
     * 1 bits: A    =  0 = 0b0
     * (13 << 4) + (5 << 1) + 1 = 218
     */
    [[nodiscard]]
    std::uint8_t id() const;

    /**
     * \brief Convert sector to 1-bit endcap ID
     *
     * XYY -> X
     * A12 -> A, e.g.
     */
    [[nodiscard]]
    std::uint8_t idEndcap() const;

    /**
     * \brief Convert sector to 4-bit sector ID
     *
     * XYY -> YY
     * A12 -> 12, e.g.
     */
    [[nodiscard]]
    std::uint8_t idSector() const;

    /**
     * \brief Convert layer to 3-bit layer ID
     *
     * Router_LZ -> Z
     */
    [[nodiscard]]
    std::uint8_t idLayer() const;

    /**
     * \brief Check the format of the name provided
     */
    void idCheck() const;

    /**
     * \brief Throw an exception if the format of the name is unrecognized
     */
    void idCrash() const;

    /**
     * \brief Get the \ref RouterConfig object associated with this Router object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]]
    RouterConfig& getConfig() { return m_config; }
    [[nodiscard]]
    const RouterConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    RouterConfig m_config;
    std::string m_opcserverIp;  //!< address and port of the Opc Server
    std::string m_scaAddress;   //!< SCA address of Router item in the Opc address space
    std::string m_name;         //!< Name composed of OPC and SCA addresses

    static constexpr std::string_view
      m_old_convention{"Router_LZ"}; //!< the old convention for Router names

    static constexpr std::string_view
      m_convention{"sTGC-A/V0/SCA/Router/S2/L0"}; //!< the latest convention for Router names

    static constexpr std::string_view
      m_name_error{"This Router doesnt follow the naming convention"}; //!< A helpful error message

    static constexpr size_t m_num_gpios = 32;
    static constexpr std::array<std::string_view, m_num_gpios> m_ordered_gpios = {
      "fpgaConfigOK",
      "routerId0",
      "routerId1",
      "routerId2",
      "routerId3",
      "routerId4",
      "routerId5",
      "routerId6",
      "routerId7",
      "mmcmBotLock",
      "designNum0",
      "designNum1",
      "designNum2",
      "fpgaInit",
      "mmcmReset",
      "softReset",
      "rxClkReady",
      "txClkReady",
      "cpllTopLock",
      "cpllBotLock",
      "mmcmTopLock",
      "semFatalError",
      "semHeartBeat",
      "debugEnable",
      "notConnected",
      "mtxRst",
      "masterChannel0",
      "masterChannel1",
      "masterChannel2",
      "ctrlMod0",
      "ctrlMod1",
      "multibootTrigger",
    };

    static constexpr size_t m_num_checks = 10;
    static constexpr
      std::array< std::pair<std::string_view, bool>, m_num_checks > m_gpio_checks = {{
        {"fpgaConfigOK",   true},
        {"mmcmBotLock",    true},
        {"fpgaInit",       true},
        {"rxClkReady",     true},
        {"txClkReady",     true},
        {"cpllTopLock",    true},
        {"cpllBotLock",    true},
        {"mmcmTopLock",    true},
        {"semFatalError",  false},
        {"masterChannel0", true},
    }};

  };
}  // namespace nsw::hw

#endif
