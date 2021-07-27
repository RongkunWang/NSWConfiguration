#ifndef NSWCONFIGURATION_HW_ART_H
#define NSWCONFIGURATION_HW_ART_H

#include "NSWConfiguration/ADDCConfig.h"
#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/OpcClient.h"

namespace nsw::hw {
  /**
   * \brief Class representing an ART ASIC
   *
   * Provides methods to read/write individual ART registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   */
  class ART
  {
  public:
    /**
     * \brief Constructor from a \ref ARTConfig object
     */
    ART(const nsw::ADDCConfig& config, std::size_t numArt);

    /**
     * \brief Read the full ART address space
     *
     * \returns a map of address to register value
     */
    std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration();

    /**
     * \brief Write the full ART configuration
     */
    void writeConfiguration();

    /**
     * \brief Write a value to a ART register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRegister(std::uint8_t registerId, std::uint32_t value);

    /**
     * \brief Read an individual ART register by its address
     */
    std::vector<std::uint8_t> readRegister([[maybe_unused]] std::uint8_t registerId);

    /**
     * \brief Reset the ART (part of writeConfiguration)
     */
    void resetArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Reset the GTBx (part of writeConfiguration)
     */
    void resetGbtx(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Configure the GTBx (part of writeConfiguration)
     */
    void configureGbtx(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Reset the ART part 2 (part of writeConfiguration)
     */
    void resetArt2(const OpcClientPtr& opcConnection) const;  // FIXME name

    /**
     * \brief Common configuration of the ART (part of writeConfiguration)
     */
    void configureCommonArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Mask the ART (part of writeConfiguration)
     */
    void maskArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Train the GBTx (part of writeConfiguration)
     */
    void trainGbtx(const OpcClientPtr& opcConnection) const;

    /**
     * \brief ART failsafe mode (part of writeConfiguration)
     */
    void failsafeModeArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Unmask the ART (part of writeConfiguration)
     */
    void unmaskArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Configure the ART BCRCLK phase (part of writeConfiguration)
     */
    void configureClockArt(const OpcClientPtr& opcConnection) const;

    /**
     * \brief Get the \ref ARTConfig object associated with this ART object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] ARTConfig& getConfig() { return m_config; }
    [[nodiscard]] const ARTConfig& getConfig() const { return m_config; }  //!< \overload

  private:
    ARTConfig m_config;         //!< ARTConfig object associated with this ART
    std::string m_opcserverIp;  //!< address and port of Opc Server
    std::string m_scaAddress;   //!< SCA address of ART item in Opc address space
    std::size_t m_numArt;       //!< Number of the ART

    static constexpr std::array<uint8_t, 4> m_artCoreRegisters{9, 10, 11, 12};

    static constexpr std::array<uint8_t, nsw::addc::NUM_ART_REGISTERS> m_artRegisters{
      // clang-format off
      21,   22,   23,   24,   25,   26,   27,
      28,   29,   30,   31,   32,   33,   34,   2
      // clang-format on
    };

    static constexpr std::array<uint8_t, nsw::addc::NUM_ART_REGISTERS> m_artRegisterValues{
      // clang-format off
      0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00,
      0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00, 0x80
      // clang-format off
    };

    static constexpr std::array<uint8_t, 21> m_gbtxEportRegisters{
      // clang-format off
      78, 79, 80, 102, 103, 104, 126, 127, 128, 150, 151, 152,
      174, 175, 176, 198, 199, 200, 222, 223, 224
      // clang-format off
    };

    static constexpr std::array<uint8_t, nsw::addc::NUM_GBTX_REGISTERS> m_gbtxConfigurationData{
      // clang-format off
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x23, 0x03, 0x33, 0x03, 0x03, 0x00, 0x00, 0x00,
      0x1f, 0x03, 0x7f, 0x28, 0x00, 0x15, 0x15, 0x15, 0x66, 0x00, 0x0d, 0x42,
      0x00, 0x0f, 0x04, 0x08, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x15, 0x15,
      0x15, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
      0x00, 0x00, 0x00, 0x3f, 0xdd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x3f, 0x3f, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
      0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x40, 0x40, 0x40, 0x2a,
      0x2a, 0x2a, 0x00, 0x00, 0xff, 0xff, 0xff, 0x40, 0x40, 0x40, 0x2a, 0x2a,
      0x2a, 0x4e, 0x4e, 0x4e, 0xaa, 0x0a, 0x07, 0x00, 0x11, 0x11, 0x11, 0x11,
      0x11, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
      0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x01, 0x00, 0x01
      // clang-format off
    };
  };

  constexpr bool RESET_INACTIVE_HIGH = true;
  constexpr bool RESET_ACTIVE_LOW = false;
}  // namespace nsw::hw

#endif
