// NSW constants

#ifndef NSWCONFIGURATION_CONSTANTS_H_
#define NSWCONFIGURATION_CONSTANTS_H_

#include <cstddef>
#include <cstdint>

namespace nsw {
  /// Make it clear when we are using 8 as the size in bits of a byte
  constexpr std::size_t NUM_BITS_IN_BYTE = 8;

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

  constexpr std::size_t MIN_SECTOR_ID = 1;
  constexpr std::size_t MAX_SECTOR_ID = 16;

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
  }

}  // namespace nsw

#endif  // NSWCONFIGURATION_CONSTANTS_H_
