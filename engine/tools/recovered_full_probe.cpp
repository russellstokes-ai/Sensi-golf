#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "sensigolf/recovered_flight.hpp"
#include "sensigolf/recovered_ground.hpp"

using namespace sensigolf::recovered;

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
        << ",\"swing_adjuster\":" << s.swing_adjuster
        << ",\"adjusted_power\":" << s.adjusted_power
        << "}";
}

struct PointEvent {
    int tick = -1;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

} // namespace

int main(int argc, char** argv) {
    if (argc != 7 && argc != 8 && argc != 10) {
        std::cerr
            << "usage: recovered_full_probe "
            << "<club> <lie> <power> <accuracy_tick> <direction> <max_ticks> "
            << "[surface_code [slope_direction slope_magnitude]]\n";
        return 2;
    }

    try {
        LaunchInput input{};
        input.club_index =
            static_cast<std::uint16_t>(parse_long(argv[1], "club"));
        input.lie_index =
            static_cast<std::uint16_t>(parse_long(argv[2], "lie"));
        input.captured_power =
            static_cast<std::int32_t>(parse_long(argv[3], "power"));
        input.accuracy_tick =
            static_cast<std::int32_t>(parse_long(argv[4], "accuracy_tick"));
        input.player_direction =
            static_cast<std::uint16_t>(parse_long(argv[5], "direction"));
        const int max_ticks =
            static_cast<int>(parse_long(argv[6], "max_ticks"));
        if (max_ticks <= 0) {
            throw std::invalid_argument("max_ticks must be positive");
        }
        const long parsed_surface =
            argc == 8 ? parse_long(argv[7], "surface_code") : 0;
        if (parsed_surface < 0 || parsed_surface > 0xFFFF) {
            throw std::invalid_argument("surface_code outside uint16 range");
        }
        const auto surface_code = static_cast<std::uint16_t>(parsed_surface);
        const auto slope_direction = argc == 10
            ? static_cast<std::uint16_t>(parse_long(argv[8], "slope_direction"))
            : 0;
        const auto slope_magnitude = argc == 10
            ? static_cast<std::uint16_t>(parse_long(argv[9], "slope_magnitude"))
            : 0;

        auto state = launch_normal_shot(input);
        std::optional<PointEvent> landing;
        std::optional<PointEvent> rest;

        if (input.club_index == 12) {
            if (surface_code != 1) {
                throw std::invalid_argument(
                    "club 12 recovered full probe currently scopes flat GREEN H4 code 1");
            }
            landing = PointEvent{0, state.x, state.y};
        }

        std::cout << "{\n  \"surface_code\":" << surface_code
                  << ",\n  \"samples\":[\n";
        sample_json(state, 0, false);

        for (int tick = 1; tick <= max_ticks; ++tick) {
            const auto step = input.club_index == 12
                ? step_green_putt(state, slope_direction, slope_magnitude)
                : step_controlled_surface(state, surface_code);
            if (step.contacted_ground && !landing) {
                landing = PointEvent{tick, step.contact_x, step.contact_y};
            }
            sample_json(state, tick, true);
            if (step.resting) {
                rest = PointEvent{tick, state.x, state.y};
                break;
            }
        }

        if (!landing || !rest) {
            throw std::runtime_error(
                "controlled-surface probe did not reach landing/rest within max_ticks");
        }

        std::cout
            << "\n  ],\n  \"events\":{"
            << "\"landing\":{\"tick\":" << landing->tick
            << ",\"x\":" << landing->x
            << ",\"y\":" << landing->y << "},"
            << "\"rest\":{\"tick\":" << rest->tick
            << ",\"x\":" << rest->x
            << ",\"y\":" << rest->y << "}"
            << "}\n}\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
