#pragma once
#include <array>
#include <cstdint>
namespace pixtee {
// Sensible Golf v1.014 mode-1 player score correction branch at 0x40C005
// through 0x40C098. A source-verified isolated *turn-adjustment* component,
// NOT all competition outcomes, per-player holes or tournament scoring.
struct OriginalTurnScores {
    std::uint8_t count=1;
    std::array<std::uint16_t,4> strokes{};
    std::array<std::uint16_t,4> total_strokes{};
    std::array<std::uint16_t,4> completed_marker{}; // player+0x5e
    std::array<std::uint32_t,4> ball_distance{}; // ball+0x18
};
struct OriginalTurnScoreAdjustment {
    OriginalTurnScores result{};
    int corrected_index=-1; // -1 if x86 falls through
};
OriginalTurnScoreAdjustment original_turn_score_adjust(
    const OriginalTurnScores& source);
}
