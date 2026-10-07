#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_shot_model.hpp"

namespace sensigolf {

enum class HoleSessionPhase : std::uint8_t {
    ReadyForShot = 0,
    ShotActive,
    HazardStopped,
    HazardRecovered,
    SpecialGreenStopped,
    CupTerminal,
    HoleScored,
    UnsupportedTerrain,
};

struct ClassicRecoveredCounters {
    // Original v1.014 player fields with their now-proven single-player roles.
    std::uint16_t player_52 = 0; // current-hole stroke counter
    std::uint16_t player_56 = 0; // cumulative stroke total
    std::uint16_t player_58 = 0; // cumulative par
    std::int16_t player_48 = 0;  // cumulative par - cumulative strokes
    std::uint16_t player_70 = 0; // completed-hole counter
};

struct ClassicHoleMetadata {
    std::uint16_t hole_index = 0; // original zero-based 0..17 index
    std::uint16_t par = 0;
};

struct ClassicShotRequest {
    RawScalar aim_raw = 0;
    std::int32_t power_tick = 0;
    std::int32_t accuracy_tick = 0;
    std::uint16_t club_index = 0;
};

class ClassicHoleSession {
public:
    ClassicHoleSession(
        const ClassicCourseResources& course,
        std::int32_t start_x_raw,
        std::int32_t start_y_raw,
        std::optional<ClassicHoleMetadata> metadata = std::nullopt);

    ClassicHoleSession(
        const ClassicCourseResources& course,
        std::size_t player_slot,
        std::optional<ClassicHoleMetadata> metadata = std::nullopt);

    void reset(
        std::int32_t start_x_raw,
        std::int32_t start_y_raw);

    void begin_shot(const ClassicShotRequest& request);
    void step();

    // The original clears the post-hazard terminal flag when its presentation
    // objects finish. Rendering/presentation owns that timing in the port;
    // this explicit handoff releases the recovered gameplay state for input.
    void acknowledge_hazard_recovery();

    HoleSessionPhase phase() const noexcept;
    std::uint32_t strokes() const noexcept;
    std::int32_t ball_x_raw() const noexcept;
    std::int32_t ball_y_raw() const noexcept;
    const BallState& ball_state() const noexcept;

    ResolvedCourseSurface current_surface() const;
    ClassicCoursePoint hole_position() const;
    std::optional<std::uint16_t> unsupported_descriptor() const noexcept;
    std::uint16_t hazard_pause_remaining() const noexcept;
    const ClassicRecoveredCounters& recovered_counters() const noexcept;
    std::uint32_t distance_to_hole() const noexcept;
    std::optional<ClassicHoleMetadata> hole_metadata() const noexcept;
    std::optional<std::uint16_t> next_hole_index() const noexcept;
    bool round_complete() const noexcept;

private:
    bool surface_supported_for_active_shot(
        const ResolvedCourseSurface& surface) const noexcept;
    ClassicSurfaceContext to_context(
        const ResolvedCourseSurface& surface) const noexcept;
    void complete_scored_putter_hole();

    const ClassicCourseResources& course_;
    ClassicShotModel shot_;
    HoleSessionPhase phase_ = HoleSessionPhase::ReadyForShot;
    std::uint32_t strokes_ = 0;
    std::int32_t ball_x_raw_ = 0;
    std::int32_t ball_y_raw_ = 0;
    std::uint16_t active_club_ = 0;
    std::int32_t safe_anchor_x_raw_ = 0;
    std::int32_t safe_anchor_y_raw_ = 0;
    bool safe_anchor_valid_ = false;
    std::uint16_t hazard_pause_remaining_ = 0;
    ClassicRecoveredCounters recovered_counters_{};
    std::optional<ClassicHoleMetadata> hole_metadata_{};
    std::optional<std::uint16_t> next_hole_index_{};
    bool round_complete_ = false;
    std::optional<std::uint16_t> unsupported_descriptor_{};
};

} // namespace sensigolf
