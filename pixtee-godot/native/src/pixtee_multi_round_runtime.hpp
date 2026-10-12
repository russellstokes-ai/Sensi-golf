#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include "original_turn_orchestrator.hpp"
#include "pixtee_round_runtime.hpp"

namespace pixtee {
// A host safety gate, NOT a recovered claim about the original UI tick
// scheduler. A pending hazard, cup transition, special green or unsupported
// dynamic collision cannot be abandoned merely because the original
// next-player selector returned an index.
inline constexpr bool host_turn_handoff_phase_allowed(
    sensigolf::HoleSessionPhase phase) noexcept {
    return phase==sensigolf::HoleSessionPhase::ReadyForShot
        || phase==sensigolf::HoleSessionPhase::HoleScored;
}
// Portable multi-player *session isolation* using native recovered shot,
// round, score and replay controllers for each participant. Turn choice is
// delegated to the original source-verified selector. Host must supply
// original candidate metrics. This does NOT invent tournament/match scoring
// or original game-wide turn scheduling.
class PixteeMultiRoundRuntime final {
public:
    PixteeMultiRoundRuntime(
        std::uint8_t participants,std::array<std::uint8_t,18> hole_order,
        std::array<std::uint8_t,100> par_by_resource,
        std::array<sensigolf::recovered::OriginalPrng16,4> seeds);
    std::size_t participants() const noexcept {return players_.size();}
    void register_course(
        std::uint8_t resource_id,
        const sensigolf::ClassicCourseResources& course);
    void load_next_hole_for_everyone();
    void propose_original_turn(const OriginalTurnRequest& req);
    std::uint8_t acknowledge_original_turn();
    bool turn_switch_pending() const noexcept {return turn_.has_pending_turn();}
    bool has_active_owner() const noexcept {return current_owner_.has_value();}
    std::uint8_t active_owner() const;
    PixteeRoundRuntime& active_round();
    const PixteeRoundRuntime& player_round(std::size_t player) const;
    PixteeRoundRuntime& player_round(std::size_t player);
    void reset_owner() noexcept;
private:
    std::vector<std::unique_ptr<PixteeRoundRuntime>> players_{};
    OriginalTurnOrchestrator turn_{};
    std::optional<std::uint8_t> current_owner_{};
};
} // namespace pixtee
