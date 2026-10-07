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

ClassicCourseResources uniform_course(std::uint16_t descriptor_index) {
    constexpr std::uint16_t width = 4;
    constexpr std::uint16_t height = 128;

    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes
        + static_cast<std::size_t>(width) * height * 2u,
        0);
    be16(mapm, 0x54, width);
    be16(mapm, 0x56, height);

    // Every MAPM cell references MAPI tile 0.
    for (std::size_t off = ClassicCourseResources::kMapHeaderBytes;
         off < mapm.size();
         off += 2) {
        be16(mapm, off, 0);
    }

    std::vector<std::uint8_t> spt(50, 0);
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

} // namespace

int main() {
    // Fairway shot returns to ReadyForShot and counts exactly one stroke.
    auto fairway = uniform_course(1);
    ClassicHoleSession session(fairway, 0);
    assert(session.ball_x_raw() == 0);
    assert(session.ball_y_raw() == 0);
    assert(session.hole_position().x == 0);
    assert(session.hole_position().y == 0);
    ClassicShotRequest drive{};
    drive.club_index = 0;
    drive.power_tick = 105;
    drive.accuracy_tick = 63;
    drive.aim_raw = 0;
    session.begin_shot(drive);
    assert(session.phase() == HoleSessionPhase::ShotActive);
    assert(session.strokes() == 1);
    run_active(session);
    assert(session.phase() == HoleSessionPhase::ReadyForShot);
    assert(session.ball_y_raw() > 0);

    // A course made from the recovered hole descriptor reaches HoleComplete.
    auto hole = uniform_course(7);
    ClassicHoleSession hole_session(hole, 0, 0);
    ClassicShotRequest approach{};
    approach.club_index = 5;
    approach.power_tick = 83;
    approach.accuracy_tick = 63;
    approach.aim_raw = 0;
    hole_session.begin_shot(approach);
    run_active(hole_session);
    assert(hole_session.phase() == HoleSessionPhase::HoleComplete);
    assert(hole_session.strokes() == 1);
    assert(hole_session.ball_state().holed);

    // Water terminates in an explicit hazard state; recovery/drop rules are
    // intentionally not invented by this session layer.
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

    // Flat GREEN H4 supports the parity-proven putter path.
    auto green = uniform_course(31);
    ClassicHoleSession putt_session(green, 0, 0);
    ClassicShotRequest putt{};
    putt.club_index = 12;
    putt.power_tick = 30;
    putt.accuracy_tick = 63;
    putt.aim_raw = 0;
    putt_session.begin_shot(putt);
    run_active(putt_session);
    assert(putt_session.phase() == HoleSessionPhase::ReadyForShot);
    assert(putt_session.strokes() == 1);

    // GREEN H3/code-9 is known/recovered but not yet integrated into the
    // production shot wrapper. Refuse it rather than approximating.
    auto special_green = uniform_course(23);
    ClassicHoleSession blocked(special_green, 0, 0);
    bool threw = false;
    try {
        blocked.begin_shot(approach);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    assert(blocked.phase() == HoleSessionPhase::UnsupportedTerrain);
    assert(blocked.unsupported_descriptor().has_value());
    assert(*blocked.unsupported_descriptor() == 23);

    return 0;
}
