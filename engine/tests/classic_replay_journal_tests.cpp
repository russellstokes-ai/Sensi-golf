#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "sensigolf/classic_replay_journal.hpp"

using namespace sensigolf;

namespace {

void be16(std::vector<std::uint8_t>& data, std::size_t offset,
          std::uint16_t value) {
    data[offset] = static_cast<std::uint8_t>(value >> 8);
    data[offset + 1] = static_cast<std::uint8_t>(value);
}

ClassicCourseResources cup_course() {
    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes + 4u * 128u * 2u, 0);
    be16(mapm, 0x54, 4);
    be16(mapm, 0x56, 128);
    std::vector<std::uint8_t> spt(50, 0);
    be16(spt, 4, 10); be16(spt, 6, 20);
    be16(spt, 44, 10); be16(spt, 46, 20);
    std::vector<std::uint8_t> desc(8, 0), select(8, 0);
    be16(desc, 0, 7); // code-8 synthetic putter terminal
    return ClassicCourseResources(
        std::move(mapm), std::move(spt),
        std::move(desc), std::move(select));
}

ClassicHolePlan original_style_plan(bool wrong_first = false) {
    std::array<std::uint8_t, kClassicRoundHoleCount> order{};
    std::array<std::uint8_t, kClassicParTableSize> par{};
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = static_cast<std::uint8_t>(i + 1u);
        par[i+1] = 4u;
    }
    if (wrong_first) order[0] = 3;
    return ClassicHolePlan(order, par);
}

template<typename Fn>
void rejected(Fn&& fn) {
    bool threw = false;
    try { fn(); } catch (const std::exception&) { threw = true; }
    assert(threw);
}

void score(ClassicGameSession& game, ClassicReplayJournal& journal) {
    const auto req = game.resource_request();
    assert(req.has_value());
    game.load_current_hole(req->resource_id, cup_course());
    ClassicShotRequest shot{0, 30, 63, 12};
    auto& hole = game.active_hole();
    hole.begin_shot(shot);
    int ticks = 0;
    while (hole.phase() == HoleSessionPhase::ShotActive && ticks++ < 128) {
        hole.step();
    }
    assert(ticks < 128);
    assert(hole.phase() == HoleSessionPhase::CupTerminal);
    hole.step();
    assert(hole.phase() == HoleSessionPhase::HoleScored);
    journal.append_after_shot(hole, shot);
    rejected([&] { journal.append_after_shot(hole, shot); });
    game.commit_scored_hole();
}

} // namespace

int main() {
    ClassicGameSession original(original_style_plan());
    ClassicReplayJournal journal;
    score(original, journal);
    score(original, journal);
    assert(original.round_state().holes_completed == 2);
    assert(original.round_state().total_strokes == 2);
    assert(journal.frames().size() == 2);

    const std::string saved = journal.serialize();
    assert(saved.find("SENSIGOLF_REPLAY 1") == 0);
    const auto restored = ClassicReplayJournal::parse(saved);
    assert(restored.frames().size() == 2);
    assert(restored.serialize() == saved);

    ClassicGameSession resumed(original_style_plan());
    restored.replay_into(resumed, [](std::uint8_t) {
        return cup_course();
    });
    assert(!resumed.has_active_hole());
    assert(resumed.round_state().holes_completed == 2);
    assert(resumed.round_state().total_strokes == 2);
    assert(resumed.round_state().cumulative_par == 8);
    assert(resumed.round_state().relative_to_par == 6);
    assert(resumed.resource_request()->resource_id == 3u);

    // A wrong original order may never be restored into a different course.
    ClassicGameSession wrong_plan(original_style_plan(true));
    rejected([&] {
        restored.replay_into(wrong_plan, [](std::uint8_t) {
            return cup_course();
        });
    });

    // Persisted text is versioned and corruption-checked, not blind state
    // injection. A cut-off save or altered numeric shot cannot be loaded.
    rejected([&] {
        ClassicReplayJournal::parse(saved.substr(0, saved.size() - 3));
    });
    auto corrupt = saved;
    const auto reading = corrupt.find(" 30 63 12 ");
    assert(reading != std::string::npos);
    corrupt[reading + 1] = '9';
    rejected([&] { ClassicReplayJournal::parse(corrupt); });
    auto version = saved;
    version.replace(17, 1, "9");
    rejected([&] { ClassicReplayJournal::parse(version); });

    // Only completed-shot boundaries may be recorded.
    auto course = cup_course();
    ClassicHoleSession unplayed(course, std::size_t{0},
                                ClassicHoleMetadata{0, 4, 1});
    ClassicReplayJournal empty;
    rejected([&] {
        empty.append_after_shot(unplayed, ClassicShotRequest{0, 30, 63, 12});
    });

    return 0;
}
