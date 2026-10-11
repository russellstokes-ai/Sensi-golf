#pragma once

#include <cstdint>

#include "sensigolf/recovered_flight.hpp"
#include "sensigolf/recovered_prng.hpp"

namespace sensigolf::recovered {

// Source-authenticated original event gates at 0x40A374..0x40A3E0,
// 0x40A456..0x40A4A8 and 0x40A50B..0x40A575.
// These boolean/enum predicates contain *no generated physics* and can
// therefore be tested separately against original machine-code branches.
enum class Code9Contact : std::uint8_t { None, Deflect, SpecialStop };
Code9Contact original_code9_contact(
    std::uint16_t landing_code, std::uint16_t club_index,
    std::int32_t vertical_force, std::int32_t ball_height_raw) noexcept;

// Original source compares both values against the high WORD of ball X,
// not ball Y: flag X == Xhi and flag Y == Xhi - arithmetic(Hhi >> 1).
bool original_flag_coordinate_hit(
    const FlightState& state, std::int16_t flag_x,
    std::int16_t flag_y, bool on_green_map) noexcept;

// The original lip branch additionally depends on its original player state
// and precomputed zero-distance register; do not guess these inputs.
bool original_near_cup_lip_contact(
    bool cup_distance_zero, std::uint16_t player_state,
    const FlightState& state) noexcept;

// Exact state mutations used by three PRNG-driven branches in the original
// Windows v1.014 whole-shot tick routine (0x40A250). Branch activation is
// deliberately kept outside these helpers until the surrounding course/object
// semantics are fully classified.
void apply_code9_low_height_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept;

bool apply_near_hole_lip_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept;

void apply_flag_coordinate_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept;

} // namespace sensigolf::recovered
