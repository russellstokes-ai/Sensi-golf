#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/classic_shot_model.hpp"

using namespace sensigolf;

namespace {

void run_until_complete(ClassicShotModel& model, int limit = 1024) {
    int ticks = 0;
    while (model.shot_active() && ticks++ < limit) {
        model.step();
    }
    assert(!model.shot_active());
    assert(model.state().phase == ShotPhase::Complete);
}

} // namespace

int main() {
    ClassicShotModel model;
    model.reset();

    assert(model.tick_rate_hz() == 70);
    assert(model.tick_interval_16_16() == 0x03A8);
    assert(!model.shot_active());

    // Straight 1W neutral surface: same recovered inputs as Gate-1 golden master.
    model.set_surface_context({0, 0, 0});
    model.set_ball_origin(0, 0);
    ShotInput straight{};
    straight.club_index = 0;
    straight.lie_index = 0;
    straight.power_tick = 105;
    straight.accuracy_tick = 63;
    straight.aim_raw = 0;
    model.begin_shot(straight);
    assert(model.shot_active());
    assert(model.state().phase == ShotPhase::Airborne);
    run_until_complete(model);
    assert(model.state().tick == 152);
    assert(model.state().x_raw == 0);
    assert(model.state().y_raw == 26251460);

    // A flat putt uses the recovered club-12 green path.
    model.reset();
    model.set_surface_context({1, 0, 0});
    ShotInput putt{};
    putt.club_index = 12;
    putt.lie_index = 6;
    putt.power_tick = 30;
    putt.accuracy_tick = 63;
    putt.aim_raw = 0;
    model.begin_shot(putt);
    assert(model.state().phase == ShotPhase::GroundRoll);
    run_until_complete(model);
    assert(model.state().z_raw == 0);

    // Hazard termination is surfaced to the platform-neutral game layer.
    model.reset();
    model.set_surface_context({0x23, 0, 0});
    ShotInput hazard{};
    hazard.club_index = 5;
    hazard.lie_index = 7;
    hazard.power_tick = 83;
    hazard.accuracy_tick = 63;
    hazard.aim_raw = 777;
    model.begin_shot(hazard);
    run_until_complete(model);
    assert(model.state().hazard);

    // Runtime game state cannot change the terrain contract mid-shot.
    model.reset();
    model.set_surface_context({0, 0, 0});
    model.begin_shot(straight);
    bool threw = false;
    try {
        model.set_surface_context({1, 0, 0});
    } catch (const std::logic_error&) {
        threw = true;
    }
    assert(threw);

    return 0;
}
