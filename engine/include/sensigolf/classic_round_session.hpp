#pragma once

#include <cstdint>

#include "sensigolf/classic_hole_session.hpp"

namespace sensigolf {

struct ClassicRoundState {
    // Original current-hole index is zero-based. Recovered v1.014 progression
    // advances it to 18 to signal completion of an 18-hole round.
    std::uint32_t current_hole_index = 0;
    std::uint16_t holes_completed = 0;
    std::uint16_t total_strokes = 0;
    std::uint16_t cumulative_par = 0;
    std::int16_t relative_to_par = 0;
    bool round_complete = false;
};

// Single-player Gate-2 owner for state that must survive between physical
// ClassicHoleSession instances.
class ClassicRoundSession {
public:
    void reset() noexcept;
    void accept_scored_hole(const ClassicHoleSession& hole);

    const ClassicRoundState& state() const noexcept;

private:
    ClassicRoundState state_{};
};

} // namespace sensigolf
