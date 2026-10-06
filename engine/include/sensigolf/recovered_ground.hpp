#pragma once

#include <cstdint>
#include "sensigolf/recovered_flight.hpp"

namespace sensigolf::recovered {

struct GroundStepResult {
    bool contacted_ground = false;
    bool resting = false;
    std::int32_t contact_x = 0;
    std::int32_t contact_y = 0;
};

GroundStepResult step_generic_flat_surface(FlightState& state);

} // namespace sensigolf::recovered
