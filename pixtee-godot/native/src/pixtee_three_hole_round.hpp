#pragma once
#include <array>
#include <cstdint>
#include <optional>

#include "pixtee_round_runtime.hpp"

namespace pixtee {
// Product-layer *three-hole preview* stopping policy only.
// Preserves untouched recovered source-authentic 18-slot ClassicGameSession;
// never claims that Sensible Golf had an original three-hole game mode.
// Only three distinct, independently authored course IDs are accessible.
class PixteeThreeHoleRound final {
public:
    static constexpr std::size_t kPreviewHoles = 3;
    PixteeThreeHoleRound(
        std::array<std::uint8_t, kPreviewHoles> ids,
        std::array<std::uint8_t, kPreviewHoles> pars,
        std::uint16_t seed0, std::uint16_t seed1);

    void register_course(
        std::uint8_t id, const sensigolf::ClassicCourseResources& course);
    std::optional<sensigolf::ClassicHolePlanEntry> next_hole() const;
    void load_next_hole();
    void commit_scored_hole();

    void set_aim_raw(std::uint16_t heading);
    void nudge_aim_raw(std::int32_t delta);
    void set_club(std::uint16_t index);
    void cycle_club(int delta);
    void meter_click_raw(std::uint16_t sample);
    void start_original_welly();
    bool tick_original_welly(bool pressed);
    void step_tick();
    void acknowledge_hazard();
    void acknowledge_special_green();

    const sensigolf::ClassicHoleSession& hole() const;
    const sensigolf::ClassicRoundState& underlying_original_round() const noexcept;
    bool complete() const noexcept;
    std::uint16_t holes_completed() const noexcept;
    std::string serialize_stable_replay() const;

private:
    std::array<std::uint8_t, kPreviewHoles> ids_{};
    PixteeRoundRuntime round_;
};
}  // namespace pixtee
