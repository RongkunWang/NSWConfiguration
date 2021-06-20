// NSW constants

#ifndef NSWCONFIGURATION_CONSTANTS_H_
#define NSWCONFIGURATION_CONSTANTS_H_

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace nsw {
  /// Fool me once
  constexpr std::size_t MAX_ATTEMPTS = 5;

  /// Make it clear when we are using 8 as the size in bits of a byte
  constexpr std::size_t NUM_BITS_IN_BYTE = 8;

  /// Make it clear when we are using 4 as the size in bytes of a 32-bit word
  constexpr std::size_t NUM_BYTES_IN_WORD32 = 4;

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

  /// Number of bytes at an I2C address
  constexpr std::size_t I2C_ADDRESS_BYTE_SIZE = 8;

  /// Maximum size of an addressed I2C register
  constexpr std::size_t MAX_I2C_REGISTER_SIZE = 32;

  constexpr std::size_t MIN_LAYER_ID = 0;
  constexpr std::size_t MAX_LAYER_ID = 7;
  constexpr std::size_t NUM_LAYERS_MM   = 8;
  constexpr std::size_t NUM_LAYERS_STGC = 8;

  constexpr std::size_t MIN_SECTOR_ID = 1;
  constexpr std::size_t MAX_SECTOR_ID = 16;

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
    "TP",
    "ADDC",
    "PadTriggerSCA",
    "Router",
  };

  // FIXME only valid for MMFE8
  namespace roc {
    namespace digital {
      constexpr std::uint8_t SCL_LINE_PIN = 17;
      constexpr std::uint8_t SDA_LINE_PIN = 18;
    }

    namespace analog {
      constexpr std::uint8_t SCL_LINE_PIN = 19;
      constexpr std::uint8_t SDA_LINE_PIN = 20;
    }
  }

  namespace vmm {
    constexpr std::uint8_t NUM_CH_PER_VMM = 64;
  }

  namespace mmfe8 {
    constexpr std::size_t MMFE8_PER_LAYER = 16; ///< 16 MMFE8s per single MM layer
    constexpr std::size_t NUM_CH_PER_MMFE8 = nsw::vmm::NUM_CH_PER_VMM*nsw::NUM_VMM_PER_MMFE8; ///< 512 channels per MMFE8
    constexpr std::size_t NUM_CH_PER_LAYER = MMFE8_PER_LAYER*NUM_CH_PER_MMFE8; ///< 8192 channels per single MM layer
    constexpr std::size_t MMFE8_PER_SECTOR = MMFE8_PER_LAYER*NUM_LAYERS_MM; ///< 128 MMFE8s per sector
    constexpr std::size_t NUM_CH_PER_SECTOR = MMFE8_PER_SECTOR*NUM_CH_PER_MMFE8; ///< 65536 channels per sector
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

  namespace scax {
    constexpr bool SCAX_LITTLE_ENDIAN = true;
  }

  namespace carrier {
    constexpr std::uint32_t RJOUT_SEL_STGC = 0x00;
    constexpr std::uint32_t RJOUT_SEL_MM   = 0x01;
    constexpr std::uint8_t REG_RJOUT_SEL = 0x01;
  }

  namespace mmtp {
    constexpr std::size_t PIPELINE_OVERFLOW_READS = 10;
    constexpr std::size_t FIBER_ALIGN_ATTEMPTS    = 10;
    constexpr std::size_t FIBER_ALIGN_SLEEP       = 5e6;
    constexpr std::size_t FIBER_ALIGN_N_READS     = 100;
    constexpr std::size_t NUM_FIBERS              = 32;
    constexpr std::size_t NUM_ADDCS               = 16;
    constexpr std::size_t NUM_FIBERS_PER_QPLL     = 4;
    constexpr std::size_t NUM_FIBER_BCID_REGS     = 4;
    constexpr std::size_t NUM_VMMS_PER_FIBER      = 32;
    constexpr std::size_t NUM_MMFE8_PER_FIBER     = 4;
    constexpr std::size_t CHAN_RATE_USLEEP        = 1e6;
    constexpr std::uint8_t DUMMY_VAL              = 0x55;
    constexpr std::uint8_t REG_ADDC_EMU_DISABLE   = 0x01;
    constexpr std::uint8_t REG_FIBER_ALIGNMENT    = 0x02;
    constexpr std::uint8_t REG_FIBER_QPLL_RESET   = 0x03;
    constexpr std::uint8_t REG_FIBER_BCIDS_00_07  = 0x04;
    constexpr std::uint8_t REG_FIBER_BCIDS_08_15  = 0x05;
    constexpr std::uint8_t REG_FIBER_BCIDS_16_23  = 0x06;
    constexpr std::uint8_t REG_FIBER_BCIDS_24_31  = 0x07;
    constexpr std::array<std::uint8_t, NUM_FIBER_BCID_REGS> REG_FIBER_BCIDS = {
      REG_FIBER_BCIDS_00_07,
      REG_FIBER_BCIDS_08_15,
      REG_FIBER_BCIDS_16_23,
      REG_FIBER_BCIDS_24_31,
    };
    constexpr std::uint8_t REG_FIBER_BC_OFFSET          = 0x08;
    constexpr std::uint8_t REG_L1A_CONTROL              = 0x10;
    constexpr std::uint8_t REG_L1A_LATENCY              = 0x11;
    constexpr std::uint8_t REG_L1A_WIN_UPPER            = 0x12;
    constexpr std::uint8_t REG_L1A_WIN_LOWER            = 0x13;
    constexpr std::uint8_t REG_L1A_STATUS               = 0x14;
    constexpr std::uint8_t REG_INPUT_PHASE              = 0x0b;
    constexpr std::uint8_t REG_INPUT_PHASEOFFSET        = 0x0c;
    constexpr std::uint8_t REG_FIBER_HOT_MUX            = 0x0d;
    constexpr std::uint8_t REG_FIBER_HOT_READ           = 0x0e;
    constexpr std::uint8_t REG_FIBER_MASK_MUX           = 0x1c;
    constexpr std::uint8_t REG_FIBER_MASK_WRITE         = 0x1d;
    constexpr std::uint8_t REG_PIPELINE_OVERFLOW        = 0x20;
    constexpr std::uint8_t REG_SELFTRIGGER_DELAY        = 0x21;
    constexpr std::uint8_t REG_VMM_MASK_HOT_THRESH      = 0x22;
    constexpr std::uint8_t REG_VMM_MASK_HOT_THRESH_HYST = 0x23;
    constexpr std::uint8_t REG_VMM_MASK_DRAIN_PERIOD    = 0x24;
    constexpr std::uint8_t REG_CHAN_RATE_ENABLE         = 0x25;
    constexpr std::array<std::string_view, NUM_ADDCS> ORDERED_ADDCS = {
      "ADDC_L1P6_IPR",
      "ADDC_L1P3_IPL",
      "ADDC_L1P3_IPR",
      "ADDC_L1P6_IPL",
      "ADDC_L4P6_IPR",
      "ADDC_L4P3_IPL",
      "ADDC_L4P3_IPR",
      "ADDC_L4P6_IPL",
      "ADDC_L4P6_HOR",
      "ADDC_L4P3_HOL",
      "ADDC_L4P3_HOR",
      "ADDC_L4P6_HOL",
      "ADDC_L1P6_HOR",
      "ADDC_L1P3_HOL",
      "ADDC_L1P3_HOR",
      "ADDC_L1P6_HOL",
    };
    constexpr std::uint32_t L1A_RESET_ENABLE  = 0xFF;
    constexpr std::uint32_t L1A_RESET_DISABLE = 0x00;
  }

  namespace padtrigger {
    constexpr bool SCA_LITTLE_ENDIAN = false;
    constexpr std::uint32_t NUM_INPUT_DELAYS = 16;
    constexpr std::uint32_t NUM_PFEB_DELAY_REGS = 3;
    constexpr std::uint32_t NUM_PFEB_BCID_REGS  = 3;
    constexpr std::uint32_t REG_CONTROL          = 0x00;
    constexpr std::uint32_t REG_STATUS           = 0x01;
    constexpr std::uint32_t REG_L1ID             = 0x02;
    constexpr std::uint32_t REG_CONTROL2         = 0x03;
    constexpr std::uint32_t REG_PFEB_DELAY_23_16 = 0x04;
    constexpr std::uint32_t REG_PFEB_DELAY_15_08 = 0x05;
    constexpr std::uint32_t REG_PFEB_DELAY_07_00 = 0x06;
    constexpr std::uint32_t REG_PFEB_BCID_23_16  = 0x07;
    constexpr std::uint32_t REG_PFEB_BCID_15_08  = 0x08;
    constexpr std::uint32_t REG_PFEB_BCID_07_00  = 0x09;
    constexpr std::array<std::uint8_t, NUM_PFEB_DELAY_REGS>
      REG_PFEB_DELAYS = {
      REG_PFEB_DELAY_23_16,
      REG_PFEB_DELAY_15_08,
      REG_PFEB_DELAY_07_00,
    };
    constexpr std::array<std::uint8_t, NUM_PFEB_BCID_REGS>
      REG_PFEB_BCIDS = {
      REG_PFEB_BCID_23_16,
      REG_PFEB_BCID_15_08,
      REG_PFEB_BCID_07_00,
    };
  }

}  // namespace nsw

#endif  // NSWCONFIGURATION_CONSTANTS_H_
