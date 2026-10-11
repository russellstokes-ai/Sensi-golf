#pragma once
#include <cstdint>

namespace sensigolf::recovered {

// Original Windows v1.014 0x409535 course bounds plus the top-level
// 0x40A318..0x40A34C negative-AX branch. This is the separate WORLD
// boundary path, not descriptor #22 ("OUT OF BOUNDS"), which resolves
// to regular numeric landing code 35 and is handled elsewhere.
// Does not model original 0x40CD00's downstream presentation/turn logic.
struct OriginalWorldBoundaryInput {
    std::int32_t ball_x = 0;          // 16.16 source ball coordinates
    std::int32_t ball_y = 0;
    std::int32_t x_extent = 0;        // original width*16 - 0x100
    std::int32_t y_extent = 0;        // original height*8 - 0xD0
    std::uint16_t player_flags = 0;
    std::uint16_t strokes_on_hole = 0;// [ESI+0x52]
    std::uint16_t strokes_total = 0;  // [ESI+0x56]
    std::uint16_t ball_pause = 0;     // [EDI+0x1C]
    std::uint16_t game_gate = 0;      // [0x41F883]
};
struct OriginalWorldBoundaryResult {
    bool outside_world = false;
    std::int16_t source_map_ax = 0; // original 0x409535: -1 OOB, 0 valid
    std::uint16_t player_flags = 0;
    std::uint16_t strokes_on_hole = 0;
    std::uint16_t strokes_total = 0;
    std::uint16_t ball_pause = 0;
    std::uint16_t game_gate = 0;
    bool requires_out_of_bounds_dispatch = false; // 0x40CD00
};

// Applies ONLY source 0x40A318 boundary branch when green mode is false;
// original green-map exit 0x40A351 takes a different path.
OriginalWorldBoundaryResult original_world_boundary_check(
    const OriginalWorldBoundaryInput& input) noexcept;

} // namespace sensigolf::recovered
