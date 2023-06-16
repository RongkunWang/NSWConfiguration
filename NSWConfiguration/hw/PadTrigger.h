#ifndef NSWCONFIGURATION_HW_PADTRIGGER_H
#define NSWCONFIGURATION_HW_PADTRIGGER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <ers/Issue.h>

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

using BcidVector  = std::vector<std::uint32_t>;
using DelayVector = std::vector<std::uint32_t>;
using ValueVector = std::vector<std::uint32_t>;
using StrobeVector = std::vector<std::uint32_t>;

ERS_DECLARE_ISSUE(nsw,
                  PadTriggerConfusion,
                  message,
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  PadTriggerConfigError,
                  message,
                  ((std::string)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  PadTriggerReadbackMismatch,
                  fmt::format("Found mismatch in {} readback", target),
                  ((std::string)target)
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
  class PadTrigger : public ScaAddressBase, public OpcConnectionBase
  {
  public:
    /**
     * \brief Constrctor from a \ref PadTriggerConfig object
     */
    PadTrigger(OpcManager& manager,const boost::property_tree::ptree& config);

    /**
     * \brief Name of pad trigger object
     *
     * \returns a name of the object
     */
    [[nodiscard]]
    std::string getName() const { return m_name; };

    /**
     * \brief Read the full PadTrigger address space
     *
     * \returns a map of address to register value
     */
    [[nodiscard]]
    std::map<std::uint8_t, std::uint32_t> readConfiguration() const;

    /**
     * \brief Read the full PadTrigger address space, including sub-registers
     *
     * \returns a map of address to sub-register value
     */
    [[nodiscard]]
    std::map<std::string, std::uint32_t> readConfigurationSubRegisters() const;

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
     * \brief Write the bitfile to the PadTrigger FPGA
     */
    void writeJTAGBitfileConfiguration() const;

    /**
     * \brief Write the PadTrigger FPGA configuration
     * \throws exception when writing to register 0x0 fails,
     *   since this register is present in all firmware versions.
     *   Does not throw exception if any other register write fails.
     */
    void writeFPGAConfiguration() const;

    /**
     * \brief Write a delay to all PFEB inputs
     */
    void writePFEBDelay(const DelayVector& values) const;

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
     * \brief Write and readback a value to a pad trigger FPGA register address
     *
     * \param regAddress is the address of the register
     * \param value is the value to be written
     */
    void writeAndReadbackFPGARegister(std::uint8_t regAddress,
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
     * \brief Write a sub-register of a register
     *
     * \param rname is the name of the register
     * \param subreg is the name of the sub-register
     * \param subval is the value to write
     */
    void writeSubRegister(const std::string& rname,
                          const std::string& subreg,
                          std::uint32_t subval,
                          const bool quiet = false) const;

    /**
     * \brief Write a sub-register of the control register
     *
     * \param subreg is the name of the sub-register
     * \param subval is the value to write
     */
    void writeControlSubRegister(const std::string& subreg, std::uint32_t subval) const
    { writeSubRegister("000_control_reg", subreg, subval); }

    /**
     * \brief Write a sub-register of the reset register
     *
     * \param subreg is the name of the sub-register
     * \param subval is the value to write
     */
    void writeControl3SubRegister(const std::string& subreg, std::uint32_t subval) const
    { writeSubRegister("00E_control_reg3", subreg, subval); }

    /**
     * \brief Toggle (write) the OCR enable from enabled to disabled
     */
    void toggleOcrEnable() const;

    /**
     * \brief Toggle (write) the GT reset from enabled to disabled
     */
    void toggleGtReset() const;

    /**
     * \brief Toggle (write) the GT RX LOL reset from enabled to disabled
     */
    void toggleGtRxLolReset() const;

    /**
     * \brief Toggle (write) the idle state from 1 to 0
     */
    void toggleIdleState() const;

    /**
     * \brief Toggle (write) the BCID error reset from enabled to disabled
     */
    void toggleBcidErrorReset() const;

    /**
     * \brief Toggle the PFEB BCID error reset (readout) from enabled to disabled
     */
    void togglePFEBBcidResetReadout() const;

    /**
     * \brief Toggle the PFEB BCID error reset (readout) from enabled to disabled
     */
    void togglePFEBBcidResetTrigger() const;

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
     * \brief Temporarily enable the pad trigger readout
     */
    void writeReadoutEnableTemporarily(std::chrono::duration<float> dur) const
    { writeReadoutEnable();
      nsw::snooze(dur);
      writeReadoutDisable();
    };

    /**
     * \brief Enable the pad trigger OCR enable
     */
    void writeOcrEnEnable() const
    { writeControlSubRegister("conf_ocr_en", std::uint32_t{true}); };

    /**
     * \brief Disable the pad trigger OCR disable
     */
    void writeOcrEnDisable() const
    { writeControlSubRegister("conf_ocr_en", std::uint32_t{false}); };

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
     * \brief Enable the pad trigger internal BCID error checker reset
     */
    void writeBcidResetEnable() const
    { writeControl3SubRegister("bcid_error_rst", 0b11); };

    /**
     * \brief Disable the pad trigger internal BCID error checker reset
     */
    void writeBcidResetDisable() const
    { writeControl3SubRegister("bcid_error_rst", 0b00); };

    /**
     * \brief Enable the pad trigger PFEB BCID error checker reset (readout)
     */
    void writePFEBBcidResetReadoutEnable() const
    { writeControl3SubRegister("ro_bcid_error_rst", std::uint32_t{true}); };

    /**
     * \brief Disable the pad trigger PFEB BCID error checker reset (readout)
     */
    void writePFEBBcidResetReadoutDisable() const
    { writeControl3SubRegister("ro_bcid_error_rst", std::uint32_t{false}); };

    /**
     * \brief Enable the pad trigger PFEB BCID error checker reset (trigger)
     */
    void writePFEBBcidResetTriggerEnable() const
    { writeControl3SubRegister("trig_bcid_error_rst", std::uint32_t{true}); };

    /**
     * \brief Disable the pad trigger PFEB BCID error checker reset (trigger)
     */
    void writePFEBBcidResetTriggerDisable() const
    { writeControl3SubRegister("trig_bcid_error_rst", std::uint32_t{false}); };

    /**
     * \brief Enable the pad trigger GT reset
     */
    void writeGtSoftResetEnable() const
    { writeControl3SubRegister("gt_soft_reset", nsw::padtrigger::GT_RESET_ENABLE); };

    /**
     * \brief Disable the pad trigger GT reset
     */
    void writeGtSoftResetDisable() const
    { writeControl3SubRegister("gt_soft_reset", nsw::padtrigger::GT_RESET_DISABLE); };

    /**
     * \brief Enable the pad trigger GT LOL reset
     */
    void writeGtRxLolResetEnable() const
    { writeSubRegister("010_gt_rx_reset", "gt_rx_lol_reset", std::uint32_t{true}); }

    /**
     * \brief Disable the pad trigger GT LOL reset
     */
    void writeGtRxLolResetDisable() const
    { writeSubRegister("010_gt_rx_reset", "gt_rx_lol_reset", std::uint32_t{false}); }

    /**
     * \brief Write readout BC offset (latency)
     *
     * \param val is the offset
     */
    void writeReadoutBCOffset(const std::uint32_t val) const
    { writeControlSubRegister("conf_ro_bc_offset", val); };

    /**
     * \brief Read the GPIO which indicates if the FPGA has firmware uploaded
     */
    [[nodiscard]]
    bool readFPGADone() const;

    /**
     * \brief Read an individual pad trigger FPGA register by its address
     *
     * \param regAddress is the address of the register
     */
    [[nodiscard]]
    std::uint32_t readFPGARegister(std::uint8_t regAddress) const;

    /**
     * \brief Read a sub-register of a register
     *
     * \param rname is the name of the register
     * \param subreg is the name of the sub-register
     */
    std::uint32_t readSubRegister(const std::string& rname,
                                  const std::string& subreg) const;

    /**
     * \brief Get the sub-register of a register from the value of the register
     *
     * \param rname is the name of the register
     * \param subreg is the name of the sub-register
     * \param value is the value of the register
     */
    std::uint32_t getSubRegisterFromRegister(const std::string& rname,
                                             const std::string& subreg,
                                             const std::uint32_t value) const;

    /**
     * \brief Write a value to a pad trigger GPIO
     *
     * \param name is the name of the GPIO
     * \param value is the value to be written
     */
    [[nodiscard]]
    bool readGPIO(const std::string& name) const;

    /**
     * \brief Write a value to a pad trigger GPIO
     *
     * \param name is the name of the GPIO
     * \param value is the value to be written
     */
    [[nodiscard]]
    bool readGPIO(const std::string_view& name) const { return readGPIO(std::string(name)); }

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
     * \brief Read the PFEB BCIDs at each 320 strobe select, and adjust accordingly
     */
    void select320Strobe() const;

    /**
     * \brief Read and decode the PFEB BCID status registers multiple times,
     *        and calculate median, for each possible 320 strobe select.
     *        Element (i)(j) is the median BCID of PFEB i for select setting j.
     *
     * \param nread number of times to read and decode the BCIDs
     */
    [[nodiscard]]
    std::vector<BcidVector> readMedianPFEBBCIDAtEachStrobe
      (std::size_t nread = nsw::padtrigger::NUM_DESKEW_READS) const;

    /**
     * \brief Write messages regarding the measured 320 strobe scan
     *
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning 320 strobes
     */
    void describeStrobe(const std::vector<BcidVector>& bcidPerPfebPerStrobe) const;

    /**
     * \brief Return the target strobe, given a list of BCID observed for each PFEB and strobe.
     *        The target strobe should be the smallest strobe for which all PFEBs have the same BCID.
     *
     * \param bcidsPerPfebPerStrobe For each PFEB, the BCIDs observed when scanning the strobe
     */
    [[nodiscard]]
    std::uint32_t getTargetStrobe(const std::vector<BcidVector>& bcidPerPfebPerStrobe) const;

    /**
     * \brief Read the PFEB BCIDs, and adjust delays to deskew them
     */
    void deskewPFEBs() const;

    /**
     * \brief Write messages regarding the measured skew
     *
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning delays
     */
    void describeSkew(const std::vector<BcidVector>& bcidPerPfebPerDelay) const;

    /**
     * \brief Read the GT RX LOL word, and toggle sticky reset afterward
     */
    [[nodiscard]]
    std::uint32_t readGtRxLol() const;

    /**
     * \brief Read the trigger rate of a particular BCID
     *
     * \param bcid is the BCID
     */
    [[nodiscard]]
    std::uint32_t readBcidTriggerRate(const std::uint32_t bcid) const;

    /**
     * \brief Read the rate of an input PFEB from a status register
     *
     * \param pfeb is the pfeb number
     */
    [[nodiscard]]
    std::uint32_t readPFEBRate(const std::uint32_t pfeb, const bool quiet) const;

    /**
     * \brief Read the rates of all input PFEBs
     */
    [[nodiscard]]
    std::vector<std::uint32_t> readPFEBRates() const;

    /**
     * \brief Read the PFEB BCID error word (readout)
     */
    [[nodiscard]]
    std::uint32_t readPFEBBcidErrorReadout() const;

    /**
     * \brief Read the PFEB BCID error word (trigger)
     */
    [[nodiscard]]
    std::uint32_t readPFEBBcidErrorTrigger() const;

    /**
     * \brief Read and decode the PFEB delay registers.
     *        Element i is the BCID of PFEB i.
     */
    [[nodiscard]]
    DelayVector readPFEBDelays() const;

    /**
     * \brief Read and decode the PFEB BCID status registers.
     *        Element i is the BCID of PFEB i.
     */
    [[nodiscard]]
    BcidVector readPFEBBCIDs() const;

    /**
     * \brief Read and decode the PFEB BCID status registers multiple times
     *        Element i is a vector of the BCIDs of PFEB i.
     *
     * \param nread number of times to read and decode the BCIDs
     */
    [[nodiscard]]
    std::vector<BcidVector> readPFEBBCIDs(std::size_t nread) const;

    /**
     * \brief Read and decode the PFEB BCID status registers multiple times, and calculate median
     *        Element i is the median BCID of PFEB i.
     *
     * \param nread number of times to read and decode the BCIDs
     */
    [[nodiscard]]
    BcidVector readMedianPFEBBCIDs(std::size_t nread) const;

    /**
     * \brief Read and decode the PFEB BCID status registers multiple times,
     *        and calculate median, for each possible PFEB input delay.
     *        Element (i)(j) is the median BCID of PFEB i for delay setting j.
     *
     * \param nread number of times to read and decode the BCIDs
     */
    [[nodiscard]]
    std::vector<BcidVector> readMedianPFEBBCIDAtEachDelay(std::size_t nread) const;

    /**
     * \brief Rotate a vector of PFEB BCIDs by 180 degrees.
     *
     * \param bcids a vector of BCIDs
     */
    [[nodiscard]]
    BcidVector rotatePFEBBCIDs(BcidVector bcids) const;

    /**
     * \brief Check if PFEB BCIDs over the range of input delays seem reasonable or not
     *
     * \param bcids BCIDs of a particular PFEB
     */
    [[nodiscard]]
    bool checkPFEBBCIDs(const BcidVector& bcids) const;

    /**
     * \brief Return a list of BCIDs whose full range is observed when scanning delays.
     *        e.g., if the BCIDs observed per delay are ccddddddeeeeeeff,
     *              then 0xe and 0xd are viable BCIDs.
     *
     * \param bcidsPerDelay BCIDs observed when scanning delays
     */
    [[nodiscard]]
    BcidVector getViableBcids(const BcidVector& bcidsPerDelay) const;

    /**
     * \brief Return a list of BCIDs whose full range is observed for all PFEBs when scanning delays.
     *        See getViableBcids for an example for one PFEB.
     *
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning delays
     */
    [[nodiscard]]
    BcidVector getViableBcids(const std::vector<BcidVector>& bcidPerPfebPerDelay) const;

    /**
     * \brief Return the target BCID, given a list of BCID observed for each PFEB and delay.
     *        The target BCID should be a viable BCID for all PFEBs.
     *        If more than one BCID are viable, then the BCID which incurs the least delay is chosen.
     *
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning delays
     */
    [[nodiscard]]
    std::uint32_t getTargetBcid(const std::vector<BcidVector>& bcidPerPfebPerDelay) const;

    /**
     * \brief Return the target delays, given a target BCID and list of BCID observed for each PFEB and delay.
     *
     * \param bcid The target BCID
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning delays
     */
    [[nodiscard]]
    DelayVector getTargetDelays(const std::uint32_t targetBcid,
                                const std::vector<BcidVector>& bcidPerPfebPerDelay) const;

    /**
     * \brief Return the target delay, given a target BCID and list of BCID observed for each delay.
     *
     * \param bcid The target BCID
     * \param bcidsPerDelay The BCIDs observed when scanning delays
     */
    [[nodiscard]]
    std::uint32_t getTargetDelay(const std::uint32_t targetBcid,
                                 const BcidVector& bcidPerDelay) const;

    /**
     * \brief Return the median delay for a particular BCID
     *
     * \param bcid The particular BCID under test
     * \param bcidsPerPfebPerDelay For each PFEB, the BCIDs observed when scanning delays
     */
    std::uint32_t getMedianDelay(const std::uint32_t bcid,
                                 const std::vector<BcidVector>& bcidPerPfebPerDelay) const;

    /**
     * \brief Get the \ref PadTriggerConfig object associated with this PadTrigger object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] boost::property_tree::ptree& getConfig() { return m_ptree; }
    [[nodiscard]] const boost::property_tree::ptree& getConfig() const { return m_ptree; }  //!< overload

    /**
     * \brief Get the \ref associated I2cMasterConfig object
     */
    [[nodiscard]]
    const I2cMasterConfig & getFpga() const
    { return m_padtriggerfpga; };

    /**
     * \brief Get the register address from the register name
     *
     * e.g., "00B_register_description" returns 0x00B
     */
    [[nodiscard]]
    std::uint8_t addressFromRegisterName(const std::string& name) const;

    /**
     * \brief Convert the "firmware" string into an integer
     *
     * e.g., "2021.01.01" returns 20210101
     */
    [[nodiscard]]
    std::uint32_t firmware_dateword() const;

    /**
     * \brief Get the "firmware" provided by the user configuration
     */
    [[nodiscard]]
    std::string firmware() const
    { return m_ptree.get("firmware", std::string{""}); };

    /**
     * \brief Get the "ConfigFPGA" provided by the user configuration
     */
    [[nodiscard]]
    bool ConfigFPGA() const
    { return m_ptree.get("ConfigFPGA", true); };

    /**
     * \brief Get the "ConfigRepeaters" provided by the user configuration
     */
    [[nodiscard]]
    bool ConfigRepeaters() const
    { return m_ptree.get<bool>("ConfigRepeaters"); };

    /**
     * \brief Get the "ConfigVTTx" provided by the user configuration
     */
    [[nodiscard]]
    bool ConfigVTTx() const
    { return m_ptree.get<bool>("ConfigVTTx"); };

    /**
     * \brief Get the "OcrEnable" provided by the user configuration
     */
    [[nodiscard]]
    bool OcrEnable() const
    { return m_ptree.get("OcrEnable", false); };

    /**
     * \brief Get the "GtReset" provided by the user configuration
     */
    [[nodiscard]]
    bool GtReset() const
    { return m_ptree.get("GtReset", false); };

    /**
     * \brief Get the "GtRxLolReset" provided by the user configuration
     */
    [[nodiscard]]
    bool GtRxLolReset() const
    { return m_ptree.get("GtRxLolReset", false); };

    /**
     * \brief Get the "BcidErrorReset" provided by the user configuration
     */
    [[nodiscard]]
    bool BcidErrorReset() const
    { return m_ptree.get("BcidErrorReset", false); };

    /**
     * \brief Get the "Toggle" provided by the user configuration
     */
    [[nodiscard]]
    bool Toggle() const
    { return m_ptree.get("Toggle", false); };

    /**
     * \brief Get the "DeskewPFEBs" provided by the user configuration
     */
    [[nodiscard]]
    bool Deskew() const
    { return m_ptree.get("Deskew", false); };

    /**
     * \brief Get the "SelectStrobe" provided by the user configuration
     */
    [[nodiscard]]
    bool SelectStrobe() const
    { return m_ptree.get("SelectStrobe", false); };

    /**
     * \brief Get the "ForceFirmwareUpload" provided by the user configuration
     */
    [[nodiscard]]
    bool ForceFirmwareUpload() const
    { return m_ptree.get("ForceFirmwareUpload", true); };

    /**
     * \brief Get the "LatencyScanStart" if provided by the user configuration
     */
    std::uint32_t LatencyScanStart() const
    { return m_ptree.get<std::uint32_t>("LatencyScanStart"); };

    /**
     * \brief Get the "LatencyScanNBC" if provided by the user configuration
     */
    std::uint32_t LatencyScanNBC() const
    { return m_ptree.get<std::uint32_t>("LatencyScanNBC"); };

    /**
     * \brief Get the "TrigBcidSelect" if provided by the user configuration
     */
    std::uint32_t TrigBcidSelect() const
    { return m_ptree.get("TrigBcidSelect", std::uint32_t{0x39}); };

    /**
     * \brief Decode a vector of PFEB BCIDs from the three PFEB BCID registers
     */
    ValueVector PFEBValues(std::uint32_t val_07_00,
                           std::uint32_t val_15_08,
                           std::uint32_t val_23_16
                           ) const;

    /**
     * \brief Read the FPGA temperature via a status register
     */
    std::uint32_t readFPGATemperature() const;

    /**
     * \brief Convert a Xilinx XADC value into a temperature
     * Report integer temperature because decimal places arent important
     */
    static std::uint32_t xadcToCelsius(std::uint32_t temp);

  private:

    /**
     * \brief Convert a vector into string of hex nibbles
     */
    static void pushBackColumn(std::vector< std::vector<std::uint32_t > >& matrix,
                               const std::vector<uint32_t>& column);

    /**
     * \brief Convert a vector into string of hex nibbles
     */
    static std::string joinHexReversed(const std::vector<uint32_t>& vec);

    boost::property_tree::ptree m_ptree; //!< ptree object associated with this PadTrigger
    std::string m_scaAddressFPGA;        //!< SCA address of PadTrigger FPGA i2c
    std::string m_scaAddressJTAG;        //!< SCA address of PadTrigger FPGA JTAG
    std::string m_name;                  //!< Name composed of OPC and SCA addresses
    I2cMasterConfig m_padtriggerfpga;    //!< I2cMasterConfig object from user configuration

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
        {0x01, 0x14, 0x55}, {0x01, 0x16, 0x55}, {0x01, 0x18, 0x55}, {0x01, 0x1A, 0x55},
        {0x02, 0x14, 0x55}, {0x02, 0x16, 0x55}, {0x02, 0x18, 0x55}, {0x02, 0x1A, 0x55},
        {0x03, 0x14, 0x55}, {0x03, 0x16, 0x55}, {0x03, 0x18, 0x55}, {0x03, 0x1A, 0x55},
        {0x04, 0x14, 0x55}, {0x04, 0x16, 0x55}, {0x04, 0x18, 0x55}, {0x04, 0x1A, 0x55},
        {0x05, 0x14, 0x55}, {0x05, 0x16, 0x55}, {0x05, 0x18, 0x55}, {0x05, 0x1A, 0x55},
        {0x06, 0x14, 0x55}, {0x06, 0x16, 0x55}, {0x06, 0x18, 0x55}, {0x06, 0x1A, 0x55},
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

    static constexpr std::string_view FPGA_DONE{"FPGA_DONE"};
    static constexpr std::uint32_t ZERO32{0};

  };

}  // namespace nsw::hw

#endif
