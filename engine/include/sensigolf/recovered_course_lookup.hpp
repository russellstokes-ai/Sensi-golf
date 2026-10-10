#pragma once

#include <cstddef>
#include <cstdint>

namespace sensigolf::recovered {

struct CourseLookupResult {
    std::uint16_t descriptor_index = 0;
    std::uint16_t slope_direction = 0;
    std::uint16_t slope_magnitude = 0;
    std::uint16_t raw_word = 0;
};

// Clean-room reproduction of the v1.014 MAPI sub-cell lookup at 0x409535.
//
// Each MAPI pair contains 640 eight-byte tile records. The selector/mask bank
// chooses one of four big-endian 16-bit words in the descriptor bank for a
// 2x2 map sub-cell. The selected word encodes:
//   low byte  = terrain descriptor index (>= 0x4D clamps to descriptor 4)
//   high byte = slope direction nibble + slope magnitude nibble
//
// x_subcell is 0..7 and y_subcell is 0..3.
CourseLookupResult lookup_course_subcell(
    const std::uint8_t* descriptor_bank,
    std::size_t descriptor_size,
    const std::uint8_t* selector_bank,
    std::size_t selector_size,
    std::uint16_t tile_index,
    std::uint8_t x_subcell,
    std::uint8_t y_subcell);

} // namespace sensigolf::recovered
