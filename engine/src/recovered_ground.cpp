#include "sensigolf/recovered_ground.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace sensigolf::recovered {
namespace {

std::int32_t checked_i32(std::int64_t value, const char* what) {
    if (value < std::numeric_limits<std::int32_t>::min()
        || value > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error(what);
    }
    return static_cast<std::int32_t>(value);
}

std::int64_t sar_floor(std::int64_t value, unsigned bits) {
    if (value >= 0) return value >> bits;
    const auto divisor = std::int64_t{1} << bits;
    return -(((-value) + divisor - 1) / divisor);
}

std::int32_t q14_delta(std::int32_t force, std::int16_t trig) {
    return checked_i32(
        sar_floor(
            static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig),
            14),
        "ground Q14 delta overflow");
}

void move_after_drag(FlightState& state) {
    state.direction = static_cast<std::uint16_t>(
        static_cast<std::int64_t>(state.direction)
        - static_cast<std::int64_t>(state.swing_adjuster) * 2)
        & kDirectionMask;

    const auto dx = q14_delta(state.horizontal_force, trig_q14(state.direction));
    const auto dy = q14_delta(
        state.horizontal_force,
        trig_q14(static_cast<std::uint16_t>(state.direction + 1024)));

    state.x = checked_i32(
        static_cast<std::int64_t>(state.x) + dx, "ground x overflow");
    state.y = checked_i32(
        static_cast<std::int64_t>(state.y) + dy, "ground y overflow");
}

bool drag_then_move_or_reenter_ground(FlightState& state) {
    if (state.horizontal_force > 0) {
        const auto after =
            static_cast<std::int64_t>(state.horizontal_force)
            - kHorizontalDragPerTick;
        if (after < 0) {
            state.horizontal_force = 0;
            if (state.height == 0) return true;
        } else {
            state.horizontal_force = static_cast<std::int32_t>(after);
        }
    } else {
        state.horizontal_force = 0;
    }

    move_after_drag(state);
    return false;
}

} // namespace

GroundStepResult step_generic_flat_surface(FlightState& state) {
    GroundStepResult result{};

    state.vertical_force = checked_i32(
        static_cast<std::int64_t>(state.vertical_force) - kGravityPerTick,
        "ground vertical force overflow");

    const auto next_height =
        static_cast<std::int64_t>(state.height) + state.vertical_force;
    state.height = next_height < 0
        ? 0
        : checked_i32(next_height, "ground height overflow");

    if (state.height != 0) {
        (void)drag_then_move_or_reenter_ground(state);
        return result;
    }

    result.contacted_ground = true;
    result.contact_x = state.x;
    result.contact_y = state.y;

    for (;;) {
        if (state.vertical_force == 0) {
            result.resting = state.horizontal_force == 0;
            return result;
        }

        const auto rebound = sar_floor(
            -static_cast<std::int64_t>(state.vertical_force), 1);
        state.vertical_force = checked_i32(rebound, "bounce V overflow");

        const auto transfer = sar_floor(rebound, 1);
        const auto horizontal =
            static_cast<std::int64_t>(state.horizontal_force) + transfer;

        if (horizontal <= 0) {
            state.horizontal_force = 0;
            state.vertical_force = 0;
            result.resting = true;
            return result;
        }
        state.horizontal_force =
            checked_i32(horizontal, "bounce H overflow");

        if (drag_then_move_or_reenter_ground(state)) {
            continue;
        }
        return result;
    }
}

} // namespace sensigolf::recovered
