#pragma once

#include <cstdint>
#include "sensigolf/recovered_flight.hpp"

namespace sensigolf::recovered {

struct GroundStepResult {
    bool contacted_ground = false;
    bool resting = false;
    bool hazard_stop = false;
    bool holed = false;
    std::int32_t contact_x = 0;
    std::int32_t contact_y = 0;
};

// Execute the recovered landing/ground branch for a controlled terrain
// descriptor landing code. Code 0 is the neutral oracle surface; codes 2..7
// are ordinary named lies. Code 8 is the original terminal hole/cup branch.
// Code 0x23 (35) is the immediate-stop hazard branch used by
// WATER/NO GO/OUT OF BOUNDS descriptors.
GroundStepResult step_controlled_surface(
    FlightState& state,
    std::uint16_t landing_code);

// Original club-12 flat-green path: no gravity, green drag (0x780),
// normal Q14 planar movement, and V/H are cleared when the putt stops.
struct GreenSlopeAdjustment {
    std::int32_t raw_x = 0;
    std::int32_t raw_y = 0;
};

GreenSlopeAdjustment green_slope_adjustment(
    std::uint16_t slope_direction,
    std::uint16_t slope_magnitude);

GroundStepResult step_green_putt(
    FlightState& state,
    std::uint16_t slope_direction,
    std::uint16_t slope_magnitude);

// Original club-12 path on ordinary non-green terrain codes below 8.
// It bypasses gravity like a putt but uses full 0xF00 drag because green mode
// is clear. No green slope adjustment is applied.
GroundStepResult step_non_green_putt(FlightState& state);

inline GroundStepResult step_flat_green_putt(FlightState& state) {
    return step_green_putt(state, 0, 0);
}

inline GroundStepResult step_generic_flat_surface(FlightState& state) {
    return step_controlled_surface(state, 0);
}

} // namespace sensigolf::recovered
