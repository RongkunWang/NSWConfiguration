// NSW constants

#ifndef NSWCONFIGURATION_CONSTANTS_H_
#define NSWCONFIGURATION_CONSTANTS_H_

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <utility> // for std::pair
#include <chrono>
#include <map>
#include <cmath>

namespace nsw {
  /// Traditional magic debug value
  constexpr std::uint32_t DEADBEEF = 0xdeadbeef;

  /// Special string for i2c registers and VMM codec
  constexpr std::string_view NOT_USED{"NOT_USED"};

  /// Fool me once
  constexpr std::size_t MAX_ATTEMPTS = 5;

  /// Fool me twice
  constexpr std::size_t RETRY_TWICE = 2;

  /// Make it clear when we are using 0 to indicate auto-detection (used in std::stoul, for example)
  constexpr std::size_t BASE_AUTO = 0;

  /// Make it clear when we are using 2 to indicate binary
  constexpr std::size_t BASE_BIN = 2;

  /// Make it clear when we are using 16 to indicate hexadecimal
  constexpr std::size_t BASE_HEX = 16;

  /// Make it clear when we are using 8 as the size in bits of a byte
  constexpr std::size_t NUM_BITS_IN_BYTE = 8;

  /// Make it clear when we are using 32 as the size in bits of a 32-bit word
  constexpr std::size_t NUM_BITS_IN_WORD32 = 32;

  /// Make it clear when we are using 4 as the size in bytes of a 32-bit word
  constexpr std::size_t NUM_BYTES_IN_WORD32 = 4;

  /// Make it clear when we are using 64 as the size in bits of a 64-bit int
  constexpr std::size_t NUM_BITS_IN_WORD64 = 64;

  /// Maximum size of an addressed register
  constexpr std::size_t MAX_REGISTER_SIZE = 32;

  /// Maximum number of VMMs on a FEB
  constexpr std::size_t MAX_NUMBER_OF_VMM = 8;

  /// Maximum number of TDSs on a FEB
  constexpr std::size_t MAX_NUMBER_OF_TDS = 4;

  /// Maximum number of ARTs on an ADDC
  constexpr std::size_t NUM_ART_PER_ADDC = 2;

  // An MMFE8 has 8 VMMs
  constexpr std::uint8_t NUM_VMM_PER_MMFE8 = 8;

  // An SFEB has 8 VMMs
  constexpr std::uint8_t NUM_VMM_PER_SFEB = 8;

  // A PFEB has 3 VMMs
  constexpr std::uint8_t NUM_VMM_PER_PFEB = 3;

  // An MMFE8 has 0 TDSs
  constexpr std::uint8_t NUM_TDS_PER_MMFE8 = 0;

  // An SFEB_old has 3 TDSs
  constexpr std::uint8_t NUM_TDS_PER_SFEB_OLD  = 3;

  // An SFEB has 4 TDSs
  constexpr std::uint8_t NUM_TDS_PER_SFEB  = 4;

  // A PFEB has 1 TDSs
  constexpr std::uint8_t NUM_TDS_PER_PFEB = 1;

  // The first VMM on the sFEB6 is in position 2
  constexpr std::size_t SFEB6_FIRST_VMM = 2;

  // The first VMM on the sFEB6 is in position 2
  constexpr std::size_t SFEB6_FIRST_TDS = 1;

  // A PFEB has 1 wire VMM and 2 pad VMMs
  // The first pad VMM is in position 1
  constexpr std::size_t PFEB_WIRE_VMM       = 0;
  constexpr std::size_t PFEB_FIRST_PAD_VMM  = 1;

  // A PFEB has 2 pad VMMs
  constexpr std::size_t NUM_PAD_VMM_PER_PFEB = NUM_VMM_PER_PFEB - PFEB_FIRST_PAD_VMM;

  /// Number of bytes at an I2C address
  constexpr std::size_t I2C_ADDRESS_BYTE_SIZE = 8;

  /// Maximum size of an addressed I2C register
  constexpr std::size_t MAX_I2C_REGISTER_SIZE = 32;

  constexpr std::size_t MIN_LAYER_ID = 0;
  constexpr std::size_t MAX_LAYER_ID = 7;
  constexpr std::size_t NUM_LAYERS_PER_TECH = 8;
  constexpr std::size_t NUM_RADII_MM   = 16;
  constexpr std::size_t NUM_RADII_STGC = 3;

  constexpr std::size_t MIN_SECTOR_ID = 1;
  constexpr std::size_t MAX_SECTOR_ID = 16;

  /// Number of wheels and sectors
  constexpr std::size_t NUM_WHEELS = 2;
  constexpr std::size_t NUM_SECTORS = 16;

  // Magic names of electronics
  // Do not easily change the order.
  const std::vector<std::string> ELEMENT_NAMES = {
    "MMFE8",
    "PFEB",
    "SFEB_old",
    "SFEB8",
    "SFEB6",
    "SFEB",
    "TPCarrier",
    "STGCTP",
    "TP",
    "ADDC",
    "PadTrigger",
    "Router",
    "GBTX",
    "RimL1DDC",
    "L1DDC",
  };

  namespace geoid {
    enum class Detector { MM, STGC, UNKNOWN };
    enum class Wheel { A, C, UNKNOWN };
    constexpr static std::uint8_t DoesNotExist{0xff};
  }

  namespace l1ddc {
      // Numbers of GBTx on each kind of L1DDC
      constexpr std::size_t MMG_L1DDC_NUMBER_GBTx = 3;
      constexpr std::size_t RIM_L1DDC_NUMBER_GBTx = 1;
      constexpr std::size_t PFEB_L1DDC_NUMBER_GBTx = 2;
      constexpr std::size_t SFEB_L1DDC_NUMBER_GBTx = 2;
  }

  namespace roc {
    namespace mmfe8 {
      namespace digital {
        constexpr std::uint8_t SCL_LINE_PIN = 17;
        constexpr std::uint8_t SDA_LINE_PIN = 18;
      }  // namespace digital

      namespace analog {
        constexpr std::uint8_t SCL_LINE_PIN = 19;
        constexpr std::uint8_t SDA_LINE_PIN = 20;
      }  // namespace analog
    }    // namespace mmfe8

    namespace sfeb {  // Also valid for PFEBs
      namespace digital {
        constexpr std::uint8_t SCL_LINE_PIN = 31;
        constexpr std::uint8_t SDA_LINE_PIN = 30;
      }  // namespace digital

      namespace analog {
        constexpr std::uint8_t SCL_LINE_PIN = 21;
        constexpr std::uint8_t SDA_LINE_PIN = 18;
      }  // namespace analog
    }    // namespace sfeb

    constexpr std::size_t NUM_SROCS = 4;
    constexpr std::size_t NUM_PHASES_CTRL_PHASE = 8;
    constexpr std::size_t NUM_PHASES_EPLL_TDS_40MHZ = 128;
  }      // namespace roc

  namespace vmm {
    constexpr std::uint8_t NUM_CH_PER_VMM = 64;
  }

  namespace mmfe8 {
    constexpr std::size_t MMFE8_PER_LAYER = 16; ///< 16 MMFE8s per single MM layer
    constexpr std::size_t NUM_CH_PER_MMFE8 = nsw::vmm::NUM_CH_PER_VMM*nsw::NUM_VMM_PER_MMFE8; ///< 512 channels per MMFE8
    constexpr std::size_t NUM_CH_PER_LAYER = MMFE8_PER_LAYER*NUM_CH_PER_MMFE8; ///< 8192 channels per single MM layer
    constexpr std::size_t MMFE8_PER_SECTOR = MMFE8_PER_LAYER*NUM_LAYERS_PER_TECH; ///< 128 MMFE8s per sector
    constexpr std::size_t NUM_CH_PER_SECTOR = MMFE8_PER_SECTOR*NUM_CH_PER_MMFE8; ///< 65536 channels per sector
  }

  namespace tds {
    constexpr std::size_t NUM_CH_PER_PAD_TDS   = 104;
    constexpr std::size_t NUM_CH_PER_STRIP_TDS = 128;
  }

  namespace art {
    constexpr std::size_t ADDRESS_SIZE = 1;
    const std::vector<uint32_t> REG_INPUT_PHASES = {
      6,  7,  8,  9,
      21, 22, 23, 24,
      36, 37, 38, 39,
      51, 52, 53, 54,
    };
    constexpr std::uint32_t REG_COUNTERS_START  = 128;
    constexpr std::uint32_t REG_COUNTERS_END    = 256;
    constexpr std::uint32_t REG_COUNTERS_SIZE   = 4;
    constexpr std::uint32_t REG_COUNTERS_SIMULT = 16;
    constexpr std::uint32_t NUM_PHASE_INPUT     = 16;
  }

  namespace padtrigger {
    constexpr bool SCA_LITTLE_ENDIAN = false;
    constexpr std::uint32_t NUM_INPUT_DELAYS       = 16;
    constexpr std::uint32_t NUM_INPUT_DELAYS_PER_BC = 6;
    constexpr std::uint32_t NUM_INPUT_DELAYS_MARGIN = 2;
    constexpr std::uint32_t NUM_BITS_PER_PFEB_DELAY = 4;
    constexpr std::uint32_t NUM_PFEBS              = 24;
    constexpr std::uint32_t NUM_QUADS              = 6;
    constexpr std::uint32_t NUM_PFEBS_PER_QUAD     = NUM_PFEBS / NUM_QUADS;
    constexpr std::uint32_t NUM_PFEB_BCID_READS    = 100;
    constexpr std::uint32_t NUM_DESKEW_READS       = 10;
    constexpr std::uint32_t NUM_TRIGGER_RATE_READS = 2;
    constexpr std::uint32_t NUM_BITS_PER_PFEB_BCID = 4;
    constexpr std::uint32_t PFEB_BCID_RANGE        = (1 << NUM_BITS_PER_PFEB_BCID);
    constexpr std::uint32_t PFEB_BCID_BITMASK      = 0b1111;
    constexpr std::uint32_t PFEB_BCID_DISCONNECTED = 0x5;
    constexpr std::uint32_t TRIGGER_RATE_BITSHIFT  = 16;
    constexpr std::chrono::seconds PFEB_HIT_RATE_TIME{1};
    constexpr std::uint8_t REG_CONTROL          = 0x00;
    constexpr std::uint8_t REG_STATUS           = 0x01;
    constexpr std::uint8_t REG_L1ID             = 0x02;
    constexpr std::uint8_t REG_CONTROL2         = 0x03;
    constexpr std::uint8_t REG_PFEB_DELAY_23_16 = 0x04;
    constexpr std::uint8_t REG_PFEB_DELAY_15_08 = 0x05;
    constexpr std::uint8_t REG_PFEB_DELAY_07_00 = 0x06;
    constexpr std::uint8_t REG_PFEB_BCID_23_16  = 0x07;
    constexpr std::uint8_t REG_PFEB_BCID_15_08  = 0x08;
    constexpr std::uint8_t REG_PFEB_BCID_07_00  = 0x09;
    constexpr std::uint8_t REG_MASK_TO_0        = 0x0B;
    constexpr std::uint8_t REG_MASK_TO_1        = 0x0C;
    constexpr std::uint8_t REG_STATUS2          = 0x0D;
    constexpr std::uint8_t REG_CONTROL3         = 0x0E;
    constexpr std::uint8_t REG_PFEB_STATUS      = 0x0F;
    constexpr std::uint8_t REG_GT_RX_RESET      = 0x10;
    constexpr std::uint8_t REG_GT_TX_RESET      = 0x11;
    constexpr std::uint8_t REG_BCID_ERROR       = 0x12;
    constexpr std::uint8_t REG_PT_2_TP_LAT      = 0x13;
    constexpr std::uint8_t REG_TP_BCID_ERROR    = 0x14;
    constexpr std::uint8_t REG_TTC_BCR_OCR_RATE = 0x15;

    constexpr std::uint32_t GT_RESET_ENABLE  = 0x00FFFFFF;
    constexpr std::uint32_t GT_RESET_DISABLE = 0x00000000;
    constexpr std::array<std::string_view, NUM_PFEBS> ORDERED_PFEBS = {
      "PFEB_L1Q1_IP", "PFEB_L2Q1_IP", "PFEB_L3Q1_IP", "PFEB_L4Q1_IP",
      "PFEB_L1Q1_HO", "PFEB_L2Q1_HO", "PFEB_L3Q1_HO", "PFEB_L4Q1_HO",
      "PFEB_L1Q2_IP", "PFEB_L2Q2_IP", "PFEB_L3Q2_IP", "PFEB_L4Q2_IP",
      "PFEB_L1Q2_HO", "PFEB_L2Q2_HO", "PFEB_L3Q2_HO", "PFEB_L4Q2_HO",
      "PFEB_L1Q3_IP", "PFEB_L2Q3_IP", "PFEB_L3Q3_IP", "PFEB_L4Q3_IP",
      "PFEB_L1Q3_HO", "PFEB_L2Q3_HO", "PFEB_L3Q3_HO", "PFEB_L4Q3_HO",
    };
    constexpr std::array<std::string_view, NUM_PFEBS> ORDERED_PFEBS_GEOID = {
      "/L0/R0", "/L1/R0", "/L2/R0", "/L3/R0", "/L4/R0", "/L5/R0", "/L6/R0", "/L7/R0",
      "/L0/R1", "/L1/R1", "/L2/R1", "/L3/R1", "/L4/R1", "/L5/R1", "/L6/R1", "/L7/R1",
      "/L0/R2", "/L1/R2", "/L2/R2", "/L3/R2", "/L4/R2", "/L5/R2", "/L6/R2", "/L7/R2",
    };
  }

  namespace router {
    constexpr std::size_t MAX_RESETS = 3;
    constexpr std::size_t MAX_GPIO_CHECKS = 5;
    constexpr std::chrono::seconds RESET_HOLD{0};
    constexpr std::chrono::seconds RESET_SLEEP{1};
    constexpr std::chrono::seconds RESET_PAUSE{1};
    constexpr std::chrono::seconds PAUSE_AFTER_RESET{1};
    constexpr std::chrono::seconds PAUSE_BETWEEN_RESETS{2};
  }

}  // namespace nsw

#endif  // NSWCONFIGURATION_CONSTANTS_H_
