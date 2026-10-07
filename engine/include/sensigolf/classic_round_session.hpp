#pragma once

#include <cstdint>
#include <optional>

#include "sensigolf/classic_hole_plan.hpp"
#include "sensigolf/classic_hole_session.hpp"

namespace sensigolf {

struct ClassicRoundState {
    std::uint32_t current_hole_index = 0;
    std::uint16_t holes_completed = 0;
    std::uint16_t total_strokes = 0;
    std::uint16_t cumulative_par = 0;
    std::int16_t relative_to_par = 0;
    bool round_complete = false;
};

class ClassicRoundSession {
public:
    void reset() noexcept;
    void accept_scored_hole(const ClassicHoleSession& hole);
    void accept_scored_hole(
        const ClassicHoleSession& hole,
        const ClassicHolePlan& plan);

    std::optional<ClassicHolePlanEntry> current_hole_request(
        const ClassicHolePlan& plan) const;

    const ClassicRoundState& state() const noexcept;

private:
    void accept_scored_hole_impl(
        const ClassicHoleSession& hole,
        const ClassicHolePlan* plan);

    ClassicRoundState state_{};
};

} // namespace sensigolf
