#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_interactions.hpp"

using namespace sensigolf::recovered;

int main() {
    // Source-mapped branch boundaries, including special codes 9 and 10:
    assert(original_code9_contact(9,12,0,0) == Code9Contact::SpecialStop);
    assert(original_code9_contact(9,0,-1,0x2ffff) == Code9Contact::SpecialStop);
    assert(original_code9_contact(9,0,-1,0x30000) == Code9Contact::Deflect);
    assert(original_code9_contact(9,0,-1,0x140000) == Code9Contact::Deflect);
    assert(original_code9_contact(9,0,-1,0x140001) == Code9Contact::None);
    assert(original_code9_contact(9,0,0,0x30000) == Code9Contact::None);
    assert(original_code9_contact(10,12,0,0) == Code9Contact::None);
    FlightState position{};
    position.x=20<<16;
    position.y=31<<16;
    position.height=2<<16;
    assert(original_flag_coordinate_hit(position,20,30,false));
    assert(!original_flag_coordinate_hit(position,20,30,true));
    position.height=0xA0001;
    assert(!original_flag_coordinate_hit(position,20,30,false));
    position.height=0;
    position.horizontal_force=0xD000;
    assert(original_near_cup_lip_contact(true,0,position));
    assert(!original_near_cup_lip_contact(true,4,position));
    assert(!original_near_cup_lip_contact(false,0,position));
    position.vertical_force=1;
    assert(original_near_cup_lip_contact(true,0,position));

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
