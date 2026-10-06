#pragma once

#include <cstdint>
#include "sensigolf/recovered_flight.hpp"

namespace sensigolf::recovered {

struct GroundStepResult {
    bool contacted_ground = false;
    bool resting = false;
    bool hazard_stop = false;
    std::int32_t contact_x = 0;
    std::int32_t contact_y = 0;
};

// Execute the recovered landing/ground branch for a controlled terrain
// descriptor landing code. Code 0 is the neutral oracle surface; codes 2..7
// are ordinary named lies. Code 0x23 (35) is the original immediate-stop
// hazard branch used by WATER/NO GO/OUT OF BOUNDS descriptors.
GroundStepResult step_controlled_surface(
    FlightState& state,
    std::uint16_t landing_code);

inline GroundStepResult step_generic_flat_surface(FlightState& state) {
    return step_controlled_surface(state, 0);
}

} // namespace sensigolf::recovered
