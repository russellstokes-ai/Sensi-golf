#include "sensigolf/classic_hole_session.hpp"

#include <limits>
#include <stdexcept>

#include "sensigolf/recovered_distance.hpp"
#include "sensigolf/recovered_hazard_recovery.hpp"
#include "sensigolf/recovered_world_boundary.hpp"
#include "sensigolf/recovered_putter_terminal.hpp"

namespace sensigolf {

ClassicHoleSession::ClassicHoleSession(
    const ClassicCourseResources& course,
    std::int32_t start_x_raw,
    std::int32_t start_y_raw,
    std::optional<ClassicHoleMetadata> metadata,
    recovered::OriginalPrng16* prng)
    : course_(course),
      hole_metadata_(metadata),
      prng_(prng) {
    if (hole_metadata_
        && (hole_metadata_->hole_index >= 18
            || hole_metadata_->par == 0
            || hole_metadata_->resource_id >= 100)) {
        throw std::invalid_argument("classic hole metadata outside recovered range");
    }
    reset(start_x_raw, start_y_raw);
}

ClassicHoleSession::ClassicHoleSession(
    const ClassicCourseResources& course,
    std::size_t player_slot,
    std::optional<ClassicHoleMetadata> metadata,
    recovered::OriginalPrng16* prng)
    : course_(course),
      hole_metadata_(metadata),
      prng_(prng) {
    if (hole_metadata_
        && (hole_metadata_->hole_index >= 18
            || hole_metadata_->par == 0
            || hole_metadata_->resource_id >= 100)) {
        throw std::invalid_argument("classic hole metadata outside recovered range");
    }
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
    // A launch from a valid low-code lie can exit the map before its first
    // sampled terrain tick. Seed a safe landing anchor from the starting
    // lie to avoid an unrecoverable missing-anchor state. The -15 geometry
    // is source-original 0x40A441; tee/init timing is a host integration
    // safeguard pending original player-setup event parity.
    {
        recovered::OriginalWorldBoundaryInput start_bounds{};
        start_bounds.ball_x=start_x_raw;
        start_bounds.ball_y=start_y_raw;
        start_bounds.x_extent=course_.recovery_x_extent();
        start_bounds.y_extent=course_.recovery_y_extent();
        if(!recovered::original_world_boundary_check(start_bounds).outside_world) {
            const auto start=course_.resolve_raw_position(
                start_x_raw,start_y_raw,false);
            if(start.landing_code<=6u) {
                safe_anchor_x_raw_=static_cast<std::int32_t>(
                    static_cast<std::uint32_t>(start_x_raw)-0x000F0000u);
                safe_anchor_y_raw_=static_cast<std::int32_t>(
                    static_cast<std::uint32_t>(start_y_raw)-0x000F0000u);
                safe_anchor_valid_=true;
            }
        }
    }
    hazard_pause_remaining_ = 0;
    last_hazard_kind_ = ClassicNamedHazard::None;
    pending_world_oob_event_.reset();
    special_green_pause_remaining_ = 0;
    special_green_continuation_ready_ = false;
    recovered_counters_ = {};
    next_hole_index_.reset();
    round_complete_ = false;
    green_mode_ = false;
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
        return (surface.product_supported
                && surface.landing_code >= 1
                && surface.landing_code <= 7)
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
        ball_x_raw_, ball_y_raw_, green_mode_);

    active_club_ = request.club_index;
    if (!surface_supported_for_active_shot(surface)) {
        unsupported_descriptor_ = surface.descriptor_index;
        phase_ = HoleSessionPhase::UnsupportedTerrain;
        throw std::runtime_error(
            "current terrain requires a classic rule not yet product-integrated");
    }

    last_hazard_kind_ = ClassicNamedHazard::None;
    shot_.reset();
    shot_.set_ball_origin(ball_x_raw_, ball_y_raw_);
    shot_.set_surface_context(to_context(surface));

    ShotInput input{};
    input.aim_raw = request.aim_raw;
    input.power_tick = request.power_tick;
    input.accuracy_tick = request.accuracy_tick;
    input.club_index = request.club_index;
    input.lie_index = surface.profile_slot;

    // Source Windows v1.014 0x40AD1D original-machine 990-case audit:
    // BOTH 16-bit player counters increment on every entered shot attempt,
    // including an accuracy outside the selected swing-profile bounds.
    // Those outside-profile attempts cause zero flight (668/668 probes).
    // Never treat a completed, invalid-accuracy swing as a free retry.
    if (input.aim_raw < 0 || input.aim_raw > 4095 ||
        input.power_tick < 0 || input.power_tick > 105 ||
        input.club_index > 12)
        throw std::out_of_range("original input outside raw gameplay register range");
    if (!recovered::accuracy_within_original_profile_bounds(
            input.club_index, input.lie_index, input.accuracy_tick)) {
        shot_.relocate_inactive_ball(
            ball_x_raw_, ball_y_raw_, surface.landing_code);
        recovered_counters_.player_52 = static_cast<std::uint16_t>(
            recovered_counters_.player_52 + 1u);
        recovered_counters_.player_56 = static_cast<std::uint16_t>(
            recovered_counters_.player_56 + 1u);
        ++strokes_;
        phase_ = HoleSessionPhase::ReadyForShot;
        unsupported_descriptor_.reset();
        return;
    }
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

void ClassicHoleSession::complete_scored_hole(bool correct_putter_code8_terminal) {
    if (!hole_metadata_) {
        return;
    }

    // The original zero-distance pre-update branch scores the hole regardless
    // of club. Correct only the extra transient club-12/code-8 increment,
    // not a normal resting putt that reaches zero recovered distance.
    if (correct_putter_code8_terminal) {
        recovered_counters_.player_52 = static_cast<std::uint16_t>(
            recovered_counters_.player_52 - 1u);
        recovered_counters_.player_56 = static_cast<std::uint16_t>(
            recovered_counters_.player_56 - 1u);
    }

    recovered_counters_.player_58 = static_cast<std::uint16_t>(
        recovered_counters_.player_58 + hole_metadata_->par);

    const auto relative_raw = static_cast<std::uint16_t>(
        recovered_counters_.player_58 - recovered_counters_.player_56);
    recovered_counters_.player_48 =
        (relative_raw & 0x8000u)
        ? static_cast<std::int16_t>(
            static_cast<std::int32_t>(relative_raw) - 0x10000)
        : static_cast<std::int16_t>(relative_raw);

    recovered_counters_.player_70 = static_cast<std::uint16_t>(
        recovered_counters_.player_70 + 1u);

    const auto next = static_cast<std::uint16_t>(
        hole_metadata_->hole_index + 1u);
    next_hole_index_ = next;
    round_complete_ = next == 18u;
    phase_ = HoleSessionPhase::HoleScored;
}

void ClassicHoleSession::acknowledge_special_green_stop() {
    if (phase_ != HoleSessionPhase::SpecialGreenStopped) {
        throw std::logic_error(
            "special-green stop can only be acknowledged from terminal state");
    }
    if (!special_green_continuation_ready_) {
        throw std::logic_error(
            "special-green terminal pause has not completed");
    }

    special_green_continuation_ready_ = false;
    phase_ = HoleSessionPhase::ReadyForShot;
}

void ClassicHoleSession::acknowledge_hazard_recovery() {
    if (phase_ != HoleSessionPhase::HazardRecovered) {
        throw std::logic_error(
            "hazard recovery can only be acknowledged after relocation");
    }

    // v1.014 has not changed the single-player stroke counters through the
    // code-35 stop/recovery fragment. Terminal presentation completion clears
    // the UI/game-flow gate separately; the recovered ball position is already
    // authoritative at this point.
    pending_world_oob_event_.reset();
    phase_ = HoleSessionPhase::ReadyForShot;
}

bool ClassicHoleSession::apply_original_post_stop_recovery() {
    recovered::HazardRecoveryInput input{};
    input.ball_x = ball_x_raw_;
    input.ball_y = ball_y_raw_;
    input.safe_anchor_x = safe_anchor_x_raw_;
    input.safe_anchor_y = safe_anchor_y_raw_;
    input.x_extent = course_.recovery_x_extent();
    input.y_extent = course_.recovery_y_extent();
    input.green_mode = green_mode_;

    const auto recovered_state =
        last_hazard_kind_ == ClassicNamedHazard::OutsideWorld
            // Original 0x40AACF unconditionally enters 0x40A871, even
            // when ball high-word == map dimension (lookup itself uses
            // JGE whereas the separate 0x40A823 check uses JA).
            ? recovered::original_world_oob_restore({input,0,0,0}).relocated
            : recovered::recover_hazard_position(input);

    if (recovered_state.used_safe_anchor && !safe_anchor_valid_) {
        phase_ = HoleSessionPhase::UnsupportedTerrain;
        return false;
    }

    ball_x_raw_ = recovered_state.ball_x;
    ball_y_raw_ = recovered_state.ball_y;

    const auto recovered_surface = course_.resolve_raw_position(
        ball_x_raw_, ball_y_raw_, green_mode_);
    shot_.relocate_inactive_ball(
        ball_x_raw_,
        ball_y_raw_,
        recovered_surface.landing_code);

    return true;
}

void ClassicHoleSession::step() {
    if (phase_ == HoleSessionPhase::SpecialGreenStopped) {
        if (!special_green_continuation_ready_) {
            // Original v1.014 0x40A7E0: 16-bit signed DEC/JNS.
            // 100 -> ... -> 1 -> 0 still waits; the NEXT original tick
            // underflows 0 -> 65535 and clears the UI latch.
            // We retain an explicit host acknowledgement ONLY because
            // the downstream original terrain/player relocation state
            // after 0x40A7F1 has not yet passed end-to-end parity.
            const auto out=recovered::original_special_pause_tick(
                {special_green_pause_remaining_,1,false});
            if (out.expired) {
                special_green_pause_remaining_ = 0; // host-facing value
                // Actual original v1.014 0x40A7F1..0x40A8D6
                // shares the already recovered post-stop course/player
                // position restoration with code-35 hazard recovery.
                // Host acknowledgement remains only for presentation
                // while the outer player-turn scheduler is unverified.
                if (!apply_original_post_stop_recovery()) return;
                special_green_continuation_ready_ = true;
            } else {
                special_green_pause_remaining_ = out.remaining;
            }
        }
        return;
    }

    if (phase_ == HoleSessionPhase::CupTerminal) {
        // v1.014 scores the hole in the later zero-distance pre-update branch,
        // not in the landing-code-8 terminal itself. The full single-player
        // counter adjustment is currently proven for the putter path.
        if (hole_metadata_ && distance_to_hole() == 0) {
            complete_scored_hole(active_club_ == 12);
        }
        return;
    }

    // The original v1.014 0x40A456 zero-distance pre-update branch is
    // independent of the landing-code-8 terminal. A normal resting shot can
    // also have distance zero (the original helper rounds to integer units).
    // Model that check on the next logical tick, without manufacturing a
    // code-8 event or altering recovered ball physics.
    if (phase_ == HoleSessionPhase::ReadyForShot) {
        if (strokes_ > 0 && hole_metadata_ && distance_to_hole() == 0) {
            complete_scored_hole(false);
        }
        return;
    }

    if (phase_ == HoleSessionPhase::HazardStopped) {
        if (hazard_pause_remaining_ > 0) {
            --hazard_pause_remaining_;
            return;
        }

        if (!apply_original_post_stop_recovery()) return;

        phase_ = HoleSessionPhase::HazardRecovered;
        return;
    }

    if (phase_ != HoleSessionPhase::ShotActive) {
        return;
    }

    update_green_mode_for_ball();
    const auto& state = shot_.state();
    if (state.x_raw < std::numeric_limits<std::int32_t>::min()
        || state.x_raw > std::numeric_limits<std::int32_t>::max()
        || state.y_raw < std::numeric_limits<std::int32_t>::min()
        || state.y_raw > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error("classic ball coordinate outside recovered 32-bit range");
    }

    // Original x86 0x409535 rejects signed world coordinates outside
    // the map before the MAPI descriptor lookup. Do not throw from
    // resolve_raw_position for a legitimate shot crossing the boundary.
    // This branch is DISTINCT from in-map descriptor 22 code35.
    if(!green_mode_) {
        recovered::OriginalWorldBoundaryInput boundary{};
        boundary.ball_x=static_cast<std::int32_t>(state.x_raw);
        boundary.ball_y=static_cast<std::int32_t>(state.y_raw);
        boundary.x_extent=course_.recovery_x_extent();
        boundary.y_extent=course_.recovery_y_extent();
        boundary.strokes_on_hole=recovered_counters_.player_52;
        boundary.strokes_total=recovered_counters_.player_56;
        const auto oob=recovered::original_world_boundary_check(boundary);
        if(oob.outside_world) {
            // Verified original 0x40A327..0x40A34C: BOTH original
            // counters increment for the out-of-world penalty, separately
            // from the stroke already counted at launch.
            recovered_counters_.player_52=oob.strokes_on_hole;
            recovered_counters_.player_56=oob.strokes_total;
            ++strokes_;
            last_hazard_kind_=ClassicNamedHazard::OutsideWorld;
            pending_world_oob_event_=recovered::original_world_oob_event();
            ball_x_raw_=static_cast<std::int32_t>(state.x_raw);
            ball_y_raw_=static_cast<std::int32_t>(state.y_raw);
            shot_.stop_for_world_boundary();
            // Original exits to UI dispatch 0x40CD00 then bit2 state
            // recovery 0x40AACF -> 0x40A871. A 100-value pause is
            // source-authentic, but exact UI/ack timing is not yet proven.
            phase_=HoleSessionPhase::HazardStopped;
            hazard_pause_remaining_=oob.ball_pause;
            return;
        }
    }

    const auto surface = course_.resolve_raw_position(
        static_cast<std::int32_t>(state.x_raw),
        static_cast<std::int32_t>(state.y_raw),
        green_mode_);

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
    const auto flag=course_.hole_position();
    shot_.set_original_interaction_context(
        prng_, green_mode_, static_cast<std::int16_t>(flag.x),
        static_cast<std::int16_t>(flag.y));
    shot_.step();

    // The original transition dispatcher can switch coordinate spaces as the
    // ball crosses the green window. Re-evaluate after movement so a shot that
    // comes to rest inside the window is already in MAPS space for the next
    // input.
    update_green_mode_for_ball();

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
        last_hazard_kind_ = classic_named_hazard(surface.descriptor_index);
        phase_ = HoleSessionPhase::HazardStopped;
        hazard_pause_remaining_ = 100;
        break;
    case ClassicShotOutcome::SpecialGreenStop:
        phase_ = HoleSessionPhase::SpecialGreenStopped;
        special_green_pause_remaining_ = 100;
        special_green_continuation_ready_ = false;
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
            static_cast<std::int32_t>(state.y_raw),
            green_mode_);
    }
    return course_.resolve_raw_position(
        ball_x_raw_, ball_y_raw_, green_mode_);
}

ClassicCoursePoint ClassicHoleSession::hole_position() const {
    return course_.hole_position();
}

std::optional<std::uint16_t>
ClassicHoleSession::unsupported_descriptor() const noexcept {
    return unsupported_descriptor_;
}

ClassicNamedHazard ClassicHoleSession::last_hazard_kind() const noexcept {
    return last_hazard_kind_;
}

std::optional<recovered::OriginalWorldOobEvent>
ClassicHoleSession::pending_world_oob_event() const noexcept {
    return pending_world_oob_event_;
}

std::uint16_t ClassicHoleSession::hazard_pause_remaining() const noexcept {
    return hazard_pause_remaining_;
}

std::uint16_t ClassicHoleSession::special_green_pause_remaining() const noexcept {
    return special_green_pause_remaining_;
}

const ClassicRecoveredCounters&
ClassicHoleSession::recovered_counters() const noexcept {
    return recovered_counters_;
}

std::uint32_t ClassicHoleSession::distance_to_hole() const noexcept {
    const auto hole = mode_hole_position();
    return recovered::distance_to_hole(
        ball_x_raw_, ball_y_raw_, hole.x, hole.y, green_mode_);
}

std::optional<ClassicHoleMetadata>
ClassicHoleSession::hole_metadata() const noexcept {
    return hole_metadata_;
}

std::optional<std::uint16_t>
ClassicHoleSession::next_hole_index() const noexcept {
    return next_hole_index_;
}

bool ClassicHoleSession::round_complete() const noexcept {
    return round_complete_;
}

bool ClassicHoleSession::green_mode() const noexcept {
    return green_mode_;
}

void ClassicHoleSession::update_green_mode_for_ball() {
    const auto region = course_.green_region();
    if (!region) {
        return;
    }

    const auto& state = shot_.state();
    const auto ix = static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.x_raw) >> 16);
    const auto iy = static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.y_raw) >> 16);

    if (!green_mode_) {
        if (!region->contains_course(ix, iy)) {
            return;
        }
        shot_.enter_green_coordinates(
            static_cast<std::int32_t>(
                static_cast<std::uint32_t>(region->origin_x) << 16),
            static_cast<std::int32_t>(
                static_cast<std::uint32_t>(region->origin_y) << 16));
        green_mode_ = true;
        return;
    }

    if (region->contains_green(ix, iy)) {
        return;
    }

    shot_.leave_green_coordinates(
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(region->origin_x) << 16),
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(region->origin_y) << 16));
    green_mode_ = false;
}

ClassicCoursePoint ClassicHoleSession::mode_hole_position() const {
    const auto hole = course_.hole_position();
    if (!green_mode_) {
        return hole;
    }

    const auto region = course_.green_region();
    if (!region) {
        return hole;
    }

    return ClassicCoursePoint{
        static_cast<std::uint16_t>(hole.x - region->origin_x),
        static_cast<std::uint16_t>(hole.y - region->origin_y)};
}

bool ClassicHoleSession::has_prng_state() const noexcept {
    return prng_ != nullptr;
}

recovered::OriginalPrng16& ClassicHoleSession::prng_state() {
    if (!prng_) {
        throw std::logic_error(
            "classic hole session has no authoritative PRNG state");
    }
    return *prng_;
}

const recovered::OriginalPrng16& ClassicHoleSession::prng_state() const {
    if (!prng_) {
        throw std::logic_error(
            "classic hole session has no authoritative PRNG state");
    }
    return *prng_;
}

} // namespace sensigolf
