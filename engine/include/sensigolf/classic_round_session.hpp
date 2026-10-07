#pragma once

#include <cstdint>

#include "sensigolf/classic_hole_session.hpp"

namespace sensigolf {

struct ClassicRoundState {
    // Original current-hole index is zero-based and advances to 18 to signal
    // the end of an 18-hole round.
    std::uint32_t current_hole_index = 0;
    std::uint16_t holes_completed = 0;
    std::uint16_t total_strokes = 0;
    std::uint16_t cumulative_par = 0;
    std::int16_t relative_to_par = 0;
    bool round_complete = false;
};

// Single-player Gate-2 round ownership. Hole physics remains in
// ClassicHoleSession; this layer owns cross-hole score accumulation and the
// recovered 0x40935B next-hole transition.
class ClassicRoundSession {
public:
    void reset() noexcept;
    void accept_scored_hole(const ClassicHoleSession& hole);

    const ClassicRoundState& state() const noexcept;

private:
    ClassicRoundState state_{};
};

} // namespace sensigolf
