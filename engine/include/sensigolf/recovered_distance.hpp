#pragma once

#include <cstdint>

namespace sensigolf::recovered {

// Exact portable form of the Windows v1.014 distance-to-hole helper at
// 0x40B998. Ball coordinates use the recovered 16.16 representation.
// In original green mode the raw coordinates are arithmetically halved before
// their integer component is compared with the SPT cup coordinates.
std::uint32_t distance_to_hole(
    std::int32_t x_raw,
    std::int32_t y_raw,
    std::uint16_t hole_x,
    std::uint16_t hole_y,
    bool green_mode) noexcept;

} // namespace sensigolf::recovered
