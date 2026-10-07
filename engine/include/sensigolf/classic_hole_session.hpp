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
    SpecialGreenStopped,
    HoleComplete,
    UnsupportedTerrain,
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
        std::int32_t start_y_raw);

    ClassicHoleSession(
        const ClassicCourseResources& course,
        std::size_t player_slot);

    void reset(
        std::int32_t start_x_raw,
        std::int32_t start_y_raw);

    void begin_shot(const ClassicShotRequest& request);
    void step();

    HoleSessionPhase phase() const noexcept;
    std::uint32_t strokes() const noexcept;
    std::int32_t ball_x_raw() const noexcept;
    std::int32_t ball_y_raw() const noexcept;
    const BallState& ball_state() const noexcept;

    ResolvedCourseSurface current_surface() const;
    ClassicCoursePoint hole_position() const;
    std::optional<std::uint16_t> unsupported_descriptor() const noexcept;

private:
    bool surface_supported_for_active_shot(
        const ResolvedCourseSurface& surface) const noexcept;
    ClassicSurfaceContext to_context(
        const ResolvedCourseSurface& surface) const noexcept;

    const ClassicCourseResources& course_;
    ClassicShotModel shot_;
    HoleSessionPhase phase_ = HoleSessionPhase::ReadyForShot;
    std::uint32_t strokes_ = 0;
    std::int32_t ball_x_raw_ = 0;
    std::int32_t ball_y_raw_ = 0;
    std::uint16_t active_club_ = 0;
    std::optional<std::uint16_t> unsupported_descriptor_{};
};

} // namespace sensigolf
