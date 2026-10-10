#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_interactions.hpp"

using namespace sensigolf::recovered;

int main() {
    {
        FlightState s{};
        s.direction = 777;
        OriginalPrng16 rng{0x1234, 0xABCD};
        apply_code9_low_height_deflection(s, rng);
        assert(s.direction == ((777 + 0x800 + 117 - 0x200) & 0xFFF));
        assert(rng.seed0 == 14957);
        assert(rng.seed1 == 54758);
    }

    {
        FlightState s{};
        s.direction = 3000;
        s.horizontal_force = 0x12000;
        OriginalPrng16 rng{0x1234, 0xABCD};
        const bool marker = apply_near_hole_lip_deflection(s, rng);
        assert(marker);
        assert(s.vertical_force == 0x12000);
        assert(s.horizontal_force == 0x9000);
        assert(s.direction <= kDirectionMask);
    }

    {
        FlightState s{};
        s.direction = 100;
        s.vertical_force = -0x2100;
        OriginalPrng16 rng{0x1234, 0xABCD};
        apply_flag_coordinate_deflection(s, rng);
        const std::uint32_t v = static_cast<std::uint32_t>(-0x2100);
        assert(static_cast<std::uint32_t>(s.vertical_force) == v + (v >> 2));
        assert(s.direction <= kDirectionMask);
    }

    return 0;
}
