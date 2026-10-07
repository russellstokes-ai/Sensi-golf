#include "sensigolf/classic_hole_session.hpp"

#include <limits>
#include <stdexcept>

namespace sensigolf {

ClassicHoleSession::ClassicHoleSession(
    const ClassicCourseResources& course,
    std::int32_t start_x_raw,
    std::int32_t start_y_raw)
    : course_(course) {
    reset(start_x_raw, start_y_raw);
}

void ClassicHoleSession::reset(
    std::int32_t start_x_raw,
    std::int32_t start_y_raw) {
    shot_.reset();
    phase_ = HoleSessionPhase::ReadyForShot;
    strokes_ = 0;
    ball_x_raw_ = start_x_raw;
    ball_y_raw_ = start_y_raw;
    active_club_ = 0;
    unsupported_descriptor_.reset();
}

ClassicSurfaceContext ClassicHoleSession::to_context(
    const ResolvedCourseSurface& surface) const noexcept {
    return ClassicSurfaceContext{
        surface.landing_code,
        surface.slope_direction,
        surface.slope_magnitude,
    };
}

bool ClassicHoleSession::surface_supported_for_active_shot(
    const ResolvedCourseSurface& surface) const noexcept {
    if (!surface.product_supported) {
        return false;
    }

    // Gate-1 putter parity currently covers the GREEN H4 / code-1 path.
    // Entering the hole/special-green descriptors while putting is not yet
    // wired into ClassicShotModel, so stop explicitly rather than approximate.
    if (active_club_ == 12) {
        return surface.landing_code == 1;
    }

    // Conversely, the non-putter path has not yet been parity-integrated with
    // GREEN H4's green-specific drag/slope semantics.
    return surface.landing_code != 1;
}

void ClassicHoleSession::begin_shot(const ClassicShotRequest& request) {
    if (phase_ != HoleSessionPhase::ReadyForShot) {
        throw std::logic_error("hole session is not ready for a new shot");
    }

    const auto surface = course_.resolve_raw_position(
        ball_x_raw_, ball_y_raw_);

    active_club_ = request.club_index;
    if (!surface_supported_for_active_shot(surface)) {
        unsupported_descriptor_ = surface.descriptor_index;
        phase_ = HoleSessionPhase::UnsupportedTerrain;
        throw std::runtime_error(
            "current terrain requires a classic rule not yet product-integrated");
    }

    shot_.reset();
    shot_.set_ball_origin(ball_x_raw_, ball_y_raw_);
    shot_.set_surface_context(to_context(surface));

    ShotInput input{};
    input.aim_raw = request.aim_raw;
    input.power_tick = request.power_tick;
    input.accuracy_tick = request.accuracy_tick;
    input.club_index = request.club_index;
    input.lie_index = surface.profile_slot;

    shot_.begin_shot(input);
    ++strokes_;
    phase_ = HoleSessionPhase::ShotActive;
    unsupported_descriptor_.reset();
}

void ClassicHoleSession::step() {
    if (phase_ != HoleSessionPhase::ShotActive) {
        return;
    }

    const auto& state = shot_.state();
    if (state.x_raw < std::numeric_limits<std::int32_t>::min()
        || state.x_raw > std::numeric_limits<std::int32_t>::max()
        || state.y_raw < std::numeric_limits<std::int32_t>::min()
        || state.y_raw > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error("classic ball coordinate outside recovered 32-bit range");
    }

    const auto surface = course_.resolve_raw_position(
        static_cast<std::int32_t>(state.x_raw),
        static_cast<std::int32_t>(state.y_raw));

    if (!surface_supported_for_active_shot(surface)) {
        unsupported_descriptor_ = surface.descriptor_index;
        phase_ = HoleSessionPhase::UnsupportedTerrain;
        return;
    }

    shot_.update_surface_context_for_tick(to_context(surface));
    shot_.step();

    if (shot_.shot_active()) {
        return;
    }

    const auto& final = shot_.state();
    ball_x_raw_ = static_cast<std::int32_t>(final.x_raw);
    ball_y_raw_ = static_cast<std::int32_t>(final.y_raw);

    if (final.holed) {
        phase_ = HoleSessionPhase::HoleComplete;
    } else if (final.hazard) {
        phase_ = HoleSessionPhase::HazardStopped;
    } else {
        phase_ = HoleSessionPhase::ReadyForShot;
    }
}

HoleSessionPhase ClassicHoleSession::phase() const noexcept {
    return phase_;
}

std::uint32_t ClassicHoleSession::strokes() const noexcept {
    return strokes_;
}

std::int32_t ClassicHoleSession::ball_x_raw() const noexcept {
    return ball_x_raw_;
}

std::int32_t ClassicHoleSession::ball_y_raw() const noexcept {
    return ball_y_raw_;
}

const BallState& ClassicHoleSession::ball_state() const noexcept {
    return shot_.state();
}

ResolvedCourseSurface ClassicHoleSession::current_surface() const {
    const auto& state = shot_.state();
    if (phase_ == HoleSessionPhase::ShotActive
        || phase_ == HoleSessionPhase::UnsupportedTerrain) {
        return course_.resolve_raw_position(
            static_cast<std::int32_t>(state.x_raw),
            static_cast<std::int32_t>(state.y_raw));
    }
    return course_.resolve_raw_position(ball_x_raw_, ball_y_raw_);
}

std::optional<std::uint16_t>
ClassicHoleSession::unsupported_descriptor() const noexcept {
    return unsupported_descriptor_;
}

} // namespace sensigolf
