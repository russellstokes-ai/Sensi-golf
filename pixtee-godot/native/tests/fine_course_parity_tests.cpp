#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include "pixtee_course_adapter.hpp"
#include "sensigolf/classic_terrain.hpp"
#include "sensigolf/classic_hole_session.hpp"
#include "sensigolf/classic_shot_model.hpp"

using namespace sensigolf;
int main() {
    const auto flat=pixtee::make_lakewood_hole_1();
    assert(flat.map_width()==19 && flat.map_height()==64);
    assert(flat.has_green_map());     // Native green coordinate transition.
    assert(flat.green_map_width()==16 && flat.green_map_height()==26);
    const auto region=flat.green_region();
    assert(region.has_value());
    assert(region->origin_x==96 && region->origin_y==40);

    // Uses actual recovered 2x2 subcells, not "one lie per 16x8 tile".
    assert(flat.resolve_integer_position(91,141).landing_code==7);  // sand
    assert(flat.resolve_integer_position(235,250).landing_code==35);// water
    assert(flat.resolve_integer_position(150,390).landing_code==3); // fairway
    assert(flat.resolve_integer_position(10,300).landing_code==5);  // rough
    assert(flat.resolve_integer_position(142,82).landing_code==1);  // green
    assert(flat.resolve_integer_position(0,0,true).landing_code==2);// skirt
    assert(flat.resolve_integer_position(92,84,true).landing_code==1);// green
    const auto initial=flat.resolve_integer_position(92,84,true);
    assert(initial.slope_direction==0 && initial.slope_magnitude==0);

    // Green-arrow direction and strength use the exact original MAPI nibble
    // encoding: 16 arrow directions x 16 magnitude values.
    for(int direction=0;direction<16;direction++) {
        for(int magnitude=0;magnitude<16;magnitude++) {
            const auto sloped=pixtee::make_lakewood_hole_1_with_slope(
                static_cast<std::uint8_t>(direction),
                static_cast<std::uint8_t>(magnitude));
            const auto tile=sloped.resolve_integer_position(92,84,true);
            assert(tile.landing_code==1);
            assert(tile.slope_direction==direction*256);
            assert(tile.slope_magnitude==magnitude);
        }
    }
    bool rejected=false;
    try {(void)pixtee::make_lakewood_hole_1_with_slope(16,0);}
    catch(const std::invalid_argument&) {rejected=true;}
    assert(rejected);

    // Original putting state machine sees exactly what MAPI/MAPS reported.
    auto slope=pixtee::make_lakewood_hole_1_with_slope(4,8);
    const auto tile=slope.resolve_integer_position(92,84,true);
    ClassicShotModel shot;
    shot.reset();
    shot.set_surface_context(ClassicSurfaceContext{
        tile.landing_code,tile.slope_direction,tile.slope_magnitude});
    ShotInput input{};
    input.club_index=12;
    input.lie_index=tile.profile_slot;
    input.power_tick=30;
    input.accuracy_tick=63;
    shot.begin_shot(input);
    int ticks=0;
    while(shot.shot_active() && ticks++<256) shot.step();
    assert(ticks<256);
    assert(shot.state().z_raw==0);

    // No original course payload, graphics or SFX are required to run the tests.
    return 0;
}
