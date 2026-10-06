#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sensigolf/recovered_interactions.hpp"

using namespace sensigolf::recovered;

namespace {
long parse(const char* s) {
    char* end = nullptr;
    const long v = std::strtol(s, &end, 0);
    if (!end || *end != '\0') throw std::invalid_argument("invalid integer");
    return v;
}

void emit(const char* branch, const FlightState& s, const OriginalPrng16& rng, bool marker) {
    std::cout
        << "{\"branch\":\"" << branch << "\""
        << ",\"direction\":" << s.direction
        << ",\"vertical_force\":" << s.vertical_force
        << ",\"horizontal_force\":" << s.horizontal_force
        << ",\"seed0\":" << rng.seed0
        << ",\"seed1\":" << rng.seed1
        << ",\"marker\":" << (marker ? "true" : "false")
        << "}\n";
}
}

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "usage: recovered_interactions_probe <branch> <seed0> <seed1> <direction> <V> <H>\n";
        return 2;
    }
    try {
        const std::string branch = argv[1];
        OriginalPrng16 rng{
            static_cast<std::uint16_t>(parse(argv[2])),
            static_cast<std::uint16_t>(parse(argv[3]))
        };
        FlightState s{};
        s.direction = static_cast<std::uint16_t>(parse(argv[4])) & kDirectionMask;
        s.vertical_force = static_cast<std::int32_t>(parse(argv[5]));
        s.horizontal_force = static_cast<std::int32_t>(parse(argv[6]));
        bool marker = false;

        if (branch == "code9") {
            apply_code9_low_height_deflection(s, rng);
        } else if (branch == "lip") {
            marker = apply_near_hole_lip_deflection(s, rng);
        } else if (branch == "flag") {
            apply_flag_coordinate_deflection(s, rng);
        } else {
            throw std::invalid_argument("unknown branch");
        }
        emit(branch.c_str(), s, rng, marker);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
