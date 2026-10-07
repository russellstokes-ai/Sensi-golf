#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "sensigolf/classic_round_session.hpp"

using namespace sensigolf;

namespace {

void be16(std::vector<std::uint8_t>& data, std::size_t off, std::uint16_t value) {
    data[off] = static_cast<std::uint8_t>(value >> 8);
    data[off + 1] = static_cast<std::uint8_t>(value & 0xFF);
}

ClassicCourseResources cup_course() {
    constexpr std::uint16_t width = 4;
    constexpr std::uint16_t height = 128;

    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes
        + static_cast<std::size_t>(width) * height * 2u,
        0);
    be16(mapm, 0x54, width);
    be16(mapm, 0x56, height);

    std::vector<std::uint8_t> spt(50, 0);
    std::vector<std::uint8_t> desc(8, 0);
    std::vector<std::uint8_t> sel(8, 0);
    be16(desc, 0, 7); // GREEN H1 / code 8

    return ClassicCourseResources(
        std::move(mapm), std::move(spt), std::move(desc), std::move(sel));
}

ClassicShotRequest putter_request() {
    ClassicShotRequest putt{};
    putt.club_index = 12;
    putt.power_tick = 30;
    putt.accuracy_tick = 63;
    putt.aim_raw = 0;
    return putt;
}

void score_one_putt_hole(
    ClassicHoleSession& hole) {
    hole.begin_shot(putter_request());

    int ticks = 0;
    while (hole.phase() == HoleSessionPhase::ShotActive && ticks++ < 32) {
        hole.step();
    }
    assert(ticks < 32);
    assert(hole.phase() == HoleSessionPhase::CupTerminal);

    // The following session tick owns the recovered zero-distance scored-hole
    // transition when metadata is present.
    hole.step();
    assert(hole.phase() == HoleSessionPhase::HoleScored);
}

} // namespace

int main() {
    auto course = cup_course();
    ClassicRoundSession round;

    assert(round.state().current_hole_index == 0u);
    assert(!round.state().round_complete);

    for (std::uint16_t index = 0; index < 18u; ++index) {
        ClassicHoleSession hole(
            course,
            0,
            0,
            ClassicHoleMetadata{index, 4});
        score_one_putt_hole(hole);
        round.accept_scored_hole(hole);

        assert(round.state().current_hole_index
            == static_cast<std::uint32_t>(index) + 1u);
        assert(round.state().holes_completed
            == static_cast<std::uint16_t>(index + 1u));
        assert(round.state().round_complete == (index == 17u));
    }

    assert(round.state().current_hole_index == 18u);
    assert(round.state().holes_completed == 18u);
    assert(round.state().total_strokes == 18u);
    assert(round.state().cumulative_par == 72u);
    assert(round.state().relative_to_par == 54);
    assert(round.state().round_complete);

    // A completed round cannot accept another result.
    ClassicHoleSession extra(
        course,
        0,
        0,
        ClassicHoleMetadata{17, 4});
    score_one_putt_hole(extra);
    bool completed_round_blocked = false;
    try {
        round.accept_scored_hole(extra);
    } catch (const std::logic_error&) {
        completed_round_blocked = true;
    }
    assert(completed_round_blocked);

    // Hole metadata also prevents a stale/duplicate result from being applied
    // to the wrong round position.
    round.reset();
    ClassicHoleSession first(
        course,
        0,
        0,
        ClassicHoleMetadata{0, 4});
    score_one_putt_hole(first);
    round.accept_scored_hole(first);

    bool duplicate_blocked = false;
    try {
        round.accept_scored_hole(first);
    } catch (const std::logic_error&) {
        duplicate_blocked = true;
    }
    assert(duplicate_blocked);

    round.reset();
    assert(round.state().current_hole_index == 0u);
    assert(round.state().holes_completed == 0u);
    assert(round.state().total_strokes == 0u);
    assert(round.state().cumulative_par == 0u);
    assert(round.state().relative_to_par == 0);
    assert(!round.state().round_complete);

    return 0;
}
