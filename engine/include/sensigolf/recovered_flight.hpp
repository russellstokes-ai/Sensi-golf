#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sensigolf::recovered {

constexpr std::uint16_t kDirectionMask = 0x0FFF;
constexpr std::int32_t kMaxCapturedPower = 105;
constexpr std::int32_t kAccuracyCenter = 63;
constexpr std::int32_t kGravityPerTick = 0x2100;
constexpr std::int32_t kHorizontalDragPerTick = 0x0F00;

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
    std::int32_t adjusted_power = 0;
};

struct LaunchInput {
    std::uint16_t club_index = 0;
    std::uint16_t lie_index = 0;
    std::int32_t captured_power = 0;
    std::int32_t accuracy_tick = kAccuracyCenter;
    std::uint16_t player_direction = 0;
    std::int32_t start_x = 0;
    std::int32_t start_y = 0;
};

// Live player launch path at 0x40AD1D in Windows v1.014.
//
// For accuracy values inside the selected profile bounds:
//   error = accuracy_tick - 63
//   direction = (player_direction - error*16) & 0xFFF
//   adjusted_power = max(0, captured_power - abs(error))
//   swing_adjuster = selected_profile[evenized(error)]
//   V = club.vertical_base + club.power_scale * adjusted_power
//   H = club.horizontal_base + club.power_scale * adjusted_power
//
// Out-of-profile-bound accuracy behavior remains intentionally unimplemented
// until its surrounding state semantics are parity-tested.
FlightState launch_normal_shot(const LaunchInput& input);

enum class AirborneStepResult : std::uint8_t {
    Airborne,
    NeedsLandingResolution,
};

AirborneStepResult step_clear_air(FlightState& state);

} // namespace sensigolf::recovered
