#ifndef NSWCONFIGURATION_LITERALS_H
#define NSWCONFIGURATION_LITERALS_H

#include <cstdint>

constexpr std::uint8_t operator"" _u8(const unsigned long long val)
{
  return static_cast<std::uint8_t>(val);
}

#endif