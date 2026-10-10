#pragma once

#include <cstdint>

namespace sensigolf::recovered {

// Original Windows v1.014 16-bit PRNG state used by routine 0x406FFA.
// The ranged entry advances seed0, conditionally rotates seed1, then returns
// the high word of seed0 * (max_inclusive + 1). A max of 0xFFFF returns
// the advanced raw seed because the 16-bit increment wraps to zero.
struct OriginalPrng16 {
    std::uint16_t seed0 = 0;
    std::uint16_t seed1 = 0;

    std::uint16_t next(std::uint16_t max_inclusive) noexcept;
};

} // namespace sensigolf::recovered
