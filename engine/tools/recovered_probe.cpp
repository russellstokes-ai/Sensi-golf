#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sensigolf/recovered_flight.hpp"

using sensigolf::recovered::AirborneStepResult;
using sensigolf::recovered::FlightState;
using sensigolf::recovered::LaunchInput;
using sensigolf::recovered::launch_normal_shot;
using sensigolf::recovered::step_clear_air;

namespace {

long parse_long(const char* s, const char* name) {
    char* end = nullptr;
    const long value = std::strtol(s, &end, 0);
    if (!end || *end != '\0') {
        throw std::invalid_argument(std::string("invalid ") + name);
    }
    return value;
}

void sample_json(const FlightState& s, int tick, bool comma) {
    if (comma) std::cout << ",\n";
    std::cout
        << "    {\"tick\":" << tick
        << ",\"x\":" << s.x
        << ",\"y\":" << s.y
        << ",\"height\":" << s.height
        << ",\"vertical_force\":" << s.vertical_force
        << ",\"horizontal_force\":" << s.horizontal_force
        << ",\"direction\":" << s.direction
        << "}";
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "usage: recovered_probe <club> <power> <direction> <swing_adjuster> <ticks>\n";
        return 2;
    }

    try {
        LaunchInput input{};
        input.club_index = static_cast<std::uint16_t>(parse_long(argv[1], "club"));
        input.captured_power = static_cast<std::int32_t>(parse_long(argv[2], "power"));
        input.player_direction = static_cast<std::uint16_t>(parse_long(argv[3], "direction"));
        input.swing_adjuster = static_cast<std::int32_t>(parse_long(argv[4], "swing_adjuster"));
        const int ticks = static_cast<int>(parse_long(argv[5], "ticks"));
        if (ticks < 0) throw std::invalid_argument("ticks must be non-negative");

        auto state = launch_normal_shot(input);

        std::cout << "{\n  \"samples\":[\n";
        sample_json(state, 0, false);
        for (int tick = 1; tick <= ticks; ++tick) {
            if (step_clear_air(state) != AirborneStepResult::Airborne) {
                throw std::runtime_error("probe reached ground before requested tick count");
            }
            sample_json(state, tick, true);
        }
        std::cout << "\n  ]\n}\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
