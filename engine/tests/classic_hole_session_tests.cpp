#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "sensigolf/classic_hole_session.hpp"

using namespace sensigolf;

namespace {

void be16(std::vector<std::uint8_t>& data, std::size_t off, std::uint16_t value) {
    data[off] = static_cast<std::uint8_t>(value >> 8);
    data[off + 1] = static_cast<std::uint8_t>(value & 0xFF);
}

ClassicCourseResources uniform_course(
    std::uint16_t descriptor_index,
    std::uint16_t hole_x = 0,
    std::uint16_t hole_y = 0) {
    constexpr std::uint16_t width = 4;
    constexpr std::uint16_t height = 128;

    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes
        + static_cast<std::size_t>(width) * height * 2u,
        0);
    be16(mapm, 0x54, width);
    be16(mapm, 0x56, height);

    for (std::size_t off = ClassicCourseResources::kMapHeaderBytes;
         off < mapm.size();
         off += 2) {
        be16(mapm, off, 0);
    }

    std::vector<std::uint8_t> spt(50, 0);
    // SPT record 4, words 2/3 are the recovered cup coordinates.
    be16(spt, 44, hole_x);
    be16(spt, 46, hole_y);

    std::vector<std::uint8_t> desc(8, 0);
    std::vector<std::uint8_t> sel(8, 0);
    be16(desc, 0, descriptor_index);

    return ClassicCourseResources(
        std::move(mapm), std::move(spt), std::move(desc), std::move(sel));
}

void run_active(ClassicHoleSession& session, int max_ticks = 1024) {
    int ticks = 0;
    while (session.phase() == HoleSessionPhase::ShotActive
           && ticks++ < max_ticks) {
        session.step();
    }
    assert(ticks < max_ticks);
}

ClassicShotRequest putter_request() {
    ClassicShotRequest putt{};
    putt.club_index = 12;
    putt.power_tick = 30;
    putt.accuracy_tick = 63;
    putt.aim_raw = 0;
    return putt;
}

} // namespace

int main() {
    auto fairway = uniform_course(1);
    ClassicHoleSession session(fairway, 0);
    ClassicShotRequest drive{};
    drive.club_index = 0;
    drive.power_tick = 105;
    drive.accuracy_tick = 63;
    drive.aim_raw = 0;
    session.begin_shot(drive);
    assert(session.strokes() == 1);
    assert(!session.pending_cup_capture_event().has_value());
    assert(session.recovered_counters().player_52 == 1);
    assert(session.recovered_counters().player_56 == 1);
    run_active(session);
    assert(session.phase() == HoleSessionPhase::ReadyForShot);
    assert(session.ball_y_raw() > 0);

    // Tested actual original 990 x86 extremes: source charges strokes
    // even when accuracy misses the club/lie profile and ball never moves.
    ClassicHoleSession missed(fairway, 0);
    auto severe_miss=drive;
    severe_miss.accuracy_tick=105; // club0 + fairway profile 61..65
    missed.begin_shot(severe_miss);
    assert(missed.strokes()==1);
    assert(missed.phase()==HoleSessionPhase::ReadyForShot);
    assert(!missed.ball_state().holed);
    assert(missed.ball_x_raw()==0 && missed.ball_y_raw()==0);
    assert(missed.ball_state().x_raw==missed.ball_x_raw());
    assert(missed.ball_state().y_raw==missed.ball_y_raw());
    assert(missed.recovered_counters().player_52==1);
    assert(missed.recovered_counters().player_56==1);
    missed.begin_shot(drive);
    assert(missed.strokes()==2);
    assert(missed.recovered_counters().player_52==2);
    assert(missed.recovered_counters().player_56==2);
    run_active(missed);

    // Original zero-distance scoring is not dependent on code-8 tiles.
    // On ordinary fairway, a shot can come to rest at the recovered cup
    // distance (integer-scoped), and scoring occurs on the following tick.
    const auto rest_x = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(session.ball_x_raw()) >> 16);
    const auto rest_y = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(session.ball_y_raw()) >> 16);
    auto rest_course = uniform_course(1, rest_x, rest_y);
    ClassicHoleSession rest_scored(
        rest_course, 0, 0, ClassicHoleMetadata{0, 4});
    rest_scored.step();
    assert(rest_scored.phase() == HoleSessionPhase::ReadyForShot);
    rest_scored.begin_shot(drive);
    run_active(rest_scored);
    assert(rest_scored.phase() == HoleSessionPhase::ReadyForShot);
    assert(rest_scored.distance_to_hole() == 0);
    rest_scored.step();
    assert(rest_scored.phase() == HoleSessionPhase::HoleScored);
    assert(rest_scored.recovered_counters().player_52 == 1);
    assert(rest_scored.recovered_counters().player_56 == 1);
    assert(rest_scored.recovered_counters().player_70 == 1);
    rest_scored.step();
    assert(rest_scored.recovered_counters().player_70 == 1);

    auto hole = uniform_course(7);
    ClassicHoleSession hole_session(hole, 0, 0);
    ClassicShotRequest approach{};
    approach.club_index = 5;
    approach.power_tick = 83;
    approach.accuracy_tick = 63;
    approach.aim_raw = 0;
    hole_session.begin_shot(approach);
    run_active(hole_session);
    assert(hole_session.phase() == HoleSessionPhase::CupTerminal);
    // The source code8 cup *display* event is only enabled for the
    // independently verified original putter path; non-putter timing is open.
    assert(!hole_session.pending_cup_capture_event().has_value());
    assert(hole_session.ball_state().holed);

    // Code-8 CupTerminal alone must NOT count as a scored hole. This
    // independently authored fixture leaves its cup at (0,0), away from
    // the actual shot end point. Exact zero-distance non-putter scoring is
    // exercised in the earlier rest_scored test above.
    ClassicHoleSession scored_approach(
        hole, 0, 0, ClassicHoleMetadata{0, 4});
    scored_approach.begin_shot(approach);
    run_active(scored_approach);
    assert(scored_approach.phase() == HoleSessionPhase::CupTerminal);
    assert(scored_approach.distance_to_hole() > 0);
    scored_approach.step();
    assert(scored_approach.phase() == HoleSessionPhase::CupTerminal);
    assert(scored_approach.strokes() == 1);

    auto water = uniform_course(6);
    ClassicHoleSession water_session(water, 0, 0);
    ClassicShotRequest water_shot{};
    water_shot.club_index = 5;
    water_shot.power_tick = 83;
    water_shot.accuracy_tick = 63;
    water_shot.aim_raw = 0;
    water_session.begin_shot(water_shot);
    run_active(water_session);
    assert(water_session.phase() == HoleSessionPhase::HazardStopped);
    assert(water_session.last_hazard_kind() == ClassicNamedHazard::Water);
    assert(water_session.ball_state().hazard);
    assert(water_session.hazard_pause_remaining() == 100);
    const auto hazard_x = water_session.ball_x_raw();
    const auto hazard_y = water_session.ball_y_raw();

    // Original branch decrements 100..0 and recovers on the following tick.
    for (int i = 0; i < 100; ++i) {
        water_session.step();
    }
    assert(water_session.phase() == HoleSessionPhase::HazardStopped);
    assert(water_session.hazard_pause_remaining() == 0);
    water_session.step();
    assert(water_session.phase() == HoleSessionPhase::HazardRecovered);
    assert(water_session.ball_x_raw() == hazard_x);
    assert(water_session.ball_y_raw() == hazard_y);
    assert(!water_session.ball_state().hazard);

    // The original code-35 stop/recovery fragment does not add another
    // single-player stroke. Presentation completion clears the terminal gate
    // separately, so the platform host explicitly acknowledges that boundary.
    assert(water_session.recovered_counters().player_52 == 1);
    assert(water_session.recovered_counters().player_56 == 1);
    bool hazard_replay_blocked = false;
    try {
        water_session.begin_shot(water_shot);
    } catch (const std::logic_error&) {
        hazard_replay_blocked = true;
    }
    assert(hazard_replay_blocked);

    water_session.acknowledge_hazard_recovery();
    assert(water_session.phase() == HoleSessionPhase::ReadyForShot);
    water_session.begin_shot(water_shot);
    assert(water_session.strokes() == 2);
    assert(water_session.recovered_counters().player_52 == 2);
    assert(water_session.recovered_counters().player_56 == 2);

    // SKIRT/code 2 is a parity-proven original club-12 path. It must
    // remain playable at the session layer rather than being rejected as
    // unsupported terrain.
    auto skirt = uniform_course(0);
    ClassicHoleSession fringe_putt(skirt, 0, 0);
    auto short_putt = putter_request();
    short_putt.power_tick = 10;
    fringe_putt.begin_shot(short_putt);
    run_active(fringe_putt);
    assert(fringe_putt.phase() == HoleSessionPhase::ReadyForShot);
    assert(fringe_putt.strokes() == 1);

    // Normal GREEN H4/code 1 putter path.
    auto green = uniform_course(31);
    ClassicHoleSession putt_session(green, 0, 0);
    putt_session.begin_shot(putter_request());
    run_active(putt_session);
    assert(putt_session.phase() == HoleSessionPhase::ReadyForShot);

    // Original GREEN H4 also accepts every non-putter club. Native oracle
    // matrix proves its gravity/bounce/roll, not putter slope integration.
    for (std::uint16_t club_index=0; club_index<12; ++club_index) {
        ClassicHoleSession green_chip(green,0,0);
        auto chip=putter_request();
        chip.club_index=club_index;
        green_chip.begin_shot(chip);
        run_active(green_chip);
        assert(green_chip.phase()==HoleSessionPhase::ReadyForShot);
        assert(green_chip.strokes()==1);
    }

    // A normal resting putt at recovered distance zero also scores without
    // subtracting the transient extra code-8 putter terminal counters.
    const auto putt_rest_x = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(putt_session.ball_x_raw()) >> 16);
    const auto putt_rest_y = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(putt_session.ball_y_raw()) >> 16);
    auto putt_rest_course = uniform_course(31, putt_rest_x, putt_rest_y);
    ClassicHoleSession putt_rest_scored(
        putt_rest_course, 0, 0, ClassicHoleMetadata{0, 4});
    putt_rest_scored.begin_shot(putter_request());
    run_active(putt_rest_scored);
    assert(putt_rest_scored.phase() == HoleSessionPhase::ReadyForShot);
    assert(putt_rest_scored.distance_to_hole() == 0);
    putt_rest_scored.step();
    assert(putt_rest_scored.phase() == HoleSessionPhase::HoleScored);
    assert(putt_rest_scored.recovered_counters().player_52 == 1);
    assert(putt_rest_scored.recovered_counters().player_56 == 1);
    assert(putt_rest_scored.recovered_counters().player_58 == 4);
    assert(putt_rest_scored.recovered_counters().player_70 == 1);

    // GREEN H1/code 8 is the parity-proven cup terminal for a putter.
    // It is intentionally not HoleScored: original v1.014 performs scored-hole
    // activation in the separate zero-distance pre-update branch.
    auto cup = uniform_course(7);
    ClassicHoleSession cup_putt(cup, 0, 0);
    cup_putt.begin_shot(putter_request());
    run_active(cup_putt);
    assert(cup_putt.phase() == HoleSessionPhase::CupTerminal);
    assert(cup_putt.pending_cup_capture_event().has_value());
    assert(cup_putt.pending_cup_capture_event()->dword_420548==0x1000fu);
    assert(cup_putt.pending_cup_capture_event()->source_esi==0x42067eu);
    assert(cup_putt.ball_state().holed);
    assert(cup_putt.recovered_counters().player_52 == 2);
    assert(cup_putt.recovered_counters().player_56 == 2);

    // With recovered hole metadata, the next session tick models the original
    // zero-distance scored-hole branch and its single-player turn adjustment.
    ClassicHoleSession scored_putt(
        cup, 0, 0, ClassicHoleMetadata{0, 4});
    scored_putt.begin_shot(putter_request());
    run_active(scored_putt);
    assert(scored_putt.phase() == HoleSessionPhase::CupTerminal);
    assert(scored_putt.distance_to_hole() == 0);
    scored_putt.step();
    assert(scored_putt.phase() == HoleSessionPhase::HoleScored);
    assert(scored_putt.strokes() == 1);
    assert(scored_putt.recovered_counters().player_52 == 1);
    assert(scored_putt.recovered_counters().player_56 == 1);
    assert(scored_putt.recovered_counters().player_58 == 4);
    assert(scored_putt.recovered_counters().player_48 == 3);
    assert(scored_putt.recovered_counters().player_70 == 1);
    assert(scored_putt.next_hole_index().has_value());
    assert(*scored_putt.next_hole_index() == 1);
    assert(!scored_putt.round_complete());

    // Landing code 8 alone must not be promoted to scored-hole state when the
    // original recovered distance helper is non-zero.
    auto distant_cup = uniform_course(7, 20, 20);
    ClassicHoleSession distant_scored(
        distant_cup, 0, 0, ClassicHoleMetadata{0, 4});
    distant_scored.begin_shot(putter_request());
    run_active(distant_scored);
    assert(distant_scored.phase() == HoleSessionPhase::CupTerminal);
    assert(distant_scored.distance_to_hole() > 0);
    distant_scored.step();
    assert(distant_scored.phase() == HoleSessionPhase::CupTerminal);

    // Original zero-based hole index increments to 18 at end-of-round.
    ClassicHoleSession last_hole(
        cup, 0, 0, ClassicHoleMetadata{17, 4});
    last_hole.begin_shot(putter_request());
    run_active(last_hole);
    last_hole.step();
    assert(last_hole.phase() == HoleSessionPhase::HoleScored);
    assert(last_hole.next_hole_index().has_value());
    assert(*last_hole.next_hole_index() == 18);
    assert(last_hole.round_complete());

    // GREEN H3/code 9 and GREEN H2/code 10 terminate through the original
    // event-11 path. They are known terminal outcomes but the subsequent
    // game-flow transition is intentionally kept distinct until recovered.
    for (auto descriptor : {std::uint16_t{23}, std::uint16_t{15}}) {
        auto special = uniform_course(descriptor);
        ClassicHoleSession special_putt(special, 0, 0);
        special_putt.begin_shot(putter_request());
        run_active(special_putt);
        assert(special_putt.phase() == HoleSessionPhase::SpecialGreenStopped);
    assert(special_putt.pending_special_green_event().has_value());
    assert(special_putt.pending_special_green_event()->dword_420548==0x1000du);

        assert(!special_putt.ball_state().holed);
        // Source v1.014 0x40A403 increments on code9 early stop,
        // but late club-12 code10 enters directly at 0x40A40B.
        const auto source_strokes=1u; // BOTH club12 code9/10 go direct
        // 0x40A40B without early-code9 extra stroke.
        assert(special_putt.strokes()==source_strokes);
        assert(special_putt.recovered_counters().player_52==source_strokes);
        assert(special_putt.recovered_counters().player_56==source_strokes);
        assert(special_putt.special_green_pause_remaining() == 100);

        bool early_ack_blocked = false;
        try {
            special_putt.acknowledge_special_green_stop();
        } catch (const std::logic_error&) {
            early_ack_blocked = true;
        }
        assert(early_ack_blocked);

        for (int i = 0; i < 100; ++i) {
            special_putt.step();
        }
        assert(special_putt.phase() == HoleSessionPhase::SpecialGreenStopped);
        assert(special_putt.special_green_pause_remaining() == 0);
        // Original x86 decrements the zero WORD once MORE before
        // continuing (0 -> 0xffff, signed JNS becomes false).
        bool zero_ack_blocked=false;
        try {
            special_putt.acknowledge_special_green_stop();
        } catch(const std::logic_error&) {
            zero_ack_blocked=true;
        }
        assert(zero_ack_blocked);
        special_putt.step(); // original delayed pause underflow
        assert(special_putt.special_green_pause_remaining() == 0);
        // Same recovered original relocation primitive as the source
        // code-35 hazard branch: stopped ball and public origin must agree.
        assert(special_putt.ball_state().x_raw == special_putt.ball_x_raw());
        assert(special_putt.ball_state().y_raw == special_putt.ball_y_raw());
        special_putt.acknowledge_special_green_stop();
        assert(special_putt.phase() == HoleSessionPhase::ReadyForShot);
        special_putt.begin_shot(putter_request());
        assert(special_putt.strokes()==source_strokes+1u);
        assert(special_putt.recovered_counters().player_52==source_strokes+1u);
        assert(special_putt.recovered_counters().player_56==source_strokes+1u);
    }

    // GREEN D2/code 50 remains unsupported for club 12 until its specific
    // pause/state path is parity-integrated.
    auto down_green = uniform_course(47);
    ClassicHoleSession blocked(down_green, 0, 0);
    bool threw = false;
    try {
        blocked.begin_shot(putter_request());
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    assert(blocked.phase() == HoleSessionPhase::UnsupportedTerrain);
    assert(blocked.unsupported_descriptor().has_value());
    assert(*blocked.unsupported_descriptor() == 47);

    // The original MAPI descriptor 22 is named OUT OF BOUNDS and shares
    // code35 physics with water; the product must NOT silently label it
    // water or charge the separate world-boundary penalty here.
    {
        auto oob_tile_course = uniform_course(22);
        ClassicHoleSession oob_tile(oob_tile_course,0,0);
        oob_tile.begin_shot(water_shot);
        run_active(oob_tile);
        assert(oob_tile.phase()==HoleSessionPhase::HazardStopped);
        assert(oob_tile.last_hazard_kind()==ClassicNamedHazard::OutOfBoundsTile);
        assert(!oob_tile.pending_world_oob_event().has_value());
        assert(oob_tile.hazard_pause_remaining()==100);
        assert(oob_tile.strokes()==1);
        assert(oob_tile.recovered_counters().player_52==1);
        assert(oob_tile.recovered_counters().player_56==1);
        for(int i=0;i<101;++i)oob_tile.step();
        assert(oob_tile.phase()==HoleSessionPhase::HazardRecovered);
        assert(oob_tile.last_hazard_kind()==ClassicNamedHazard::OutOfBoundsTile);
        oob_tile.acknowledge_hazard_recovery();
        assert(oob_tile.phase()==HoleSessionPhase::ReadyForShot);
        oob_tile.begin_shot(water_shot);
        assert(oob_tile.strokes()==2);
        assert(oob_tile.last_hazard_kind()==ClassicNamedHazard::None);
    }

    // Real gameplay world-boundary case: drive aim 0 advances Y. Start\n    // one integer unit below the course's north edge (height 128*8),\n    // rather than trying to escape the EAST X edge on a northward shot.\n    // Do not crash inside MAPI lookup.
    // The original world negative-AX branch charges TWO counters +1
    // over the already counted launch stroke, then uses a saved safe
    // position. It is distinct from an in-map descriptor-22 hazard.
    {
        ClassicHoleSession world_oob(fairway,32<<16,1023<<16);
        world_oob.begin_shot(drive);
        run_active(world_oob);
        assert(world_oob.phase()==HoleSessionPhase::HazardStopped);
        assert(world_oob.last_hazard_kind()==ClassicNamedHazard::OutsideWorld);
        assert(world_oob.strokes()==2);
        assert(world_oob.recovered_counters().player_52==2);
        assert(world_oob.recovered_counters().player_56==2);
        assert(world_oob.hazard_pause_remaining()==100);
        assert(world_oob.pending_world_oob_event().has_value());
        assert(world_oob.pending_world_oob_event()->byte_420fef==1);
        assert(world_oob.pending_world_oob_event()->dword_420548==0x1000d);

        for(int tick=0;tick<101;++tick)world_oob.step();
        assert(world_oob.phase()==HoleSessionPhase::HazardRecovered);
        // Actual original 0x40AACF -> 0x40A871 uses safe anchor +15
        // and does not leave the ball outside course dimensions.
        assert(world_oob.ball_x_raw()>=0);
        assert(world_oob.ball_x_raw()<(64<<16));
        assert(world_oob.ball_y_raw()>=0);
        assert(world_oob.ball_y_raw()<(1024<<16));
        assert(world_oob.last_hazard_kind()==ClassicNamedHazard::OutsideWorld);
        world_oob.acknowledge_hazard_recovery();
        assert(world_oob.phase()==HoleSessionPhase::ReadyForShot);
        assert(!world_oob.pending_world_oob_event().has_value());
    }

    // Object presence is an explicit source/manifest gate: it cannot
    // silently activate from a cup location alone, and is immutable
    // once original stroke/scoring input has been recorded.
    {
        auto pflag=uniform_course(3,32,80);
        recovered::OriginalPrng16 rng{1,2};
        ClassicHoleSession with_flag(pflag,0,0,std::nullopt,&rng);
        assert(!with_flag.flag_object_verified());
        with_flag.set_verified_flag_object(true);
        assert(with_flag.flag_object_verified());
        with_flag.begin_shot(drive);
        bool locked=false;
        try{with_flag.set_verified_flag_object(false);}
        catch(const std::logic_error&){locked=true;}
        assert(locked);
        ClassicHoleSession no_prng(pflag,0,0);
        locked=false;
        try{no_prng.set_verified_flag_object(true);}
        catch(const std::logic_error&){locked=true;}
        assert(locked);
    }

    // Original mode must be explicitly supplied; near-cup lip is not
    // inferred in unauthenticated sessions or without source PRNG.
    {
        auto lip_course=uniform_course(1,0,0);
        recovered::OriginalPrng16 rng{0x2345,0x6789};
        ClassicHoleSession marked(lip_course,0,0,std::nullopt,&rng);
        assert(!marked.original_player_mode_5a().has_value());
        marked.set_original_player_mode_5a(0);
        assert(marked.original_player_mode_5a().value()==0);
        marked.begin_shot(drive);
        marked.step();
        assert(marked.original_lip_deflection_last_tick());
        bool refused=false;
        try{marked.set_original_player_mode_5a(6);}
        catch(const std::logic_error&){refused=true;}
        assert(refused);
        ClassicHoleSession unseeded(lip_course,0,0);
        refused=false;
        try{unseeded.set_original_player_mode_5a(0);}
        catch(const std::logic_error&){refused=true;}
        assert(refused);
        unseeded.set_original_player_mode_5a(4);
        assert(unseeded.original_player_mode_5a().value()==4);
    }
    // Source 0x40A3E0 non-putter code9 emits original animation id2
    // (rect x-6,y-13,6x12), charges one extra stroke and enters
    // the 100-tick special stop. Only user-authored art references
    // may populate the native animation event queue.
    {
        auto code9=uniform_course(23,48,250);
        recovered::OriginalPrng16 rng{0x2345,0x6789};
        recovered::OriginalAnimationAssetTable sprites{};
        for(std::size_t i=0;i<sprites.size();++i)
            sprites[i]=static_cast<std::uint32_t>(1000u+i);
        ClassicHoleSession early(code9,32<<16,40<<16,
                                 std::nullopt,&rng);
        early.set_original_animation_assets(sprites);
        auto low=drive;
        low.club_index=0;
        low.power_tick=20;
        low.accuracy_tick=63;
        early.begin_shot(low);
        run_active(early);
        assert(early.phase()==HoleSessionPhase::SpecialGreenStopped);
        assert(early.strokes()==2);
        const auto& slots=early.original_animation_events();
        assert(slots[0].status==0);
        assert(slots[0].asset_pointer==sprites[2]);
        assert(slots[0].width==6 && slots[0].height==12);
        assert(slots[0].ticks==4);
        for(int i=0;i<4;++i)
            early.step_original_animation_timers();
        assert(early.original_animation_events()[0].ticks==0);
        bool needs_script=false;
        try{early.step_original_animation_timers();}
        catch(const std::logic_error&){needs_script=true;}
        assert(needs_script); // cannot fabricate source frames
        // With an explicit original-format script catalog, the new
        // source 0x40C539 ordinary-frame handler can advance the actual
        // code9 event queue; negative command opcodes still reject.
        const auto active=early.step_original_animation_plain_scripts({
            {sprites[2],{5,0,0,0,0,0,17,0,19,0}}
        });
        assert(active.active_slots==1);
        assert(early.original_animation_events()[0].ticks==5);
        assert(early.original_animation_events()[0].height==17);
        assert(early.original_animation_events()[0].frame_offset==2);
        const auto progressing=early.step_original_animation_plain_scripts({});
        assert(progressing.active_slots==1);
        assert(early.original_animation_events()[0].ticks==4);
        // The real source queuing/slot lifecycle now supports plain
        // sprite frame -> 0x8008 WAIT -> 0x8004 release, with only
        // host-authorized authored script bytes, not copied game art.
        for(int i=0;i<4;++i)
            early.step_original_animation_plain_scripts({});
        assert(early.original_animation_events()[0].ticks==0);
        const std::vector<std::uint8_t> script_lifecycle{
            5,0,0,0,0,0,17,0,8,0x80,2,0,4,0x80
        };
        auto wait_event=early.step_original_animation_plain_scripts({
            {sprites[2],script_lifecycle}
        });
        assert(wait_event.active_slots==1);
        assert(early.original_animation_events()[0].ticks==2);
        assert(early.original_animation_events()[0].frame_offset==6);
        for(int i=0;i<2;++i)
            early.step_original_animation_plain_scripts({});
        assert(early.original_animation_events()[0].ticks==0);
        auto closure=early.step_original_animation_plain_scripts({
            {sprites[2],script_lifecycle}
        });
        assert(closure.active_slots==1);
        assert(early.original_animation_events()[0].status==0xffffu);
        assert(early.original_animation_events()[0].asset_pointer==0);
        assert(early.original_animation_events()[0].frame_offset==0);
        assert(early.step_original_animation_plain_scripts({}).active_slots==0);


        ClassicHoleSession direct(code9,32<<16,40<<16,
                                  std::nullopt,&rng);
        direct.set_original_animation_assets(sprites);
        direct.begin_shot(putter_request());
        run_active(direct);
        assert(direct.phase()==HoleSessionPhase::SpecialGreenStopped);
        assert(direct.strokes()==1);
        assert(direct.original_animation_events()[0].status==0xffff);
    }
    return 0;
}
