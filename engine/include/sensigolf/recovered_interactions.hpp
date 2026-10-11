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

// Original x86 0x40A686..0x40A6AB: classify the already-entered
// terminal terrain branch. Code 35 and 50 names are ORIGINAL NUMERIC
// values, not proof that any particular object class or 3D collision
// geometry has been identified. Do not wire to live collision yet.
enum class OriginalStopContact : std::uint8_t {
    Other = 0, Code35 = 1, Code50 = 2
};
OriginalStopContact original_terminal_stop_contact(
    std::uint16_t landing_code, std::int16_t height_high_word) noexcept;

// Original v1.014 0x40A7BD..0x40A7E0: terminal-terrain rebound.
// Isolated signed 32-bit NEG/SAR/SAR/ADD/JG effects, no guessed tree mask.
struct OriginalTerminalRebound {
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    bool continues_flight = false;
};
OriginalTerminalRebound original_terminal_rebound(
    std::int32_t vertical_force, std::int32_t horizontal_force) noexcept;

// Original 0x40A79D..0x40A7BD controls whether the event-zero callback
// runs once per player flag bit 0, then applies the identical rebound.
// No sound, animation or terrain/object identity is assumed.
struct OriginalTerminalLatchRebound {
    OriginalTerminalRebound rebound{};
    std::uint16_t player_flags = 0;
    bool event_zero_requested = false;
};
OriginalTerminalLatchRebound original_terminal_rebound_with_latch(
    std::uint16_t player_flags,
    std::int32_t vertical_force, std::int32_t horizontal_force) noexcept;

// Source x86 0x40A686..0x40A7E0, restricted to verified numeric terrain
// code 35 with signed height_hi <= 200 or code 50. Not a tree detector.
// event_code records the authentic event CALL argument (1 or 11), NOT an
// implementation of the game's sound, effect sprite or host dispatcher.
struct OriginalSpecialTerminalTerrainResult {
    bool handled = false;
    bool continues_flight = false;
    std::uint16_t player_state_flags = 0;
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::int32_t ball_height = 0;
    std::uint8_t event_code = 0;
};
OriginalSpecialTerminalTerrainResult original_special_terminal_terrain(
    std::uint16_t landing_code, std::uint16_t club_index,
    std::uint16_t player_state_flags, std::int32_t vertical_force,
    std::int32_t horizontal_force, std::int32_t ball_height) noexcept;

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
