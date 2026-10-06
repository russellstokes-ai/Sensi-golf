#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sensigolf::recovered {

constexpr std::uint16_t kDirectionMask = 0x0FFF;
constexpr std::int32_t kMaxCapturedPower = 105;
constexpr std::int32_t kGravityPerTick = 0x2100;
constexpr std::int32_t kHorizontalDragPerTick = 0x0F00;
constexpr std::int32_t kTrigScale = 1 << 14;

struct ClubLaunchParameters {
    std::int32_t vertical_base;
    std::int32_t horizontal_base;
    std::int32_t power_scale;
    bool special_putter_path;
};

const std::array<ClubLaunchParameters, 13>& club_launch_parameters();
std::int16_t trig_q14(std::uint16_t index);

struct FlightState {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t height = 0;
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::uint16_t direction = 0;
    std::int32_t swing_adjuster = 0;
};

struct LaunchInput {
    std::uint16_t club_index = 0;
    std::int32_t captured_power = 0;
    std::int32_t swing_adjuster = 0;
    std::uint16_t player_direction = 0;
    std::int32_t start_x = 0;
    std::int32_t start_y = 0;
};

// Live player launch path at 0x40C84F in Windows v1.014:
// direction = player.direction
// scaled = club.power_scale * DropPower
// V = club.vertical_base + scaled
// H = club.horizontal_base + scaled
FlightState launch_normal_shot(const LaunchInput& input);

enum class AirborneStepResult : std::uint8_t {
    Airborne,
    NeedsLandingResolution,
};

// Normal clear-air player path at 0x40A603 / 0x40A96F.
AirborneStepResult step_clear_air(FlightState& state);

} // namespace sensigolf::recovered
