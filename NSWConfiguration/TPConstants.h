#ifndef NSWCONFIGURATION_TPCONSTANTS_H_
#define NSWCONFIGURATION_TPCONSTANTS_H_

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

  namespace scax {
    constexpr bool SCAX_LITTLE_ENDIAN = true;
    constexpr std::uint32_t BITMASK_ALL = 0xffffffff;
  }

  namespace carrier {
    constexpr std::uint32_t RJOUT_SEL_STGC = 0x00;
    constexpr std::uint32_t RJOUT_SEL_MM   = 0x01;
    constexpr std::uint8_t REG_RJOUT_SEL = 0x01;
    constexpr std::uint8_t REG_MIRROW_RESET  = 0x02;
    constexpr std::uint8_t REG_GBT_PHASE_SKEW  = 0x0b;
    constexpr std::uint8_t REG_PRN_CHK_ERROR_CNT  = 0x0f;
    constexpr std::uint8_t REG_PRN_CHK_RESET    = 0x10;

    
    const std::map<std::string, uint32_t> REGS = {
      {"RJOUT_SEL",           0x01}, // R/W
      {"mirrowReset",         0x02}, // R/W
      {"gbtPhaseSkew",        0x0b}, // R
      {"gbtPhaseAdjust",      0x0c}, // W
      {"prnChkErrorCnt",      0x0f}, // R
      {"prnChkReset",         0x10}, // R/W
    };
  }

  namespace stgctp {
    constexpr std::uint8_t REG_ERR_BCID_MATCH   = 0x00;
    constexpr std::uint8_t REG_RST_RX           = 0x01;
    constexpr std::uint8_t REG_RST_TX           = 0x02;
    constexpr std::uint8_t REG_SL_LATENCY_COMP  = 0x03;
    constexpr std::uint8_t REG_BCR_RATE         = 0x04;
    constexpr std::uint8_t REG_PAD_BXID_SYNC_OK = 0x05;
    constexpr std::uint8_t REG_PAD_RATE         = 0x06;
    constexpr std::uint8_t REG_MON_LATENCY      = 0x07;
    constexpr std::uint8_t REG_IN_RUN           = 0x08;
    constexpr std::uint8_t REG_STRIPS_HIT_RATE  = 0x09;
    constexpr std::uint8_t REG_DESKEW_OFFSET    = 0x0a;
    constexpr std::uint8_t REG_VALID_NULL_ERROR = 0x0b;
    constexpr std::uint8_t REG_STGC_MM_DISABLE  = 0x0c;
    constexpr std::uint8_t REG_TO_SL_RATE       = 0x0d;
    constexpr std::uint8_t REG_SECTOR           = 0x0e;
    constexpr std::uint8_t REG_MM_BXID_SYNC_OK  = 0x0f;
    constexpr std::uint8_t REG_PAD_IDLE_STATUS  = 0x10; // equal to pad trigger empty flag in FIFO
    constexpr std::uint8_t REG_MM_IDLE_STATUS   = 0x11;
    // Status                                    | 0x10 | 0x11
    // pad trigger sending junk, MMG not idle    | 1    | 0 
    // pad trigger sending junk, MMG idle        | 0    | 1
    // both idle                                 | 1    | 1
    constexpr std::uint8_t REG_PAD_ARRIVAL_BC   = 0x12;
    constexpr std::uint8_t REG_MM_ARRIVAL_BC    = 0x13;
    constexpr std::uint8_t REG_IGNORE_PADS           = 0x14;
    constexpr std::uint8_t REG_IGNORE_MM             = 0x15;
    constexpr std::uint8_t REG_DISABLE_NSWMON        = 0x16;
    constexpr std::uint8_t REG_L1A_OPENING_OFFSET       = 0x17;
    constexpr std::uint8_t REG_L1A_REQUEST_OFFSET       = 0x18;
    constexpr std::uint8_t REG_L1A_CLOSING_OFFSET       = 0x19;
    constexpr std::uint8_t REG_L1A_TIMEOUT_WINDOW       = 0x1a;
    constexpr std::uint8_t REG_L1A_PAD_EN               = 0x1b;
    constexpr std::uint8_t REG_L1A_MERGE_EN             = 0x1c;
    constexpr std::uint8_t REG_STICKY_ERR_BCID_MATCH    = 0x1d;
    constexpr std::uint8_t REG_STGC_GLOSYNC_BCID_OFFSET = 0x1e;
    constexpr std::uint8_t REG_BUSY                     = 0x1f;
    constexpr std::uint8_t REG_MON_DISABLE              = 0x20;
    constexpr std::uint8_t REG_NSW_MON_LIMIT            = 0x21;
    constexpr std::uint8_t REG_MON_LIMIT                = 0x22;
    constexpr std::uint8_t REG_MM_NSW_MON_EN            = 0x23;
    constexpr std::uint8_t REG_SMALL_SECTOR             = 0x24;
    constexpr std::uint8_t REG_NO_STRETCH               = 0x25;
    constexpr std::uint8_t REG_SYNC_FIFO_EMPTY          = 0x28;
    constexpr std::uint32_t MASK_ERR_BCID_MATCH   = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_RST_RX           = (1 <<  9) - 1;
    constexpr std::uint32_t MASK_RST_TX           = (1 <<  9) - 1;
    constexpr std::uint32_t MASK_SL_LATENCY_COMP  = (1 <<  3) - 1;
    constexpr std::uint32_t MASK_BCR_RATE         = (1 << 16) - 1;
    constexpr std::uint32_t MASK_PAD_BXID_SYNC_OK = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_PAD_RATE         = (1 << 28) - 1;
    constexpr std::uint32_t MASK_MON_LATENCY      = (1 << 12) - 1;
    constexpr std::uint32_t MASK_IN_RUN           = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_STRIPS_HIT_RATE  = (1 << 26) - 1;
    constexpr std::uint32_t MASK_DESKEW_OFFSET    = (1 <<  9) - 1;
    constexpr std::uint32_t MASK_VALID_NULL_ERROR = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_STGC_MM_DISABLE  = (1 <<  2) - 1;
    constexpr std::uint32_t MASK_TO_SL_RATE       = (1 << 26) - 1;
    constexpr std::uint32_t MASK_SECTOR           = (1 <<  4) - 1;
    constexpr std::uint32_t MASK_MM_BXID_SYNC_OK  = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_PAD_IDLE_STATUS  = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_MM_IDLE_STATUS   = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_PAD_ARRIVAL_BC   = (1 << 16) - 1;
    constexpr std::uint32_t MASK_MM_ARRIVAL_BC    = (1 << 16) - 1;
    constexpr std::uint32_t MASK_IGNORE_PADS           = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_IGNORE_MM             = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_DISABLE_NSWMON        = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_L1A_OPENING_OFFSET       = (1 << 12) - 1;
    constexpr std::uint32_t MASK_L1A_REQUEST_OFFSET       = (1 << 12) - 1;
    constexpr std::uint32_t MASK_L1A_CLOSING_OFFSET       = (1 << 12) - 1;
    constexpr std::uint32_t MASK_L1A_TIMEOUT_WINDOW       = (1 << 12) - 1;
    constexpr std::uint32_t MASK_L1A_PAD_EN               = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_L1A_MERGE_EN             = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_STICKY_ERR_BCID_MATCH    = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_STGC_GLOSYNC_BCID_OFFSET = (1 << 12) - 1;
    constexpr std::uint32_t MASK_BUSY                     = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_MON_DISABLE              = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_NSW_MON_LIMIT            = (1 <<  7) - 1;
    constexpr std::uint32_t MASK_MON_LIMIT                = (1 << 17) - 1;
    constexpr std::uint32_t MASK_MM_NSW_MON_EN            = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_SMALL_SECTOR             = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_NO_STRETCH               = (1 <<  1) - 1;
    constexpr std::uint32_t MASK_SYNC_FIFO_EMPTY          = (1 <<  1) - 1;
    constexpr auto REGS = std::to_array({
        std::make_pair(REG_ERR_BCID_MATCH,   MASK_ERR_BCID_MATCH),
        std::make_pair(REG_BCR_RATE,         MASK_BCR_RATE),
        std::make_pair(REG_PAD_BXID_SYNC_OK, MASK_PAD_BXID_SYNC_OK),
        std::make_pair(REG_PAD_RATE,         MASK_PAD_RATE),
        std::make_pair(REG_IN_RUN,           MASK_IN_RUN),
        std::make_pair(REG_STRIPS_HIT_RATE,  MASK_STRIPS_HIT_RATE),
        std::make_pair(REG_DESKEW_OFFSET,    MASK_DESKEW_OFFSET),
        std::make_pair(REG_VALID_NULL_ERROR, MASK_VALID_NULL_ERROR),
        std::make_pair(REG_TO_SL_RATE,       MASK_TO_SL_RATE),
        std::make_pair(REG_SECTOR,           MASK_SECTOR),
        std::make_pair(REG_MM_BXID_SYNC_OK,  MASK_MM_BXID_SYNC_OK),
        std::make_pair(REG_PAD_IDLE_STATUS,  MASK_PAD_IDLE_STATUS),
        std::make_pair(REG_MM_IDLE_STATUS,   MASK_MM_IDLE_STATUS),
        std::make_pair(REG_PAD_ARRIVAL_BC,   MASK_PAD_ARRIVAL_BC),
        std::make_pair(REG_MM_ARRIVAL_BC,    MASK_MM_ARRIVAL_BC),
        std::make_pair(REG_IGNORE_PADS,           MASK_IGNORE_PADS),
        std::make_pair(REG_IGNORE_MM,             MASK_IGNORE_MM),
        std::make_pair(REG_STGC_MM_DISABLE,       MASK_STGC_MM_DISABLE),
        std::make_pair(REG_DISABLE_NSWMON,        MASK_DISABLE_NSWMON),
        std::make_pair(REG_L1A_OPENING_OFFSET,       MASK_L1A_OPENING_OFFSET),
        std::make_pair(REG_L1A_REQUEST_OFFSET,       MASK_L1A_REQUEST_OFFSET),
        std::make_pair(REG_L1A_CLOSING_OFFSET,       MASK_L1A_CLOSING_OFFSET),
        std::make_pair(REG_L1A_TIMEOUT_WINDOW,       MASK_L1A_TIMEOUT_WINDOW),
        std::make_pair(REG_L1A_PAD_EN,               MASK_L1A_PAD_EN),
        std::make_pair(REG_L1A_MERGE_EN,             MASK_L1A_MERGE_EN),
        std::make_pair(REG_STICKY_ERR_BCID_MATCH,    MASK_STICKY_ERR_BCID_MATCH),
        std::make_pair(REG_BUSY,                     MASK_BUSY),
        std::make_pair(REG_MON_DISABLE,              MASK_MON_DISABLE),
        std::make_pair(REG_NSW_MON_LIMIT,            MASK_NSW_MON_LIMIT),
        std::make_pair(REG_MON_LIMIT,                MASK_MON_LIMIT),
        std::make_pair(REG_MM_NSW_MON_EN,            MASK_MM_NSW_MON_EN),
        std::make_pair(REG_SMALL_SECTOR,             MASK_SMALL_SECTOR),
        std::make_pair(REG_NO_STRETCH,               MASK_NO_STRETCH),
        std::make_pair(REG_SYNC_FIFO_EMPTY,          MASK_SYNC_FIFO_EMPTY),
    });
    constexpr std::uint32_t RST_RX_ENABLE  = 0b111111111;
    constexpr std::uint32_t RST_RX_DISABLE = 0b0;
    constexpr std::uint32_t RST_TX_ENABLE  = 0b100111111;
    constexpr std::uint32_t RST_TX_DISABLE = 0b0;
  }

  namespace mmtp {
    constexpr std::chrono::seconds FIBER_ALIGN_SLEEP{3};
    constexpr float CONVERTER_HORX_FIBER_POWER_UW = 0.1f;
    constexpr float CONVERTER_HORX_TEMP_C         = 1. / 256.;
    constexpr std::size_t FIBER_ALIGN_ATTEMPTS    = 10;
    constexpr std::size_t FIBER_ALIGN_N_READS     = 100;
    constexpr std::size_t PIPELINE_OVERFLOW_READS = 10;
    constexpr std::size_t NUM_ADDCS               = 16;
    constexpr std::size_t NUM_FIBERS              = 32;
    constexpr std::size_t NUM_FIBERS_PER_MICROPOD = 12;
    constexpr std::size_t NUM_FIBERS_PER_QPLL     = 4;
    constexpr std::size_t NUM_QPLL                = NUM_FIBERS/NUM_FIBERS_PER_QPLL;
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
    constexpr std::uint8_t REG_DATE_CODE                = 0x09;
    constexpr std::uint8_t REG_GIT_HASH                 = 0x0a;
    constexpr std::uint8_t REG_INPUT_PHASE              = 0x0b;
    constexpr std::uint8_t REG_INPUT_PHASEADDCOFFSET    = 0x0c; // for ADDC PCB3/6
    constexpr std::uint8_t REG_INPUT_PHASEL1DDCOFFSET   = 0x32; // for L1DDC L/R
    constexpr std::uint8_t REG_FIBER_HOT_MUX            = 0x0d;
    constexpr std::uint8_t REG_FIBER_HOT_READ           = 0x0e;
    constexpr std::uint8_t REG_GBT_BCID_OK              = 0x0f;
    constexpr std::uint8_t REG_L1A_CONTROL              = 0x10;
    constexpr std::uint8_t REG_L1A_OPENING_OFFSET       = 0x11;
    constexpr std::uint8_t REG_L1A_REQUEST_OFFSET       = 0x12;
    constexpr std::uint8_t REG_L1A_CLOSING_OFFSET       = 0x13;
    constexpr std::uint8_t REG_L1A_TIMEOUT_WINDOW       = 0x14;
    constexpr std::uint8_t REG_L1A_BUSY_THRESHOLD       = 0x15;
    constexpr std::uint8_t REG_GBT_PHASE_OFFSET         = 0x18;
    constexpr std::uint8_t REG_TTC_ECR_CNT              = 0x19;
    constexpr std::uint8_t REG_TTC_BCR_CNT              = 0x1a;
    constexpr std::uint8_t REG_TTC_L1A_CNT              = 0x1b;
    constexpr std::uint8_t REG_FIBER_MASK_MUX           = 0x1c;
    constexpr std::uint8_t REG_FIBER_MASK_WRITE         = 0x1d;
    constexpr std::uint8_t REG_PIPELINE_OVERFLOW        = 0x20;
    constexpr std::uint8_t REG_SELFTRIGGER_DELAY        = 0x21;
    constexpr std::uint8_t REG_VMM_MASK_HOT_THRESH      = 0x22;
    constexpr std::uint8_t REG_VMM_MASK_HOT_THRESH_HYST = 0x23;
    constexpr std::uint8_t REG_VMM_MASK_DRAIN_PERIOD    = 0x24;
    constexpr std::uint8_t REG_CHAN_RATE_ENABLE         = 0x25;
    constexpr std::uint8_t REG_HORX_ENV_MON_ADDR        = 0x26;
    constexpr std::uint8_t REG_HORX_ENV_MON_DATA        = 0x27;
    constexpr std::uint8_t REG_GLO_SYNC_IDLE_STATE      = 0x28;
    constexpr std::uint8_t REG_GLO_SYNC_BCID_OFFSET     = 0x29;
    constexpr std::uint8_t REG_LAT_TX_IDLE_STATE        = 0x2F;
    constexpr std::uint8_t REG_LAT_TX_BCID_OFFSET       = 0x30;
    // for the special sectors, swapping is needed.
    constexpr std::uint8_t REG_FIBER_REMAP_SEL          = 0x2a;
    constexpr std::uint8_t REG_OFFSET_MODE_BCID         = 0x2b;
    constexpr std::uint8_t REG_OFFSET_MODE_CNT          = 0x2c;
    // determines the L1A
    constexpr std::uint8_t REG_LOCAL_BCID_OFFSET        = 0x2d;
    // determines which BCID of the window to return for segment(also determines latency)
    constexpr std::uint8_t REG_segmentBcidAdj           = 0x36;
    // should not contain Write-only, because attempting to read Write-Only register will fail.
    constexpr auto REGS = std::to_array({
      REG_ADDC_EMU_DISABLE,
      REG_FIBER_ALIGNMENT,
      REG_FIBER_QPLL_RESET,
      REG_FIBER_BCIDS_00_07,
      REG_FIBER_BCIDS_08_15,
      REG_FIBER_BCIDS_16_23,
      REG_FIBER_BCIDS_24_31,
      REG_FIBER_BC_OFFSET,
      REG_DATE_CODE,
      REG_GIT_HASH,
      REG_L1A_CONTROL,
      REG_LOCAL_BCID_OFFSET,
      REG_L1A_OPENING_OFFSET,
      REG_L1A_REQUEST_OFFSET,
      REG_L1A_CLOSING_OFFSET,
      REG_L1A_BUSY_THRESHOLD,
      REG_L1A_TIMEOUT_WINDOW,
      REG_TTC_ECR_CNT,
      REG_TTC_BCR_CNT,
      REG_TTC_L1A_CNT,
      REG_INPUT_PHASE,
      REG_INPUT_PHASEADDCOFFSET,
      REG_INPUT_PHASEL1DDCOFFSET,
      REG_FIBER_HOT_MUX,
      REG_FIBER_HOT_READ,
      REG_GBT_BCID_OK,
      // REG_FIBER_MASK_MUX,
      // REG_FIBER_MASK_WRITE,
      REG_PIPELINE_OVERFLOW,
      REG_SELFTRIGGER_DELAY,
      REG_VMM_MASK_HOT_THRESH,
      REG_VMM_MASK_HOT_THRESH_HYST,
      REG_VMM_MASK_DRAIN_PERIOD,
      REG_CHAN_RATE_ENABLE,
      REG_HORX_ENV_MON_DATA,
      REG_GLO_SYNC_IDLE_STATE,
      REG_GLO_SYNC_BCID_OFFSET,
      REG_LAT_TX_IDLE_STATE,
      REG_LAT_TX_BCID_OFFSET,
      REG_FIBER_REMAP_SEL,
      REG_OFFSET_MODE_BCID,
      REG_OFFSET_MODE_CNT,
    });
    constexpr std::array<std::pair<std::string_view, std::string_view>, NUM_ADDCS> ORDERED_ADDCS = {
      std::make_pair( "ADDC_L1P6_IPR", "L0/O" ),
      std::make_pair( "ADDC_L1P3_IPL", "L0/E" ),
      std::make_pair( "ADDC_L1P3_IPR", "L1/E" ),
      std::make_pair( "ADDC_L1P6_IPL", "L1/O" ),
      std::make_pair( "ADDC_L4P6_IPR", "L2/O" ),
      std::make_pair( "ADDC_L4P3_IPL", "L2/E" ),
      std::make_pair( "ADDC_L4P3_IPR", "L3/E" ),
      std::make_pair( "ADDC_L4P6_IPL", "L3/O" ),
      std::make_pair( "ADDC_L4P6_HOR", "L4/O" ),
      std::make_pair( "ADDC_L4P3_HOL", "L4/E" ),
      std::make_pair( "ADDC_L4P3_HOR", "L5/E" ),
      std::make_pair( "ADDC_L4P6_HOL", "L5/O" ),
      std::make_pair( "ADDC_L1P6_HOR", "L6/O" ),
      std::make_pair( "ADDC_L1P3_HOL", "L6/E" ),
      std::make_pair( "ADDC_L1P3_HOR", "L7/E" ),
      std::make_pair( "ADDC_L1P6_HOL", "L7/O" ),
    };
    constexpr std::uint32_t ADDC_EMU_DISABLE  = 0x01;
    constexpr std::uint32_t L1A_RESET_ENABLE  = 0xFF;
    constexpr std::uint32_t L1A_RESET_DISABLE = 0x00;
    constexpr std::uint32_t FIBER_QPLL_RESET_DISABLE = 0x00;
  }

}  // namespace nsw

#endif  // NSWCONFIGURATION_TPCONSTANTS_H_
