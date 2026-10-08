#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "sensigolf/classic_game_session.hpp"
#include "sensigolf/recovered_prng.hpp"

using namespace sensigolf;

namespace {

void be16(
    std::vector<std::uint8_t>& data,
    std::size_t off,
    std::uint16_t value) {
    data[off] = static_cast<std::uint8_t>(value >> 8);
    data[off + 1] = static_cast<std::uint8_t>(value & 0xFF);
}

ClassicCourseResources cup_course(
    std::uint16_t tee_x = 0,
    std::uint16_t tee_y = 0) {
    constexpr std::uint16_t width = 4;
    constexpr std::uint16_t height = 128;

    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes
        + static_cast<std::size_t>(width) * height * 2u,
        0);
    be16(mapm, 0x54, width);
    be16(mapm, 0x56, height);

    std::vector<std::uint8_t> spt(50, 0);
    be16(spt, 4, tee_x);
    be16(spt, 6, tee_y);
    // Place the synthetic cup at the same position as the tee so the recovered
    // zero-distance scored-hole path can be exercised deterministically.
    be16(spt, 44, tee_x);
    be16(spt, 46, tee_y);

    std::vector<std::uint8_t> desc(8, 0);
    std::vector<std::uint8_t> sel(8, 0);
    be16(desc, 0, 7); // GREEN H1 / landing code 8

    return ClassicCourseResources(
        std::move(mapm), std::move(spt), std::move(desc), std::move(sel));
}

ClassicHolePlan plan() {
    std::array<std::uint8_t, kClassicRoundHoleCount> order{};
    std::array<std::uint8_t, kClassicParTableSize> pars{};
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = static_cast<std::uint8_t>(i + 1u);
        pars[i + 1u] = 4u;
    }
    return ClassicHolePlan(order, pars);
}

ClassicShotRequest putter_request() {
    ClassicShotRequest putt{};
    putt.club_index = 12;
    putt.power_tick = 30;
    putt.accuracy_tick = 63;
    putt.aim_raw = 0;
    return putt;
}

void score_active_hole(ClassicGameSession& game) {
    auto& hole = game.active_hole();
    hole.begin_shot(putter_request());

    int ticks = 0;
    while (hole.phase() == HoleSessionPhase::ShotActive
           && ticks++ < 32) {
        hole.step();
    }
    assert(ticks < 32);
    assert(hole.phase() == HoleSessionPhase::CupTerminal);
    hole.step();
    assert(hole.phase() == HoleSessionPhase::HoleScored);
}

} // namespace

int main() {
    ClassicGameSession game(plan());

    auto request = game.resource_request();
    assert(request.has_value());
    assert(request->round_index == 0u);
    assert(request->resource_id == 1u);
    assert(request->resources.mapm_map == "mapm01.map");
    assert(!game.has_active_hole());

    bool wrong_resource_blocked = false;
    try {
        game.load_current_hole(2u, cup_course());
    } catch (const std::logic_error&) {
        wrong_resource_blocked = true;
    }
    assert(wrong_resource_blocked);
    assert(!game.has_active_hole());

    // Walk the complete round through the controller boundary. Each commit
    // destroys the old course/session before exposing the next resource request.
    for (std::uint16_t index = 0; index < 18u; ++index) {
        request = game.resource_request();
        assert(request.has_value());
        assert(request->round_index == index);
        assert(request->resource_id == index + 1u);

        game.load_current_hole(
            request->resource_id,
            cup_course());
        assert(game.has_active_hole());
        assert(game.active_hole().hole_metadata().has_value());
        assert(game.active_hole().hole_metadata()->resource_id
            == request->resource_id);

        score_active_hole(game);
        game.commit_scored_hole();

        assert(!game.has_active_hole());
        assert(game.round_state().holes_completed == index + 1u);
    }

    assert(game.round_state().round_complete);
    assert(game.round_state().current_hole_index == 18u);
    assert(!game.resource_request().has_value());

    bool after_round_blocked = false;
    try {
        game.load_current_hole(1u, cup_course());
    } catch (const std::logic_error&) {
        after_round_blocked = true;
    }
    assert(after_round_blocked);


    // The game session owns the captured original PRNG state across hole
    // boundaries. This keeps deterministic classic randomness in the core
    // rather than in a platform host.
    ClassicGameSession seeded(
        plan(), 0, recovered::OriginalPrng16{0x1234u, 0xABCDu});
    assert(seeded.has_prng_state());
    assert(seeded.prng_state().seed0 == 0x1234u);
    assert(seeded.prng_state().seed1 == 0xABCDu);
    const auto first_random = seeded.prng_state().next(0x0200u);
    assert(first_random == 0x0075u);
    const auto advanced_seed0 = seeded.prng_state().seed0;
    const auto advanced_seed1 = seeded.prng_state().seed1;

    auto seeded_request = seeded.resource_request();
    assert(seeded_request.has_value());
    seeded.load_current_hole(
        seeded_request->resource_id,
        cup_course());
    score_active_hole(seeded);
    seeded.commit_scored_hole();
    assert(seeded.prng_state().seed0 == advanced_seed0);
    assert(seeded.prng_state().seed1 == advanced_seed1);

    // Starting the same session again restores the captured deterministic
    // baseline instead of inheriting platform/global random state.
    seeded.reset();
    assert(seeded.prng_state().seed0 == 0x1234u);
    assert(seeded.prng_state().seed1 == 0xABCDu);

    game.reset();
    assert(!game.round_state().round_complete);
    request = game.resource_request();
    assert(request.has_value());
    assert(request->resource_id == 1u);

    return 0;
}
