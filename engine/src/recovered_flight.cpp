#include "sensigolf/recovered_flight.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace sensigolf::recovered {
namespace {

constexpr std::array<ClubLaunchParameters, 13> kClubTable{{
    {   8192, 81920, 3168, false },
    {  16384, 73728, 3120, false },
    {  36864, 65536, 3120, false },
    {  49152, 49152, 3072, false },
    {  65536, 49152, 2944, false },
    {  71680, 49152, 2880, false },
    {  77824, 49152, 2560, false },
    {  81920, 49152, 2496, false },
    { 112640, 48128, 2464, false },
    { 114688, 48128, 2368, false },
    { 131072, 47104, 2176, false },
    { 131072, 40960, 2112, false },
    {      0,  8192, 1536, true  },
}};

std::int32_t checked_i32(std::int64_t value, const char* what) {
    if (value < std::numeric_limits<std::int32_t>::min()
        || value > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error(what);
    }
    return static_cast<std::int32_t>(value);
}

std::int32_t arithmetic_shift_right_14(std::int64_t value) {
    constexpr std::int64_t divisor = 1LL << 14;
    if (value >= 0) {
        return checked_i32(value / divisor, "positive Q14 product overflow");
    }
    // x86 SAR/SHRD semantics: signed floor division by 2^14.
    return checked_i32(-(((-value) + divisor - 1) / divisor),
                       "negative Q14 product overflow");
}

std::int32_t planar_delta(std::int32_t force, std::int16_t trig) {
    const std::int64_t product =
        static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig);
    return arithmetic_shift_right_14(product);
}

} // namespace

const std::array<ClubLaunchParameters, 13>& club_launch_parameters() {
    return kClubTable;
}

std::int16_t trig_q14(std::uint16_t index) {
    constexpr double kPi = 3.141592653589793238462643383279502884;
    const std::uint16_t wrapped = static_cast<std::uint16_t>(index & kDirectionMask);
    const double angle = (2.0 * kPi * static_cast<double>(wrapped)) / 4096.0;
    std::int32_t value = static_cast<std::int32_t>(std::sin(angle) * 16384.0);
    if (value < -16383) {
        value = -16383;
    }
    return static_cast<std::int16_t>(value);
}

FlightState launch_normal_shot(const LaunchInput& input) {
    if (input.club_index >= kClubTable.size()) {
        throw std::out_of_range("club index outside original 0..12 range");
    }
    const auto& club = kClubTable[input.club_index];
    if (club.special_putter_path) {
        throw std::invalid_argument("club index 12 uses the unrecovered putter path");
    }
    if (input.captured_power < 0 || input.captured_power > kMaxCapturedPower) {
        throw std::out_of_range("captured power outside recovered 0..105 range");
    }

    const std::int64_t abs_error =
        input.accuracy_error < 0
            ? -static_cast<std::int64_t>(input.accuracy_error)
            : static_cast<std::int64_t>(input.accuracy_error);
    const std::int32_t power_after_accuracy =
        static_cast<std::int32_t>(
            std::max<std::int64_t>(0, input.captured_power - abs_error));

    const std::int64_t direction =
        static_cast<std::int64_t>(input.player_direction)
        - static_cast<std::int64_t>(input.accuracy_error) * 16;

    FlightState state{};
    state.x = input.start_x;
    state.y = input.start_y;
    state.height = 0;
    state.direction =
        static_cast<std::uint16_t>(direction) & kDirectionMask;
    state.swing_adjuster = input.swing_adjuster;

    const std::int64_t scaled =
        static_cast<std::int64_t>(club.power_scale) * power_after_accuracy;
    state.vertical_force =
        checked_i32(static_cast<std::int64_t>(club.vertical_base) + scaled,
                    "vertical launch force overflow");
    state.horizontal_force =
        checked_i32(static_cast<std::int64_t>(club.horizontal_base) + scaled,
                    "horizontal launch force overflow");
    return state;
}

AirborneStepResult step_clear_air(FlightState& state) {
    state.vertical_force =
        checked_i32(static_cast<std::int64_t>(state.vertical_force) - kGravityPerTick,
                    "vertical force overflow");

    const std::int64_t next_height =
        static_cast<std::int64_t>(state.height) + state.vertical_force;
    if (next_height <= 0) {
        state.height = 0;
        return AirborneStepResult::NeedsLandingResolution;
    }
    state.height = checked_i32(next_height, "height overflow");

    state.horizontal_force =
        std::max<std::int32_t>(0, state.horizontal_force - kHorizontalDragPerTick);

    const std::int64_t curved_direction =
        static_cast<std::int64_t>(state.direction)
        - static_cast<std::int64_t>(state.swing_adjuster) * 2;
    state.direction =
        static_cast<std::uint16_t>(curved_direction) & kDirectionMask;

    const auto sx = trig_q14(state.direction);
    const auto sy = trig_q14(static_cast<std::uint16_t>(state.direction + 1024));

    const auto dx = planar_delta(state.horizontal_force, sx);
    const auto dy = planar_delta(state.horizontal_force, sy);

    state.x = checked_i32(static_cast<std::int64_t>(state.x) + dx, "x overflow");
    state.y = checked_i32(static_cast<std::int64_t>(state.y) + dy, "y overflow");

    return AirborneStepResult::Airborne;
}

} // namespace sensigolf::recovered
