#pragma once

#include <cstdint>

namespace sensigolf::recovered {

// Exact clean-room reproduction of Windows v1.014 helper 0x40B998.
//
// The original extracts integer course coordinates from the high word of the
// 16.16-style ball coordinates, optionally arithmetic-halves the raw position
// in green coordinate mode, computes integer Euclidean distance, then scales
// the integer root by 6/10.
std::uint32_t distance_to_hole(
    std::int32_t x_raw,
    std::int32_t y_raw,
    std::uint16_t hole_x,
    std::uint16_t hole_y,
    bool green_mode) noexcept;

} // namespace sensigolf::recovered
