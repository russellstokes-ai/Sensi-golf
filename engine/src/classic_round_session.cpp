#include "sensigolf/classic_round_session.hpp"

#include <cstdint>
#include <stdexcept>

namespace sensigolf {
namespace {

std::int16_t signed_word(std::uint16_t value) noexcept {
    const auto signed_value =
        (value & 0x8000u) != 0u
        ? static_cast<std::int32_t>(value) - 0x10000
        : static_cast<std::int32_t>(value);
    return static_cast<std::int16_t>(signed_value);
}

} // namespace

void ClassicRoundSession::reset() noexcept {
    state_ = {};
}

void ClassicRoundSession::accept_scored_hole(
    const ClassicHoleSession& hole) {
    if (state_.round_complete) {
        throw std::logic_error("classic round is already complete");
    }
    if (hole.phase() != HoleSessionPhase::HoleScored) {
        throw std::logic_error(
            "round progression requires a scored hole");
    }

    const auto metadata = hole.hole_metadata();
    const auto next = hole.next_hole_index();
    if (!metadata || !next) {
        throw std::logic_error(
            "scored hole is missing recovered progression metadata");
    }
    if (metadata->hole_index != state_.current_hole_index) {
        throw std::logic_error(
            "scored hole does not match current round hole");
    }
    if (*next != state_.current_hole_index + 1u) {
        throw std::logic_error(
            "scored hole has inconsistent recovered next-hole state");
    }
    if (hole.round_complete() != (*next == 18u)) {
        throw std::logic_error(
            "scored hole has inconsistent recovered round-complete state");
    }

    const auto& counters = hole.recovered_counters();
    if (counters.player_70 != 1u) {
        throw std::logic_error(
            "scored hole has unexpected completion counter");
    }

    // A ClassicHoleSession is reset for one physical hole. Aggregate its
    // recovered score contribution into the cross-hole state using original
    // 16-bit arithmetic.
    state_.total_strokes = static_cast<std::uint16_t>(
        state_.total_strokes + counters.player_56);
    state_.cumulative_par = static_cast<std::uint16_t>(
        state_.cumulative_par + counters.player_58);
    state_.relative_to_par = signed_word(
        static_cast<std::uint16_t>(
            state_.cumulative_par - state_.total_strokes));
    state_.holes_completed = static_cast<std::uint16_t>(
        state_.holes_completed + 1u);

    // The physical hole session already executed the recovered 0x40935B
    // ownership rule. Preserve that exact result at round scope.
    state_.current_hole_index = *next;
    state_.round_complete = hole.round_complete();
}

const ClassicRoundState& ClassicRoundSession::state() const noexcept {
    return state_;
}

} // namespace sensigolf
