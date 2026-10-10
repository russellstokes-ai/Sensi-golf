#include <array>
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

    std::array<std::uint8_t, kClassicRoundHoleCount> order{};
    std::array<std::uint8_t, kClassicParTableSize> pars{};
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = static_cast<std::uint8_t>(i + 1u);
        pars[i + 1u] = 4u;
    }
    ClassicHolePlan plan(order, pars);

    assert(round.state().current_hole_index == 0u);
    assert(!round.state().round_complete);
    const auto first_request = round.current_hole_request(plan);
    assert(first_request.has_value());
    assert(first_request->round_index == 0u);
    assert(first_request->resource_id == 1u);
    assert(first_request->par == 4u);
    assert(first_request->resources.mapm_map == "mapm01.map");

    for (std::uint16_t index = 0; index < 18u; ++index) {
        const auto request = round.current_hole_request(plan);
        assert(request.has_value());
        assert(request->round_index == index);

        ClassicHoleSession hole(
            course,
            0,
            0,
            ClassicHoleMetadata{
                index,
                request->par,
                request->resource_id});
        score_one_putt_hole(hole);
        round.accept_scored_hole(hole, plan);

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
    assert(!round.current_hole_request(plan).has_value());

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

    // Plan-aware acceptance rejects a correct round index with wrong original
    // resource metadata instead of silently advancing to the wrong MAPM/MAPS.
    ClassicHoleSession wrong_resource(
        course,
        0,
        0,
        ClassicHoleMetadata{0, 4, 2});
    score_one_putt_hole(wrong_resource);
    bool wrong_resource_blocked = false;
    try {
        round.accept_scored_hole(wrong_resource, plan);
    } catch (const std::logic_error&) {
        wrong_resource_blocked = true;
    }
    assert(wrong_resource_blocked);
    assert(round.state().current_hole_index == 0u);

    ClassicHoleSession wrong_par(
        course,
        0,
        0,
        ClassicHoleMetadata{0, 5, 1});
    score_one_putt_hole(wrong_par);
    bool wrong_par_blocked = false;
    try {
        round.accept_scored_hole(wrong_par, plan);
    } catch (const std::logic_error&) {
        wrong_par_blocked = true;
    }
    assert(wrong_par_blocked);
    assert(round.state().current_hole_index == 0u);

    round.reset();
    assert(round.state().current_hole_index == 0u);
    assert(round.state().holes_completed == 0u);
    assert(round.state().total_strokes == 0u);
    assert(round.state().cumulative_par == 0u);
    assert(round.state().relative_to_par == 0);
    assert(!round.state().round_complete);

    return 0;
}
