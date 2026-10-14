#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/classic_shot_model.hpp"
#include "sensigolf/recovered_ground.hpp"

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

    // Owner's Pixtee soft-ground mode: first contact removes upward bounce,
    // then native ground drag rolls to rest. Original mode above is unchanged.
    for(const std::uint16_t code:{std::uint16_t(5),std::uint16_t(6),std::uint16_t(7)}) {
        ClassicShotModel soft;soft.reset();soft.set_surface_context({code,0,0});
        soft.set_soft_ground_enabled(true);soft.begin_shot(straight);
        bool landed=false;
        for(int tick=0;tick<1024 && soft.shot_active();++tick) {
            soft.step();
            if(soft.presentation_contact().impact)landed=true;
            if(landed)assert(soft.state().z_raw==0 && soft.state().vz_raw==0);
        }
        assert(landed && !soft.shot_active());
        soft.begin_shot(straight);soft.step();assert(soft.state().z_raw>0);
        run_until_complete(soft);
    }
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

    // Original putter drag/slope follows the scene mode, not the descriptor.
    for(const bool green_mode:{false,true}) {
        for(const std::uint16_t code:{std::uint16_t(1),std::uint16_t(2)}) {
            model.reset();model.set_surface_context({code,256,1});
            ShotInput putt{};putt.club_index=12;putt.lie_index=code==1?6:5;
            putt.power_tick=30;putt.accuracy_tick=63;
            model.begin_shot(putt);
            model.set_original_interaction_context(nullptr,green_mode,0,0);
            recovered::LaunchInput launch{};launch.club_index=12;
            launch.lie_index=putt.lie_index;launch.captured_power=30;launch.accuracy_tick=63;
            auto expected=recovered::launch_normal_shot(launch);
            if(green_mode)recovered::step_green_putt(expected,256,1);
            else recovered::step_non_green_putt(expected);
            model.step();
            assert(model.state().x_raw==expected.x && model.state().y_raw==expected.y);
        }
    }

    // Original club-12 SKIRT/code-2 path uses full drag with green
    // mode clear. Exact v1.014 oracle: profile slot 5, power 10 -> tick 7,
    // y=60672 raw.
    model.reset();
    model.set_surface_context({2, 0, 0});
    ShotInput fringe_putt{};
    fringe_putt.club_index = 12;
    fringe_putt.lie_index = 5;
    fringe_putt.power_tick = 10;
    fringe_putt.accuracy_tick = 63;
    fringe_putt.aim_raw = 0;
    model.begin_shot(fringe_putt);
    run_until_complete(model);
    assert(model.state().tick == 7);
    assert(model.state().x_raw == 0);
    assert(model.state().y_raw == 60672);
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

    // Source-recovered flag impact triggers only at the exact integer
    // flag coordinate and its original height threshold, with original PRNG.
    ClassicShotModel flag_hit;
    ClassicShotModel flag_miss;
    recovered::OriginalPrng16 impact_rng{0x1234,0xABCD};
    recovered::OriginalPrng16 miss_rng{0x1234,0xABCD};
    flag_hit.reset();
    flag_miss.reset();
    flag_hit.set_surface_context({3,0,0});
    flag_miss.set_surface_context({3,0,0});
    flag_hit.set_ball_origin(0,0);
    flag_miss.set_ball_origin(0,0);
    flag_hit.set_original_interaction_context(&impact_rng,false,0,0,true);
    flag_miss.set_original_interaction_context(&miss_rng,false,20,20,true);
    flag_hit.begin_shot(straight);
    flag_miss.begin_shot(straight);
    flag_hit.step();
    flag_miss.step();
    assert(impact_rng.seed0!=miss_rng.seed0 ||
           impact_rng.seed1!=miss_rng.seed1);
    assert(flag_hit.state().x_raw!=flag_miss.state().x_raw ||
           flag_hit.state().y_raw!=flag_miss.state().y_raw);
    // Original 0x40A456 high-speed lip: a zero-distance source
    // human-mode shot redirects velocity and consumes precisely one
    // original PRNG state. A CPU/source mode>=4 must not fake a lip.
    ClassicShotModel lip,later;
    recovered::OriginalPrng16 lip_rng{0x2345,0x6789};
    recovered::OriginalPrng16 idle_rng{0x2345,0x6789};
    lip.set_surface_context({3,0,0});
    later.set_surface_context({3,0,0});
    lip.set_ball_origin(0,0);
    later.set_ball_origin(0,0);
    lip.set_original_interaction_context(&lip_rng,false,120,120,
                                          false,true,0);
    later.set_original_interaction_context(&idle_rng,false,120,120,
                                            false,true,4);
    lip.begin_shot(straight);
    later.begin_shot(straight);
    lip.step();
    later.step();
    assert(lip.original_lip_deflection_last_tick());
    assert(!later.original_lip_deflection_last_tick());
    assert(lip_rng.seed0!=idle_rng.seed0 ||
           lip_rng.seed1!=idle_rng.seed1);
    assert(lip.state().vx_raw!=later.state().vx_raw ||
           lip.state().vy_raw!=later.state().vy_raw ||
           lip.state().vz_raw!=later.state().vz_raw);
    return 0;
}
