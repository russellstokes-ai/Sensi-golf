#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_hole_plan.hpp"
#include "sensigolf/classic_hole_session.hpp"
#include "sensigolf/classic_round_session.hpp"

namespace sensigolf {

// Platform-neutral owner for one classic single-player round.
//
// The platform is responsible only for satisfying resource requests. It passes
// the requested original MAPM/SPT+MAPI course bundle into load_current_hole();
// this class owns hole construction, score acceptance and the recovered
// next-hole transition. Android/iOS therefore do not reimplement gameflow.
class ClassicGameSession {
public:
    explicit ClassicGameSession(
        ClassicHolePlan plan,
        std::size_t player_slot = 0);

    const ClassicRoundState& round_state() const noexcept;
    std::optional<ClassicHolePlanEntry> resource_request() const;

    bool has_active_hole() const noexcept;
    ClassicHoleSession& active_hole();
    const ClassicHoleSession& active_hole() const;

    void load_current_hole(
        std::uint8_t resource_id,
        ClassicCourseResources course);

    // Accepts the already-scored active hole and advances the authoritative
    // round state. The next resource_request() becomes available immediately.
    void commit_scored_hole();

    void reset();

private:
    ClassicHolePlan plan_;
    std::size_t player_slot_ = 0;
    ClassicRoundSession round_{};
    std::unique_ptr<ClassicCourseResources> course_{};
    std::unique_ptr<ClassicHoleSession> hole_{};
};

} // namespace sensigolf
