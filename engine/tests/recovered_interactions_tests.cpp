#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_interactions.hpp"

using namespace sensigolf::recovered;

int main() {
    // Authentic x86 0x40A686 branch predicate and signed WORD threshold:
    assert(original_terminal_stop_contact(35,200)==OriginalStopContact::Code35);
    assert(original_terminal_stop_contact(35,201)==OriginalStopContact::Other);
    assert(original_terminal_stop_contact(35,-32768)==OriginalStopContact::Code35);
    assert(original_terminal_stop_contact(50,-1)==OriginalStopContact::Code50);
    assert(original_terminal_stop_contact(50,201)==OriginalStopContact::Code50);
    assert(original_terminal_stop_contact(8,200)==OriginalStopContact::Other);
    // Source 0x40A7BD physical rebound depends on exact x86 arithmetic.
    {
        auto out=original_terminal_rebound(-0x2100,0x1000);
        assert(out.continues_flight);
        assert(out.vertical_force==0x1080);
        assert(out.horizontal_force==0x1840);
    }
    {
        auto out=original_terminal_rebound(0,0);
        assert(!out.continues_flight && out.vertical_force==0
               && out.horizontal_force==0);
    }
    {
        auto out=original_terminal_rebound(5,2);
        // NEG 5 => -5; SAR 1 => -3; SAR 1 => -2; 2+(-2)=0.
        assert(!out.continues_flight);
        assert(out.vertical_force==0 && out.horizontal_force==0);
    }
    {
        auto out=original_terminal_rebound(INT32_MIN,INT32_MAX);
        assert(out.continues_flight);
        assert(out.vertical_force==INT32_MIN/2);
        assert(out.horizontal_force==INT32_MAX+INT32_MIN/4);
    }
    {
        auto first=original_terminal_rebound_with_latch(0u,-0x2100,0x1000);
        assert(first.player_flags==1u && first.event_zero_requested);
        assert(first.rebound.continues_flight);
        auto repeat=original_terminal_rebound_with_latch(first.player_flags,
                                                          -0x2100,0x1000);
        assert(repeat.player_flags==1u && !repeat.event_zero_requested);
        assert(repeat.rebound.horizontal_force==first.rebound.horizontal_force);
        auto other=original_terminal_rebound_with_latch(0x8002u,5,2);
        assert(other.player_flags==0x8003u && other.event_zero_requested);
        assert(!other.rebound.continues_flight);
    }
    // Independent airborne high-code original entry (0x40A5D3).
    {
        auto high=original_airborne_high_code_contact(
            35,0,0,-8448,65000,201<<16);
        assert(high.handled && high.event_code==1 && high.continues_flight);
        auto low=original_airborne_high_code_contact(
            35,0,0,-8448,65000,200<<16);
        assert(low.handled && low.event_code==11 && !low.continues_flight);
        assert(low.player_state_flags==0x8000u);
        auto other=original_airborne_high_code_contact(
            60,0,4,-8448,65000,0);
        assert(other.handled && other.event_code==1);
        assert(other.player_state_flags==4);
        assert(!original_airborne_high_code_contact(10,0,0,-8448,65000,0).handled);
        assert(!original_airborne_high_code_contact(7,0,0,-8448,65000,0).handled);
    }
    // Source-mapped branch boundaries, including special codes 9 and 10:
    // Original numeric terrain 35 / 50 termination or rebound.
    {
        auto result=original_special_terminal_terrain(35,0,2,-32000,22000,200<<16);
        assert(result.handled && !result.continues_flight);
        assert(result.event_code==11 && result.player_state_flags==0x8002);
        assert(result.vertical_force==0 && result.horizontal_force==0
            && result.ball_height==0);
        assert(!original_special_terminal_terrain(
            35,0,0,-32000,22000,201<<16).handled);
        auto putting=original_special_terminal_terrain(50,12,2,-8448,50000,0);
        assert(putting.handled && !putting.continues_flight);
        assert(putting.event_code==1 && putting.player_state_flags==0x8002);
        assert(putting.vertical_force==-8448 && putting.horizontal_force==50000);
        auto wedge=original_special_terminal_terrain(50,3,2,-8448,50000,0);
        auto rebound=original_terminal_rebound(-8448,50000);
        assert(wedge.handled && wedge.continues_flight==rebound.continues_flight);
        assert(wedge.event_code==1 && wedge.player_state_flags==2);
        assert(wedge.vertical_force==rebound.vertical_force);
        assert(wedge.horizontal_force==rebound.horizontal_force);
        assert(!original_special_terminal_terrain(9,0,2,-8448,50000,0).handled);
    }

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
    assert(original_flag_coordinate_hit(position,20,19,false));
    assert(!original_flag_coordinate_hit(position,20,30,false)); // original uses X, not Y
    assert(!original_flag_coordinate_hit(position,20,19,true));
    position.height=0xA0001;
    assert(!original_flag_coordinate_hit(position,20,19,false));
    position.height=0;
    position.horizontal_force=0xD000;
    assert(original_near_cup_lip_contact(true,0,position));
    assert(!original_near_cup_lip_contact(true,4,position));
    assert(!original_near_cup_lip_contact(false,0,position));
    position.vertical_force=1;
    assert(original_near_cup_lip_contact(true,0,position));
    position.height=-1;
    assert(original_near_cup_lip_contact(true,0,position));
    assert(original_near_cup_lip_contact(true,65535,position)); // signed x86 JGE, 0xffff is -1 // original x86 CMP/JG has no lower bound

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
