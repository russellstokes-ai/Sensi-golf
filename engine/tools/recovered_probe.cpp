#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sensigolf/recovered_flight.hpp"

using namespace sensigolf::recovered;

namespace {
long parse_long(const char* s, const char* name) {
    char* end = nullptr;
    const long value = std::strtol(s, &end, 0);
    if (!end || *end != '\0') throw std::invalid_argument(std::string("invalid ") + name);
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
        << ",\"swing_adjuster\":" << s.swing_adjuster
        << ",\"adjusted_power\":" << s.adjusted_power
        << "}";
}
}

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "usage: recovered_probe <club> <lie> <power> <accuracy_tick> <direction> <ticks>\n";
        return 2;
    }
    try {
        LaunchInput input{};
        input.club_index = static_cast<std::uint16_t>(parse_long(argv[1], "club"));
        input.lie_index = static_cast<std::uint16_t>(parse_long(argv[2], "lie"));
        input.captured_power = static_cast<std::int32_t>(parse_long(argv[3], "power"));
        input.accuracy_tick = static_cast<std::int32_t>(parse_long(argv[4], "accuracy_tick"));
        input.player_direction = static_cast<std::uint16_t>(parse_long(argv[5], "direction"));
        const int ticks = static_cast<int>(parse_long(argv[6], "ticks"));

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
