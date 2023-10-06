#ifndef NSWCONFIGURATION_HW_ART_H
#define NSWCONFIGURATION_HW_ART_H

#include <functional>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/ARTConfig.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

namespace nsw {
  class OpcManager;
  class ADDCConfig;
}

namespace nsw::hw {

  /**
   * \brief Class representing an ART
   *
   * Provides methods to read/write individual ART registers, as well
   * as to write a complete configuration and read back all the
   * registers contained in the configuration.
   *
   * Strictly speaking, it contains the configuration of gbtx on the ADDC.
   *
   * Register mapping: http://cern.ch/go/8jKB
   * Documentation: TODO
   */
  class ART : public ScaAddressBase, public OpcConnectionBase
  {
  private:
    static constexpr std::size_t   NUM_ART_REGISTERS            = 15;
    static constexpr std::size_t   NUM_GBTX_REGISTERS           = 369;
    static constexpr std::uint32_t REG_FLAG_MASK                = 3;
    static constexpr std::uint8_t  FLAG_MASK_DEFAULT            = 0x0E;
    static constexpr std::uint8_t  FLAG_MASK_FAILSAFE           = 0x06;
    static constexpr std::uint32_t REG_FLAG_PATTERN             = 4;
    static constexpr std::uint8_t  FLAG_PATTERN_DEFAULT         = 0x3F;
    static constexpr std::uint8_t  FLAG_PATTERN_FAILSAFE        = 0x27;

    static constexpr std::uint32_t REG_GBTX_TRAIN_MODE          = 62;
    static constexpr std::uint8_t  GBTX_TRAIN_MODE_TRAINING     = 0x15;
    static constexpr std::uint32_t REG_GBTX_TTC_COARSE_DELAY    = 11;

    static constexpr std::array<uint8_t, 4> m_ARTCoreregisters{9, 10, 11, 12};

    static constexpr std::array<uint8_t, NUM_ART_REGISTERS> m_ARTregisters{
       21,   22,   23,   24,   25,   26,   27,
       28,   29,   30,   31,   32,   33,   34,   2};

    static constexpr std::array<uint8_t, NUM_ART_REGISTERS> m_ARTregistervalues{
       0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00,
       0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00, 0x80};

    static constexpr std::array<uint8_t, 21> m_GBTx_eport_registers{
       78, 79, 80, 102, 103, 104, 126, 127, 128, 150, 151, 152,
       174, 175, 176, 198, 199, 200, 222, 223, 224};

    static constexpr std::array<uint8_t, NUM_GBTX_REGISTERS> m_GBTx_ConfigurationData{
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
       0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x01, 0x00, 0x01};

    ARTConfig m_config;         //!< ARTConfig object associated with this ART
    std::size_t m_iArt{};       //!< index of this ART, either 0 or 1, set to 2 to be unphysical. It must be set in constructor

  public:
    /**
     * \brief Constructor from an \ref ADDCConfig object
     * 
     * @param manager Link to the OPC Manager
     * @param config Configuration of the ADDC
     * @param numArt Number of the ART on the ADDC
     */
    ART(nsw::OpcManager& manager, const nsw::ADDCConfig& config, const std::size_t numArt);

    /**
     * \brief Read the full ART address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]] std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration() const;

    [[nodiscard]] std::string getName()     const { return m_config.getName(); }
    [[nodiscard]] std::string getNameCore() const { return m_config.getNameCore(); }
    [[nodiscard]] std::string getNamePs()   const { return m_config.getNamePs(); }
    [[nodiscard]] std::string getNameGbtx() const { return m_config.getNameGbtx(); }

    // getScaAddress is returning ADDC's SCA address
    [[nodiscard]] std::string getAddressCore() const { return getScaAddress() + "." + getNameCore(); }
    [[nodiscard]] std::string getAddressPs()   const { return getScaAddress() + "." + getNamePs(); }
    [[nodiscard]] std::string getAddressGbtx() const { return getScaAddress() + "." + getNameGbtx(); }

    [[nodiscard]] bool SkipConfigure() const { return m_config.SkipConfigure(); }
    [[nodiscard]] bool MustConfigure() const { return m_config.MustConfigure(); }

    /**
     * \brief Write the full ART configuration
     */
    void writeConfiguration() const;

    /**
     * \brief belows are components used in writeConfiguration
     */
    void setArtGpio(const std::string& gpio, const bool state) const ;
    void toggleArtGpio(const std::string& gpio) const;

    void initART() const;
    void resetGBTx() const;
    void configGBTx() const;
    void resetART() const;
    void configART() const;
    void maskART() const;
    void trainGBTx() const;
    void failsafeMode() const;
    void unmaskART() const;
    void adjustPhaseART() const;

    /**
     * \brief Write a value to a GBTx register (through I2C) 
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeGBTXRegister(std::uint32_t regAddress,
                           std::uint8_t value) const;

    /**
     * \brief Write a value to an ART Core register 
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeARTCoreRegister(std::uint8_t regAddress,
                              std::uint8_t value) const;

    /**
     * \brief Write a value to an ART PhaseShifter(PS) register 
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeARTPsRegister(std::uint8_t regAddress,
                            std::uint8_t value) const;

    /**
     * \brief Write a value to an ART register 
     *
     * \param nodeName points to the slave address 0 of the bus, not including ADDC SCA 
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeARTRegister(const std::string& nodeName,
                          std::uint8_t regAddress,
                          std::uint8_t value) const;

    /**
     * \brief Read an individual ART register by its address
     *
     * \param nodeName points to the slave address 0 of the bus, not including ADDC SCA 
     * \param regAddress is the address of the ART register
     * \param addressSize is the size of the address field, in byte
     * \param regAddress is the number of bytes to do in one OPC read operation(passed as argument)
     */
    [[nodiscard]] std::vector<std::uint8_t> readRegister(const std::string& nodeName,
                                                         std::uint8_t regAddress,
                                                         std::size_t addressSize,
                                                         std::size_t numberOfBytes 
      ) const;

    /**
     * \brief Get the \ref ARTConfig object associated with this ART object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] ARTConfig& getConfig() { return m_config; }
    [[nodiscard]] const ARTConfig& getConfig() const { return m_config; }  //!< \overload

    [[nodiscard]] std::size_t index() const { return m_iArt; }

  };
}  // namespace nsw::hw

#endif
