#pragma once
#include <cstdint>
#include <stdexcept>
#include "original_competition_turn.hpp"
#include "original_next_player.hpp"

namespace pixtee {
// Original-source decision *integration* across the two machine-verified
// turn routes. No predicted match outcomes, CPU skills, handicap or shots.
// Host must supply authentic player records and ball metrics; no guessing.
struct OriginalTurnRequest {
    std::uint8_t original_mode = 0;
    OriginalPlayerChoiceInput ordinary{};
    OriginalCompetitionTurnInput competition{};
};
struct OriginalTurnDecision {
    std::uint8_t active_player = 0;
    bool used_competition_controller = false;
    bool actual_player_state_copied = false; // NOT part of this source scope
    int active_ball_array = -1;
    int source_player = -1;
    std::uint16_t next_side = 0;
};
OriginalTurnDecision original_decide_next_turn(
    const OriginalTurnRequest& request);

class OriginalTurnOrchestrator {
public:
    void select_next(const OriginalTurnRequest& request);
    bool has_pending_turn() const noexcept { return pending_; }
    const OriginalTurnDecision& pending_turn() const;
    // Only acknowledge after the host has actually changed ownership.
    // No hidden turn side effects until explicit confirmation.
    OriginalTurnDecision acknowledge_owner_switch();
    void reset() noexcept;
private:
    OriginalTurnDecision next_{};
    bool pending_ = false;
};
} // namespace pixtee
