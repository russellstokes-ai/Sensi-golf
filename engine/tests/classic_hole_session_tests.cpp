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

    // Penalty/scoring semantics are not yet promoted, so recovered hazard
    // position is deliberately not ReadyForShot.
    bool hazard_replay_blocked = false;
    try {
        water_session.begin_shot(water_shot);
    } catch (const std::logic_error&) {
        hazard_replay_blocked = true;
    }
    assert(hazard_replay_blocked);

    // Normal GREEN H4/code 1 putter path.
    auto green = uniform_course(31);
    ClassicHoleSession putt_session(green, 0, 0);
    putt_session.begin_shot(putter_request());
    run_active(putt_session);
    assert(putt_session.phase() == HoleSessionPhase::ReadyForShot);

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

    // The original scored-hole branch is a second transition, gated by the
    // recovered distance helper reaching exactly zero. The single-player
    // post-code8 adjustment removes the putter terminal's transient extra
    // counter increment before the original score update.
    assert(cup_putt.distance_to_hole(false) == 0u);
    assert(cup_putt.scored_hole_ready(false));
    cup_putt.activate_scored_hole(4, false);
    assert(cup_putt.phase() == HoleSessionPhase::HoleScored);
    assert(cup_putt.strokes() == 1u);
    assert(cup_putt.recovered_counters().player_52 == 1u);
    assert(cup_putt.recovered_counters().player_56 == 1u);
    assert(cup_putt.recovered_counters().player_58 == 4u);
    assert(cup_putt.recovered_counters().player_48 == 3);
    assert(cup_putt.recovered_counters().player_70 == 1u);

    bool rescore_blocked = false;
    try {
        cup_putt.activate_scored_hole(4, false);
    } catch (const std::logic_error&) {
        rescore_blocked = true;
    }
    assert(rescore_blocked);

    // Code 8 alone is not sufficient: with the SPT cup elsewhere the session
    // remains CupTerminal and the score transition is correctly rejected.
    auto distant_cup = uniform_course(7, 10, 10);
    ClassicHoleSession distant_putt(distant_cup, 0, 0);
    distant_putt.begin_shot(putter_request());
    run_active(distant_putt);
    assert(distant_putt.phase() == HoleSessionPhase::CupTerminal);
    assert(distant_putt.distance_to_hole(false) != 0u);
    assert(!distant_putt.scored_hole_ready(false));
    bool nonzero_score_blocked = false;
    try {
        distant_putt.activate_scored_hole(4, false);
    } catch (const std::logic_error&) {
        nonzero_score_blocked = true;
    }
    assert(nonzero_score_blocked);

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
