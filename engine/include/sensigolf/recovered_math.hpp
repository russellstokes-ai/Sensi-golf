#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sensigolf::recovered {

inline constexpr std::uint32_t kDirectionMask = 0x0FFFu;
inline constexpr std::uint16_t kDropPowerMax = 0x0069u;
inline constexpr std::int32_t kGravityPerTick = 0x2100;
inline constexpr std::int32_t kNormalRollDrag = 0x0F00;
inline constexpr std::int32_t kGreenRollDrag = 0x0780;

struct ClubPhysics {
    std::int32_t raw_vertical_base;
    std::int32_t raw_horizontal_base;
    std::int32_t power_scale;

    constexpr std::int32_t loaded_vertical_base() const noexcept {
        return raw_vertical_base / 2;
    }

    constexpr std::int32_t loaded_horizontal_base() const noexcept {
        return raw_horizontal_base / 2;
    }
};

inline constexpr std::array<ClubPhysics, 13> kClubPhysics = {{
    {  16384, 163840, 3168},
    {  32768, 147456, 3120},
    {  73728, 131072, 3120},
    {  98304,  98304, 3072},
    { 131072,  98304, 2944},
    { 143360,  98304, 2880},
    { 155648,  98304, 2560},
    { 163840,  98304, 2496},
    { 225280,  96256, 2464},
    { 229376,  96256, 2368},
    { 262144,  94208, 2176},
    { 262144,  81920, 2112},
    {      0,  16384, 1536},
}};

inline constexpr std::size_t kPutterClubIndex = 12;

struct RecoveredBallState {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::uint32_t direction = 0;
    std::int32_t height = 0;
    std::int32_t distance_to_hole = 0;
    std::int32_t pause = 0;
};

struct LaunchState {
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::uint32_t direction = 0;
};

std::int16_t sin_q14(std::uint32_t direction) noexcept;
std::int16_t cos_q14(std::uint32_t direction) noexcept;
std::uint32_t wrap_direction(std::int64_t direction) noexcept;

// Equivalent to the x86 signed multiply followed by a 14-bit arithmetic shift.
std::int32_t project_q14(std::int32_t force, std::int16_t trig_value) noexcept;
std::pair<std::int32_t, std::int32_t>
project_horizontal(std::int32_t force, std::uint32_t direction) noexcept;

// drop_power is the original raw Welly value. Normal gameplay caps it at 105;
// this low-level function deliberately does not clamp injected analysis values.
LaunchState make_launch_state(
    std::size_t club_index,
    std::uint16_t drop_power,
    std::uint32_t direction,
    std::int32_t swing_adjuster) noexcept;

void apply_airborne_vertical_tick(RecoveredBallState& ball) noexcept;
void apply_roll_drag(RecoveredBallState& ball, bool green_mode) noexcept;
void apply_bounce(RecoveredBallState& ball) noexcept;
void apply_horizontal_tick(RecoveredBallState& ball) noexcept;

} // namespace sensigolf::recovered
