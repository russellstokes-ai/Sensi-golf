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

// The 13 records loaded by the original function at 0x40A1C1.
// The first two source dwords are shifted right by one before use; these are
// the post-load values actually consumed by shot initialization.
const std::array<ClubLaunchParameters, 13>& club_launch_parameters();

// Exact mathematical regeneration of the original 4096-step Q14 sine table.
// Binary analysis verified every source-table value against this formula:
// trunc(sin(2*pi*i/4096) * 16384), with the negative peak clamped at -16383.
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
    std::int32_t accuracy_error = 0;
    std::int32_t swing_adjuster = 0;
    std::uint16_t player_direction = 0;
    std::int32_t start_x = 0;
    std::int32_t start_y = 0;
};

// Reproduce the normal (non-putter) launch initializer at 0x40AD1D..0x40ADDF.
//
// power_after_accuracy = max(0, captured_power - abs(accuracy_error))
// direction = (player_direction - accuracy_error * 16) & 0xFFF
// vForce = club.vertical_base + club.power_scale * power_after_accuracy
// hForce = club.horizontal_base + club.power_scale * power_after_accuracy
FlightState launch_normal_shot(const LaunchInput& input);

enum class AirborneStepResult : std::uint8_t {
    Airborne,
    NeedsLandingResolution,
};

// Advance one recovered *clear-air* tick.
//
// This intentionally stops when height would reach the ground. Bounce,
// terrain response and roll are handled by code paths that are still being
// recovered and must not be approximated here.
AirborneStepResult step_clear_air(FlightState& state);

} // namespace sensigolf::recovered
