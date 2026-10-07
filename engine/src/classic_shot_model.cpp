#include "sensigolf/classic_shot_model.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

#include "sensigolf/recovered_ground.hpp"

namespace sensigolf {
namespace {

std::int64_t sar_floor(std::int64_t value, unsigned bits) {
    if (value >= 0) return value >> bits;
    const auto divisor = std::int64_t{1} << bits;
    return -(((-value) + divisor - 1) / divisor);
}

std::int32_t planar_component(
    std::int32_t force,
    std::int16_t trig) {
    const auto wide =
        static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig);
    const auto shifted = sar_floor(wide, 14);
    if (shifted < std::numeric_limits<std::int32_t>::min()
        || shifted > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error("classic shot planar component overflow");
    }
    return static_cast<std::int32_t>(shifted);
}

} // namespace

void ClassicShotModel::reset() {
    flight_ = {};
    public_ = {};
    surface_ = {};
    origin_x_ = 0;
    origin_y_ = 0;
    active_club_ = 0;
    active_ = false;
}

void ClassicShotModel::set_surface_context(
    const ClassicSurfaceContext& context) {
    if (active_) {
        throw std::logic_error("cannot change surface context during an active shot");
    }
    surface_ = context;
}

const ClassicSurfaceContext& ClassicShotModel::surface_context() const noexcept {
    return surface_;
}

void ClassicShotModel::set_ball_origin(
    std::int32_t x_raw,
    std::int32_t y_raw) noexcept {
    if (active_) {
        return;
    }
    origin_x_ = x_raw;
    origin_y_ = y_raw;
}

void ClassicShotModel::begin_shot(const ShotInput& input) {
    if (active_) {
        throw std::logic_error("shot already active");
    }
    if (input.aim_raw < 0 || input.aim_raw > recovered::kDirectionMask) {
        throw std::out_of_range("aim_raw must be original direction 0..4095");
    }

    recovered::LaunchInput launch{};
    launch.club_index = input.club_index;
    launch.lie_index = input.lie_index;
    launch.captured_power = input.power_tick;
    launch.accuracy_tick = input.accuracy_tick;
    launch.player_direction = static_cast<std::uint16_t>(input.aim_raw);
    launch.start_x = origin_x_;
    launch.start_y = origin_y_;

    flight_ = recovered::launch_normal_shot(launch);
    active_club_ = input.club_index;
    active_ = true;

    public_ = {};
    public_.phase = active_club_ == 12
        ? ShotPhase::GroundRoll
        : ShotPhase::Airborne;
    sync_public_state();
}

void ClassicShotModel::step() {
    if (!active_) {
        return;
    }

    ++public_.tick;

    recovered::GroundStepResult result{};
    if (active_club_ == 12) {
        if (surface_.landing_code != 1) {
            throw std::logic_error(
                "recovered club-12 model requires green landing code 1");
        }
        result = recovered::step_green_putt(
            flight_,
            surface_.slope_direction,
            surface_.slope_magnitude);
    } else {
        result = recovered::step_controlled_surface(
            flight_,
            surface_.landing_code);
    }

    if (result.contacted_ground && !result.resting) {
        public_.phase = ShotPhase::GroundRoll;
    }
    if (result.hazard_stop) {
        public_.hazard = true;
    }
    if (result.holed) {
        public_.holed = true;
    }
    if (result.resting) {
        active_ = false;
        public_.phase = ShotPhase::Complete;
    }

    sync_public_state();
}

void ClassicShotModel::sync_public_state() {
    public_.x_raw = flight_.x;
    public_.y_raw = flight_.y;
    public_.z_raw = flight_.height;

    public_.vx_raw = planar_component(
        flight_.horizontal_force,
        recovered::trig_q14(flight_.direction));
    public_.vy_raw = planar_component(
        flight_.horizontal_force,
        recovered::trig_q14(
            static_cast<std::uint16_t>(flight_.direction + 1024)));
    public_.vz_raw = flight_.vertical_force;
    public_.surface_index = surface_.landing_code;
}

const BallState& ClassicShotModel::state() const {
    return public_;
}

bool ClassicShotModel::shot_active() const {
    return active_;
}

std::uint32_t ClassicShotModel::tick_rate_hz() const {
    return kNominalTickRateHz;
}

} // namespace sensigolf
