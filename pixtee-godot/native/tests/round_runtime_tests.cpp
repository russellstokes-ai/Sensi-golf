#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include "pixtee_round_runtime.hpp"

namespace {
void be16(std::vector<std::uint8_t>& bytes,std::size_t at,std::uint16_t w) {
    bytes.at(at)=static_cast<std::uint8_t>(w>>8);
    bytes.at(at+1)=static_cast<std::uint8_t>(w);
}
sensigolf::ClassicCourseResources original_rule_fixture() {
    // Minimal authored fixture using exact recovered code8/putting behaviour;
    // never original commercial course graphics or MAPM resource bytes.
    constexpr int w=4,h=128;
    std::vector<std::uint8_t> map(0x60u+w*h*2u,0),spt(50,0);
    std::vector<std::uint8_t> descriptor(8,0),selector(8,0);
    be16(map,0x54,w);be16(map,0x56,h);
    be16(descriptor,0,7); // green code-8 (original terminal)
    return sensigolf::ClassicCourseResources(
        std::move(map),std::move(spt),std::move(descriptor),
        std::move(selector));
}
sensigolf::ClassicCourseResources away_from_cup_fixture() {
    // Independently authored test layout: ball at (0,0), cup at (0,64).
    // A rejected swing must not be accidentally scored by the next tick.
    constexpr int w=4,h=128;
    std::vector<std::uint8_t> map(0x60u+w*h*2u,0),spt(50,0);
    std::vector<std::uint8_t> descriptor(8,0),selector(8,0);
    be16(map,0x54,w);be16(map,0x56,h);
    be16(spt,46,64);
    be16(descriptor,0,7);
    return sensigolf::ClassicCourseResources(
        std::move(map),std::move(spt),std::move(descriptor),
        std::move(selector));
}
pixtee::PixteeRoundRuntime make_runtime() {
    std::array<std::uint8_t,18> order{};
    std::array<std::uint8_t,100> pars{};
    for(unsigned i=0;i<18;++i) {order[i]=static_cast<std::uint8_t>(i+1);pars[i+1]=4;}
    return pixtee::PixteeRoundRuntime(order,pars,0x1234,0xabcd);
}
void hit_cup(pixtee::PixteeRoundRuntime& rt) {
    rt.set_club(12);
    rt.meter_click_raw(0);
    rt.meter_click_raw(30);
    rt.meter_click_raw(63);
    bool live=false;
    for(int t=0;t<256;++t) {
        if(rt.hole().phase()==sensigolf::HoleSessionPhase::ShotActive) live=true;
        rt.step_tick();
        if(rt.hole().phase()==sensigolf::HoleSessionPhase::HoleScored) break;
    }
    assert(live);
    assert(rt.hole().phase()==sensigolf::HoleSessionPhase::HoleScored);
}
} // namespace
int main() {
    // Actual original x86 counter is the authority for Welly inputs.
    // Rejected/timeout samples do not manufacture strokes or modify rounds.
    {
        auto timed=make_runtime();
        auto fixture=original_rule_fixture();
        timed.register_course(1,fixture);
        timed.load_next_hole();
        timed.set_club(12);
        timed.start_original_welly();
        bool threw=false;
        try {timed.set_club(0);} catch(const std::logic_error&) {threw=true;}
        assert(threw); // no club mutation during original meter
        for(int tick=0;tick<176;++tick)
            assert(!timed.tick_original_welly(false));
        assert(!timed.original_welly_active());
        assert(timed.original_welly_rejected());
        assert(timed.hole().strokes()==0);
        timed.start_original_welly();
        bool submitted=false;
        for(int tick=0;tick<=111;++tick)
            submitted=timed.tick_original_welly(tick==12 || tick==111) || submitted;
        assert(submitted);
        assert(timed.hole().strokes()==1);
        assert(!timed.original_welly_rejected());
        // A phase-zero meter must not be ticked again; loop above must
        // therefore use exactly the original 176 sampled callback count.
    }
    // Original x86 out-of-profile swing creates NO flight but CHARGES one stroke.
    // Persist the charged stroke as an atomic original-input replay frame.
    {
        auto rejected=make_runtime();
        auto fixture=away_from_cup_fixture();
        rejected.register_course(1,fixture);
        rejected.load_next_hole();
        rejected.set_club(12);
        rejected.start_original_welly();
        for(int tick=0;tick<=114;++tick)
            assert(!rejected.tick_original_welly(tick==12 || tick==114));
        assert(rejected.original_welly_rejected());
        assert(rejected.hole().strokes()==1);
        assert(rejected.hole().recovered_counters().player_52==1);
        assert(rejected.hole().recovered_counters().player_56==1);
        assert(rejected.controls_enabled());
        assert(rejected.replay_supported());
        assert(rejected.saved_shots()==1);
        const auto checkpoint=rejected.serialize_stable_replay();
        auto resumed=make_runtime();
        resumed.register_course(1,fixture);
        resumed.restore_replay(checkpoint);
        assert(resumed.has_hole());
        assert(resumed.hole().strokes()==1);
        assert(resumed.hole().recovered_counters().player_52==1);
        assert(resumed.hole().recovered_counters().player_56==1);
        assert(resumed.saved_shots()==1);
        assert(resumed.controls_enabled());
        assert(resumed.serialize_stable_replay()==checkpoint);
        rejected.start_original_welly();
        bool accepted=false;
        for(int tick=0;tick<=111;++tick)
            accepted=rejected.tick_original_welly(tick==12 || tick==111) || accepted;
        assert(accepted);
        assert(rejected.hole().strokes()==2);
    }
    // The manual three-click adapter must persist the very same paid,
    // no-flight original input, including its stationary coordinates.
    {
        auto manual=make_runtime();
        auto fixture=away_from_cup_fixture();
        manual.register_course(1,fixture);
        manual.load_next_hole();
        manual.set_club(12);
        const auto initial_x=manual.hole().ball_x_raw();
        const auto initial_y=manual.hole().ball_y_raw();
        assert(manual.hole().distance_to_hole()>0);
        manual.meter_click_raw(0);
        manual.meter_click_raw(30);
        manual.meter_click_raw(105); // outside authentic club-12 profile
        assert(manual.hole().phase()==sensigolf::HoleSessionPhase::ReadyForShot);
        assert(manual.hole().strokes()==1);
        assert(manual.hole().ball_x_raw()==initial_x);
        assert(manual.hole().ball_y_raw()==initial_y);
        assert(manual.controls_enabled());
        assert(manual.replay_supported());
        assert(manual.saved_shots()==1);
        const auto checkpoint=manual.serialize_stable_replay();
        auto reloaded=make_runtime();
        reloaded.register_course(1,fixture);
        reloaded.restore_replay(checkpoint);
        assert(reloaded.hole().phase()==sensigolf::HoleSessionPhase::ReadyForShot);
        assert(reloaded.hole().strokes()==1);
        assert(reloaded.hole().ball_x_raw()==initial_x);
        assert(reloaded.hole().ball_y_raw()==initial_y);
        assert(reloaded.saved_shots()==1);
        assert(reloaded.serialize_stable_replay()==checkpoint);
    }
    auto game=make_runtime();
    auto empty=original_rule_fixture();
    for(unsigned id=1;id<=18;++id)
        game.register_course(static_cast<std::uint8_t>(id),empty);
    assert(!game.has_hole());
    auto next=game.resource_request();
    assert(next && next->resource_id==1);
    game.load_next_hole();
    assert(game.has_hole());
    assert(game.controls_enabled());
    game.set_aim_raw(4095);
    game.nudge_aim_raw(1);
    assert(game.aim_raw()==0);
    game.cycle_club(-1);
    assert(game.club_index()==12);
    game.cancel_meter();
    game.meter_click_raw(0);
    game.meter_click_raw(30);
    game.cancel_meter();
    assert(game.meter_stage()==sensigolf::ClassicMeterStage::Idle);
    // Score, accumulate, genuinely transition through all 18 core sessions.
    hit_cup(game);
    assert(game.saved_shots()==1);
    assert(game.hole().strokes()==1);
    assert(!game.controls_enabled());
    const auto checkpoint=game.serialize_stable_replay();
    assert(checkpoint.find("SENSIGOLF_REPLAY 1")!=std::string::npos);
    auto restored=make_runtime();
    for(unsigned id=1;id<=18;++id)
        restored.register_course(static_cast<std::uint8_t>(id),empty);
    restored.restore_replay(checkpoint);
    // Original replay automatically commits a scored hole and requests next.
    assert(!restored.has_hole());
    assert(restored.round_state().holes_completed==1);
    assert(restored.round_state().total_strokes==1);
    std::string corrupted=checkpoint;
    corrupted.at(corrupted.find("SENSIGOLF_REPLAY")+2)='X';
    bool rejected=false;
    try {restored.restore_replay(corrupted);}
    catch(const std::exception&) {rejected=true;}
    assert(rejected);
    assert(restored.round_state().holes_completed==1); // fail-atomic restore.
    for(unsigned index=1;index<18;++index) {
        auto req=restored.resource_request();
        assert(req && req->resource_id==index+1);
        restored.load_next_hole();
        hit_cup(restored);
        assert(restored.saved_shots()==index+1);
        restored.commit_scored_hole();
        assert(restored.round_state().holes_completed==index+1);
    }
    const auto& score=restored.round_state();
    assert(score.round_complete && score.total_strokes==18);
    assert(score.cumulative_par==72 && score.relative_to_par==54);
    assert(restored.saved_shots()==18);
    assert(!restored.resource_request());
    rejected=false;
    try {restored.load_next_hole();}
    catch(const std::logic_error&) {rejected=true;}
    assert(rejected);
    const auto final_save=restored.serialize_stable_replay();
    auto final_replay=make_runtime();
    for(unsigned id=1;id<=18;++id)
        final_replay.register_course(static_cast<std::uint8_t>(id),empty);
    final_replay.restore_replay(final_save);
    assert(final_replay.round_state().round_complete);
    assert(final_replay.round_state().total_strokes==18);
    assert(final_replay.saved_shots()==18);
    return 0;
}
