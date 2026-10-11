#include <cassert>
#include <cstddef>
#include <cstdint>
#include "pixtee_course_adapter.hpp"
#include "sensigolf/classic_hole_session.hpp"
#include "sensigolf/classic_mobile_controls.hpp"
int main() {
    auto course = pixtee::make_lakewood_hole_1();
    assert(course.player_start(0).x == 150);
    assert(course.player_start(0).y == 458);
    assert(course.hole_position().x == 142);
    assert(course.hole_position().y == 82);
    assert(course.resolve_integer_position(235, 250).landing_code == 35);
    assert(course.resolve_integer_position(91, 141).landing_code == 7);
    assert(course.resolve_integer_position(150, 390).landing_code == 3);
    sensigolf::ClassicHoleSession hole(course, std::size_t{0},
        sensigolf::ClassicHoleMetadata{0,4,0});
    sensigolf::ClassicMobileControls controls;
    controls.sync_hole(hole);
    assert(controls.enabled());
    controls.set_aim_raw(2048); // Aim north toward Lakewood green (0 aims south in recovered heading)
    controls.set_club(0);
    controls.meter_click(0);
    controls.meter_click(105);
    controls.meter_click(63);
    assert(controls.dispatch_to(hole));
    assert(hole.strokes() == 1);
    assert(hole.phase() == sensigolf::HoleSessionPhase::ShotActive);
    int ticks = 0;
    while (hole.phase() == sensigolf::HoleSessionPhase::ShotActive &&
           ticks++ < 2048) hole.step();
    assert(ticks < 2048);
    assert(hole.ball_state().tick > 0);
    return 0;
}
