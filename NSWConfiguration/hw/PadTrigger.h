#ifndef NSWCONFIGURATION_HW_PADTRIGGER_H
#define NSWCONFIGURATION_HW_PADTRIGGER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/PadTriggerSCAConfig.h"

#include <ers/Issue.h>

ERS_DECLARE_ISSUE(nsw,
                  PadTriggerConfigError,
                  message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  PadTriggerReadbackMismatch,
                  message,
                  ((const char *)message)
                  )

namespace nsw::hw {
  /**
   * \brief Class representing a Pad Trigger
   *
   * Provides methods to read/write individual PadTrigger registers,
   * as well as to write a complete configuration and read back all
   * the registers contained in the configuration.
   *
   * Register mapping: TODO
   * Documentation: TODO
   */
  class PadTrigger
  {
  public:
    /**
     * \brief Constrctor from a \ref PadTriggerConfig object
     */
    explicit PadTrigger(const PadTriggerSCAConfig& config);

    /**
     * \brief Name of pad trigger object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string name() const {return m_name;};

    /**
     * \brief Read the full PadTrigger address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::uint8_t, std::uint32_t> readConfiguration() const;

    /**
     * \brief Write the full PadTrigger configuration
     */
    void writeConfiguration() const;

    /**
     * \brief Write the PadTrigger repeaters configuration
     */
    void writeRepeatersConfiguration() const;

    /**
     * \brief Write the PadTrigger VTTx configuration
     */
    void writeVTTxConfiguration() const;

    /**
     * \brief Write the PadTrigger FPGA configuration
     */
    void writeFPGAConfiguration() const;

    /**
     * \brief Write a common delay to all PFEB inputs
     */
    void writePFEBCommonDelay(std::uint32_t value) const;

    /**
     * \brief Write a value to a pad trigger GPIO
     *
     * \param name is the name of the GPIO
     * \param value is the value to be written
     */
    void writeGPIO(const std::string& name, bool value) const;

    /**
     * \brief Write a value to a pad trigger FPGA register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeFPGARegister(std::uint8_t regAddress,
                           std::uint32_t value) const;

    /**
     * \brief Write a value to a pad trigger VTTx register address
     *
     * \param vttx is the number of the VTTx
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeVTTxRegister(std::uint8_t vttx,
                           std::uint8_t regAddress,
                           std::uint8_t value) const;

    /**
     * \brief Write a value to a pad trigger repeater register address
     *
     * \param repeater is the number of the repeater
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeRepeaterRegister(std::uint8_t repeater,
                               std::uint8_t regAddress,
                               std::uint8_t value) const;


    /**
     * \brief Write a sub-register of the control register
     *
     * \param subreg is the name of the sub-register
     * \param subval is the value to write
     */
    void writeControlSubRegister(const std::string& subreg, std::uint32_t subval) const;

    /**
     * \brief Enable the pad trigger readout
     */
    void writeReadoutEnable() const
    { writeControlSubRegister("conf_ro_en", std::uint32_t{true}); };

    /**
     * \brief Disable the pad trigger readout
     */
    void writeReadoutDisable() const
    { writeControlSubRegister("conf_ro_en", std::uint32_t{false}); };

    /**
     * \brief Enable the pad trigger idle state
     */
    void writeStartIdleStateEnable() const
    { writeControlSubRegister("conf_startIdleState", std::uint32_t{true}); };

    /**
     * \brief Disable the pad trigger idle state
     */
    void writeStartIdleStateDisable() const
    { writeControlSubRegister("conf_startIdleState", std::uint32_t{false}); };

    /**
     * \brief Write readout BC offset (latency)
     *
     * \param val is the offset
     */
    void writeReadoutBCOffset(const std::uint32_t val) const
    { writeControlSubRegister("conf_ro_bc_offset", val); };

    /**
     * \brief Read an individual pad trigger FPGA register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readFPGARegister(std::uint8_t regAddress) const;

    /**
     * \brief Write a value to a pad trigger GPIO
     *
     * \param name is the name of the GPIO
     * \param value is the value to be written
     */
    [[nodiscard]]
    bool readGPIO(const std::string& name) const;

    /**
     * \brief Read an individual pad trigger repeater register by its address
     *
     * \param repeater is the number of the repeater
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint8_t readRepeaterRegister(std::uint8_t repeater,
                                      std::uint8_t regAddress) const;

    /**
     * \brief Read an individual pad trigger VTTx register by its address
     *
     * \param VTTx is the number of the VTTx
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint8_t readVTTxRegister(std::uint8_t vttx,
                                  std::uint8_t regAddress) const;

    /**
     * \brief Read and decode the PFEB BCID status registers
     */
    [[nodiscard]]
    std::vector<std::uint32_t> readPFEBBCIDs() const;

    /**
     * \brief Get the \ref PadTriggerConfig object associated with this PadTrigger object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] PadTriggerSCAConfig& getConfig() { return m_config; }
    [[nodiscard]] const PadTriggerSCAConfig& getConfig() const { return m_config; }  //!< overload

    /**
     * \brief Get the register address from the register name
     *
     * e.g., "00B_register_description" returns 0x00B
     */
    [[nodiscard]]
    std::uint8_t addressFromRegisterName(const std::string& name) const;


  private:
    PadTriggerSCAConfig m_config;  //!< PadTriggerConfig object associated with this PadTrigger
    std::string m_opcserverIp;     //!< Address and port of OPC Server
    std::string m_scaAddress;      //!< SCA address of PadTrigger item in the OPC address space
    std::string m_scaAddressFPGA;  //!< SCA address of PadTrigger FPGA i2c
    std::string m_name;            //!< Name composed of OPC and SCA addresses

    static constexpr std::array<std::uint8_t, 2> m_vttxs{1, 2};
    static constexpr std::array<std::uint8_t, 1> m_vttx_datas{0xC7};
    static constexpr std::uint8_t m_vttx_addr{0x00};

    static constexpr size_t NUM_CMD = 72;
    static constexpr std::array< std::tuple< std::uint8_t, std::uint8_t, std::uint8_t >, NUM_CMD>
      m_repeaterSequenceOfCommands = {{
        // clang-format off
        // enable smbus registers on all repeater chips
        {0x01, 0x07, 0x01}, {0x01, 0x07, 0x11}, {0x01, 0x07, 0x21}, {0x01, 0x07, 0x31},
        {0x02, 0x07, 0x01}, {0x02, 0x07, 0x11}, {0x02, 0x07, 0x21}, {0x02, 0x07, 0x31},
        {0x03, 0x07, 0x01}, {0x03, 0x07, 0x11}, {0x03, 0x07, 0x21}, {0x03, 0x07, 0x31},
        {0x04, 0x07, 0x01}, {0x04, 0x07, 0x11}, {0x04, 0x07, 0x21}, {0x04, 0x07, 0x31},
        {0x05, 0x07, 0x01}, {0x05, 0x07, 0x11}, {0x05, 0x07, 0x21}, {0x05, 0x07, 0x31},
        {0x06, 0x07, 0x01}, {0x06, 0x07, 0x11}, {0x06, 0x07, 0x21}, {0x06, 0x07, 0x31},
        // set repeater chips equaliser settings
        {0x01, 0x14, 0x03}, {0x01, 0x16, 0x03}, {0x01, 0x18, 0x03}, {0x01, 0x1A, 0x03},
        {0x02, 0x14, 0x03}, {0x02, 0x16, 0x03}, {0x02, 0x18, 0x03}, {0x02, 0x1A, 0x03},
        {0x03, 0x14, 0x03}, {0x03, 0x16, 0x03}, {0x03, 0x18, 0x03}, {0x03, 0x1A, 0x03},
        {0x04, 0x14, 0x03}, {0x04, 0x16, 0x03}, {0x04, 0x18, 0x03}, {0x04, 0x1A, 0x03},
        {0x05, 0x14, 0x03}, {0x05, 0x16, 0x03}, {0x05, 0x18, 0x03}, {0x05, 0x1A, 0x03},
        {0x06, 0x14, 0x03}, {0x06, 0x16, 0x03}, {0x06, 0x18, 0x03}, {0x06, 0x1A, 0x03},
        // disable smbus registers on all repeater chips
        // not really necessary but it’s a protection against occasional commands sent by mistake
        {0x01, 0x07, 0x00}, {0x01, 0x07, 0x10}, {0x01, 0x07, 0x20}, {0x01, 0x07, 0x30},
        {0x02, 0x07, 0x00}, {0x02, 0x07, 0x10}, {0x02, 0x07, 0x20}, {0x02, 0x07, 0x30},
        {0x03, 0x07, 0x00}, {0x03, 0x07, 0x10}, {0x03, 0x07, 0x20}, {0x03, 0x07, 0x30},
        {0x04, 0x07, 0x00}, {0x04, 0x07, 0x10}, {0x04, 0x07, 0x20}, {0x04, 0x07, 0x30},
        {0x05, 0x07, 0x00}, {0x05, 0x07, 0x10}, {0x05, 0x07, 0x20}, {0x05, 0x07, 0x30},
        {0x06, 0x07, 0x00}, {0x06, 0x07, 0x10}, {0x06, 0x07, 0x20}, {0x06, 0x07, 0x30},
        // clang-format on
    }};

  };
}  // namespace nsw::hw

#endif
