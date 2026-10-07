#include "sensigolf/classic_hole_session.hpp"

#include <limits>
#include <stdexcept>

#include "sensigolf/recovered_hazard_recovery.hpp"

namespace sensigolf {

ClassicHoleSession::ClassicHoleSession(
    const ClassicCourseResources& course,
    std::int32_t start_x_raw,
    std::int32_t start_y_raw)
    : course_(course) {
    reset(start_x_raw, start_y_raw);
}

ClassicHoleSession::ClassicHoleSession(
    const ClassicCourseResources& course,
    std::size_t player_slot)
    : course_(course) {
    const auto start = course_.player_start(player_slot);
    reset(start.x_raw(), start.y_raw());
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
    safe_anchor_x_raw_ = 0;
    safe_anchor_y_raw_ = 0;
    safe_anchor_valid_ = false;
    hazard_pause_remaining_ = 0;
    recovered_counters_ = {};
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
    if (active_club_ == 12) {
        return surface.landing_code == 1
            || surface.landing_code == 8
            || surface.landing_code == 9
            || surface.landing_code == 10;
    }

    if (!surface.product_supported) {
        return false;
    }

    // GREEN H4 uses putter-specific green drag/slope semantics.
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

    // Original live launch increments both recovered player counters.
    recovered_counters_.player_52 = static_cast<std::uint16_t>(
        recovered_counters_.player_52 + 1u);
    recovered_counters_.player_56 = static_cast<std::uint16_t>(
        recovered_counters_.player_56 + 1u);

    ++strokes_;
    phase_ = HoleSessionPhase::ShotActive;
    unsupported_descriptor_.reset();
}

void ClassicHoleSession::step() {
    if (phase_ == HoleSessionPhase::HazardStopped) {
        if (hazard_pause_remaining_ > 0) {
            --hazard_pause_remaining_;
            return;
        }

        recovered::HazardRecoveryInput input{};
        input.ball_x = ball_x_raw_;
        input.ball_y = ball_y_raw_;
        input.safe_anchor_x = safe_anchor_x_raw_;
        input.safe_anchor_y = safe_anchor_y_raw_;
        input.x_extent = course_.recovery_x_extent();
        input.y_extent = course_.recovery_y_extent();
        input.green_mode = false;

        const auto recovered_state =
            recovered::recover_hazard_position(input);

        if (recovered_state.used_safe_anchor && !safe_anchor_valid_) {
            phase_ = HoleSessionPhase::UnsupportedTerrain;
            return;
        }

        ball_x_raw_ = recovered_state.ball_x;
        ball_y_raw_ = recovered_state.ball_y;

        const auto recovered_surface = course_.resolve_raw_position(
            ball_x_raw_, ball_y_raw_);
        shot_.relocate_inactive_ball(
            ball_x_raw_,
            ball_y_raw_,
            recovered_surface.landing_code);

        phase_ = HoleSessionPhase::HazardRecovered;
        return;
    }

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

    // Original 0x40A43B..0x40A453 refreshes the safe recovery anchor whenever
    // the current landing code is <= 6. The stored value is ball position -15
    // integer units in recovered 16.16 coordinates.
    if (surface.landing_code <= 6) {
        safe_anchor_x_raw_ = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(state.x_raw) - 0x000F0000u);
        safe_anchor_y_raw_ = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(state.y_raw) - 0x000F0000u);
        safe_anchor_valid_ = true;
    }

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

    switch (shot_.outcome()) {
    case ClassicShotOutcome::CupTerminal:
        // Scoped original putter code-8 terminal increments both counters
        // once more. Non-putter cup counter semantics remain separately scoped.
        if (active_club_ == 12) {
            recovered_counters_.player_52 = static_cast<std::uint16_t>(
                recovered_counters_.player_52 + 1u);
            recovered_counters_.player_56 = static_cast<std::uint16_t>(
                recovered_counters_.player_56 + 1u);
        }
        phase_ = HoleSessionPhase::CupTerminal;
        break;
    case ClassicShotOutcome::Holed:
        phase_ = HoleSessionPhase::HoleScored;
        break;
    case ClassicShotOutcome::Hazard:
        phase_ = HoleSessionPhase::HazardStopped;
        hazard_pause_remaining_ = 100;
        break;
    case ClassicShotOutcome::SpecialGreenStop:
        phase_ = HoleSessionPhase::SpecialGreenStopped;
        break;
    case ClassicShotOutcome::Rest:
    case ClassicShotOutcome::None:
        phase_ = HoleSessionPhase::ReadyForShot;
        break;
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
        || phase_ == HoleSessionPhase::UnsupportedTerrain
        || phase_ == HoleSessionPhase::SpecialGreenStopped) {
        return course_.resolve_raw_position(
            static_cast<std::int32_t>(state.x_raw),
            static_cast<std::int32_t>(state.y_raw));
    }
    return course_.resolve_raw_position(ball_x_raw_, ball_y_raw_);
}

ClassicCoursePoint ClassicHoleSession::hole_position() const {
    return course_.hole_position();
}

std::optional<std::uint16_t>
ClassicHoleSession::unsupported_descriptor() const noexcept {
    return unsupported_descriptor_;
}

std::uint16_t ClassicHoleSession::hazard_pause_remaining() const noexcept {
    return hazard_pause_remaining_;
}

const ClassicRecoveredCounters&
ClassicHoleSession::recovered_counters() const noexcept {
    return recovered_counters_;
}

} // namespace sensigolf
