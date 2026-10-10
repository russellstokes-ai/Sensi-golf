#pragma once

#include <cstdint>

#include "sensigolf/recovered_flight.hpp"
#include "sensigolf/recovered_prng.hpp"

namespace sensigolf::recovered {

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
