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
    assert(session.recovered_counters().player_52 == 1);
    assert(session.recovered_counters().player_56 == 1);
    run_active(session);
    assert(session.phase() == HoleSessionPhase::ReadyForShot);
    assert(session.ball_y_raw() > 0);

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
    assert(hole_session.ball_state().holed);

    // The original zero-distance scored-hole branch is not putter-specific.
    // A non-putter cup terminal at zero distance scores on the next session
    // tick without applying the transient club-12 counter correction.
    ClassicHoleSession scored_approach(
        hole, 0, 0, ClassicHoleMetadata{0, 4});
    scored_approach.begin_shot(approach);
    run_active(scored_approach);
    assert(scored_approach.phase() == HoleSessionPhase::CupTerminal);
    assert(scored_approach.distance_to_hole() == 0);
    scored_approach.step();
    assert(scored_approach.phase() == HoleSessionPhase::HoleScored);
    assert(scored_approach.strokes() == 1);
    assert(scored_approach.recovered_counters().player_52 == 1);
    assert(scored_approach.recovered_counters().player_56 == 1);
    assert(scored_approach.recovered_counters().player_58 == 4);
    assert(scored_approach.recovered_counters().player_48 == 3);
    assert(scored_approach.recovered_counters().player_70 == 1);

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
        assert(!special_putt.ball_state().holed);
        assert(special_putt.recovered_counters().player_52 == 1);
        assert(special_putt.recovered_counters().player_56 == 1);
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

        special_putt.acknowledge_special_green_stop();
        assert(special_putt.phase() == HoleSessionPhase::ReadyForShot);
        special_putt.begin_shot(putter_request());
        assert(special_putt.strokes() == 2);
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

    return 0;
}
