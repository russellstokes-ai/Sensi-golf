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

    const auto& counters = hole.recovered_counters();
    if (counters.player_70 == 0u) {
        throw std::logic_error(
            "scored hole is missing recovered completion state");
    }

    // Each ClassicHoleSession owns one hole, so its recovered +0x56 and +0x58
    // values are the completed hole's stroke and par contributions. Aggregate
    // them into the round-level equivalents using original 16-bit wrapping.
    state_.total_strokes = static_cast<std::uint16_t>(
        state_.total_strokes + counters.player_56);
    state_.cumulative_par = static_cast<std::uint16_t>(
        state_.cumulative_par + counters.player_58);
    state_.relative_to_par = signed_word(
        static_cast<std::uint16_t>(
            state_.cumulative_par - state_.total_strokes));
    state_.holes_completed = static_cast<std::uint16_t>(
        state_.holes_completed + 1u);

    // Exact ownership recovered at Windows v1.014 0x40935B:
    // increment current hole, then set the finish state when it equals 18.
    state_.current_hole_index += 1u;
    state_.round_complete = state_.current_hole_index == 18u;
}

const ClassicRoundState& ClassicRoundSession::state() const noexcept {
    return state_;
}

} // namespace sensigolf
