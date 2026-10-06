#include "sensigolf/recovered_flight.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>

#include "sensigolf/generated/trig_q14.hpp"

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

// 1-based profile IDs, exactly as stored at 0x41D4C4.
// Rows are club index 0..12; columns are lie/surface slot 0..9.
constexpr std::array<std::array<std::uint8_t, 10>, 13> kProfileSelector{{
    {{1,2,3,8,7,8,4,1,1,1}},
    {{1,2,3,8,7,8,4,1,1,1}},
    {{1,2,3,8,7,8,4,1,1,1}},
    {{1,2,4,8,7,8,4,2,2,2}},
    {{1,2,5,8,7,8,4,2,2,2}},
    {{1,2,5,8,7,8,4,2,2,2}},
    {{2,3,5,8,7,8,4,2,2,2}},
    {{2,3,5,8,7,8,4,2,2,2}},
    {{3,3,6,8,7,8,4,2,2,2}},
    {{4,4,6,8,7,8,4,4,4,4}},
    {{5,6,8,8,8,8,4,5,5,5}},
    {{6,7,8,8,8,8,4,5,5,5}},
    {{1,1,1,11,11,11,1,1,1,1}},
}};

struct Bounds { std::int16_t lower; std::int16_t upper; };
constexpr std::array<Bounds, 11> kProfileBounds{{
    {61,65}, {61,65}, {60,66}, {60,66}, {58,68}, {58,68},
    {55,70}, {55,70}, {55,70}, {55,70}, {55,70},
}};

// Columns are evenized accuracy errors -8,-6,-4,-2,0,+2,+4,+6.
// Entries outside each profile's legal bounds are unused.
constexpr std::array<std::array<std::int16_t, 8>, 11> kProfileSamples{{
    {{0,0,0,-2,0, 2,0,0}},
    {{0,0,0,-2,0, 2,0,0}},
    {{0,0,-4,-2,0, 2,0,0}},
    {{0,0,-1,-1,0, 1,0,0}},
    {{0,-2,-1,-1,0, 1,1,0}},
    {{0,-2,-1,-1,0, 1,1,0}},
    {{-2,-2,-1,-1,0,1,1,1}},
    {{-2,-2,-1, 0,0,0,1,1}},
    {{-2,-2,-1, 0,0,0,1,1}},
    {{-2,-2,-1, 0,0,0,1,1}},
    {{-2,-1,-1, 0,0,0,1,1}},
}};

std::int32_t checked_i32(std::int64_t value, const char* what) {
    if (value < std::numeric_limits<std::int32_t>::min()
        || value > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error(what);
    }
    return static_cast<std::int32_t>(value);
}

std::int32_t arithmetic_shift_right(std::int32_t value, unsigned bits) {
    if (value >= 0) return value >> bits;
    const auto magnitude = static_cast<std::uint32_t>(-(value + 1)) + 1u;
    const auto rounded = (magnitude + ((1u << bits) - 1u)) >> bits;
    return -static_cast<std::int32_t>(rounded);
}

std::int32_t arithmetic_shift_right_14(std::int64_t value) {
    constexpr std::int64_t divisor = 1LL << 14;
    if (value >= 0) return checked_i32(value / divisor, "Q14 overflow");
    return checked_i32(-(((-value) + divisor - 1) / divisor), "Q14 overflow");
}

std::int32_t planar_delta(std::int32_t force, std::int16_t trig) {
    return arithmetic_shift_right_14(
        static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig));
}

std::int32_t profile_sample(std::uint8_t profile_id, std::int32_t error) {
    if (profile_id < 1 || profile_id > 11) {
        throw std::logic_error("invalid recovered profile id");
    }
    const auto even_error = arithmetic_shift_right(error, 1) * 2;
    if (even_error < -8 || even_error > 6 || (even_error & 1) != 0) {
        throw std::out_of_range("accuracy error outside recovered profile sample range");
    }
    const auto column = static_cast<std::size_t>((even_error + 8) / 2);
    return kProfileSamples[profile_id - 1][column];
}

} // namespace

const std::array<ClubLaunchParameters, 13>& club_launch_parameters() {
    return kClubTable;
}

std::int16_t trig_q14(std::uint16_t index) {
    return generated::kSinQ14[index & kDirectionMask];
}

FlightState launch_normal_shot(const LaunchInput& input) {
    if (input.club_index >= kClubTable.size()) {
        throw std::out_of_range("club index outside original 0..12 range");
    }
    if (input.lie_index >= 10) {
        throw std::out_of_range("lie index outside recovered 0..9 selector range");
    }
    const auto& club = kClubTable[input.club_index];
    if (club.special_putter_path) {
        throw std::invalid_argument("club index 12 uses the special putter path");
    }
    if (input.captured_power < 0 || input.captured_power > kMaxCapturedPower) {
        throw std::out_of_range("captured power outside recovered 0..105 range");
    }

    const auto profile_id = kProfileSelector[input.club_index][input.lie_index];
    const auto bounds = kProfileBounds[profile_id - 1];
    if (input.accuracy_tick < bounds.lower || input.accuracy_tick > bounds.upper) {
        throw std::out_of_range(
            "accuracy tick outside currently parity-scoped profile bounds");
    }

    const auto error = input.accuracy_tick - kAccuracyCenter;
    const auto abs_error = std::abs(error);
    const auto adjusted_power = std::max(0, input.captured_power - abs_error);
    const auto direction = static_cast<std::int64_t>(input.player_direction)
        - static_cast<std::int64_t>(error) * 16;

    FlightState state{};
    state.x = input.start_x;
    state.y = input.start_y;
    state.height = 0;
    state.direction = static_cast<std::uint16_t>(direction) & kDirectionMask;
    state.swing_adjuster = profile_sample(profile_id, error);
    state.adjusted_power = adjusted_power;

    const std::int64_t scaled =
        static_cast<std::int64_t>(club.power_scale) * adjusted_power;
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

    if (state.horizontal_force > 0) {
        const auto after_drag =
            static_cast<std::int64_t>(state.horizontal_force)
            - kHorizontalDragPerTick;
        if (after_drag < 0) {
            // Original v1.014 ends this airborne tick immediately when a
            // positive H crosses below zero. Direction/movement resume on
            // the following tick with H already zero.
            state.horizontal_force = 0;
            return AirborneStepResult::Airborne;
        }
        state.horizontal_force = static_cast<std::int32_t>(after_drag);
    } else {
        state.horizontal_force = 0;
    }

    state.direction = static_cast<std::uint16_t>(
        static_cast<std::int64_t>(state.direction)
        - static_cast<std::int64_t>(state.swing_adjuster) * 2)
        & kDirectionMask;

    const auto dx = planar_delta(state.horizontal_force, trig_q14(state.direction));
    const auto dy = planar_delta(
        state.horizontal_force,
        trig_q14(static_cast<std::uint16_t>(state.direction + 1024)));

    state.x = checked_i32(static_cast<std::int64_t>(state.x) + dx, "x overflow");
    state.y = checked_i32(static_cast<std::int64_t>(state.y) + dy, "y overflow");
    return AirborneStepResult::Airborne;
}

} // namespace sensigolf::recovered
