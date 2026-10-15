#pragma once
#include <cstdint>
#include "sensigolf/recovered_flight.hpp"
namespace sensigolf {
// Authored Pixtee extension; zero wind bypasses every operation exactly.
struct AuthoredWindState {
    bool enabled=false;
    std::uint16_t toward_raw=1536;
    std::uint16_t speed_mph_x10=0;
    std::uint16_t response=96;
};
struct AuthoredWindAccumulator {
    std::int64_t fraction_x=0,fraction_y=0;
    std::int32_t residual_x=0,residual_y=0;
    std::uint8_t coordinate_scale=1;
};
void validate_authored_wind(const AuthoredWindState& wind);
void apply_authored_wind(recovered::FlightState& flight,const AuthoredWindState& wind,
                        AuthoredWindAccumulator& accumulator,bool green_mode);
}
