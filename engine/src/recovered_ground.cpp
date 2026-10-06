#include "sensigolf/recovered_ground.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace sensigolf::recovered {
namespace {

constexpr std::uint16_t kImmediateStopHazardCode = 0x23;

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

void move_after_drag(
    FlightState& state,
    std::int32_t slope_x_raw = 0,
    std::int32_t slope_y_raw = 0) {
    state.direction = static_cast<std::uint16_t>(
        static_cast<std::int64_t>(state.direction)
        - static_cast<std::int64_t>(state.swing_adjuster) * 2)
        & kDirectionMask;

    const auto dx = q14_delta(state.horizontal_force, trig_q14(state.direction));
    const auto dy = q14_delta(
        state.horizontal_force,
        trig_q14(static_cast<std::uint16_t>(state.direction + 1024)));
    const auto slope_dx = static_cast<std::int32_t>(sar_floor(slope_x_raw, 14));
    const auto slope_dy = static_cast<std::int32_t>(sar_floor(slope_y_raw, 14));

    state.x = checked_i32(
        static_cast<std::int64_t>(state.x) + dx + slope_dx,
        "ground x overflow");
    state.y = checked_i32(
        static_cast<std::int64_t>(state.y) + dy + slope_dy,
        "ground y overflow");
}

enum class DragResult {
    Moved,
    EndTick,
    ReenterGround,
};

DragResult drag_then_move(FlightState& state) {
    if (state.horizontal_force > 0) {
        const auto after =
            static_cast<std::int64_t>(state.horizontal_force)
            - kHorizontalDragPerTick;

        if (after < 0) {
            state.horizontal_force = 0;

            // Original v1.014 branch at 0x40A634:
            // when positive H crosses below zero, the game does NOT enter
            // 0x40A96F on this tick. Airborne shots end the tick immediately;
            // ground shots fall straight back into landing resolution.
            if (state.height == 0) {
                return DragResult::ReenterGround;
            }
            return DragResult::EndTick;
        }

        state.horizontal_force = static_cast<std::int32_t>(after);
    } else {
        // If H was already zero/non-positive on entry, original code does
        // continue through 0x40A96F, so direction still advances.
        state.horizontal_force = 0;
    }

    move_after_drag(state);
    return DragResult::Moved;
}

} // namespace

GreenSlopeAdjustment green_slope_adjustment(
    std::uint16_t slope_direction,
    std::uint16_t slope_magnitude) {
    // Original 0x40B965:
    //   eax = magnitude << 12
    //   imul q14_sin/cos -> low 32-bit raw adjustment
    const auto base =
        static_cast<std::int32_t>(static_cast<std::uint32_t>(slope_magnitude) << 12);
    const auto raw_x_wide =
        static_cast<std::int64_t>(base) * trig_q14(slope_direction);
    const auto raw_y_wide =
        static_cast<std::int64_t>(base) *
        trig_q14(static_cast<std::uint16_t>(slope_direction + 1024));

    return GreenSlopeAdjustment{
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(raw_x_wide)),
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(raw_y_wide)),
    };
}

GroundStepResult step_green_putt(
    FlightState& state,
    std::uint16_t slope_direction,
    std::uint16_t slope_magnitude) {
    GroundStepResult result{};

    // Club 12 enters the original update at 0x40A581. On a normal green
    // descriptor it bypasses gravity entirely and stays at height zero.
    state.height = 0;

    if (state.horizontal_force <= 0) {
        state.horizontal_force = 0;
        state.vertical_force = 0;
        result.resting = true;
        return result;
    }

    const auto after =
        static_cast<std::int64_t>(state.horizontal_force)
        - kGreenHorizontalDragPerTick;

    if (after < 0) {
        // Original stop path clears the launch V field as the putt comes
        // to rest; no movement/slope term is applied on the crossing tick.
        state.horizontal_force = 0;
        state.vertical_force = 0;
        result.resting = true;
        return result;
    }

    state.horizontal_force =
        checked_i32(after, "green putter H overflow");
    const auto slope = green_slope_adjustment(
        slope_direction, slope_magnitude);
    move_after_drag(state, slope.raw_x, slope.raw_y);
    return result;
}

GroundStepResult step_controlled_surface(
    FlightState& state,
    std::uint16_t landing_code) {
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
        (void)drag_then_move(state);
        return result;
    }

    result.contacted_ground = true;
    result.contact_x = state.x;
    result.contact_y = state.y;

    // Windows v1.014 0x40A730 code-8 terminal branch. It transfers to
    // hole-completion flow without zeroing the current ball forces here.
    if (landing_code == 8) {
        result.holed = true;
        result.resting = true;
        return result;
    }

    // Windows v1.014 0x40A692..0x40A72B. Terrain descriptors WATER,
    // NO GO and OUT OF BOUNDS use landing code 0x23. On contact the original
    // zeros H, V and height before leaving the normal bounce branch.
    if (landing_code == kImmediateStopHazardCode) {
        state.horizontal_force = 0;
        state.vertical_force = 0;
        state.height = 0;
        result.hazard_stop = true;
        result.resting = true;
        return result;
    }

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

        const auto drag = drag_then_move(state);
        if (drag == DragResult::ReenterGround) {
            continue;
        }
        return result;
    }
}

} // namespace sensigolf::recovered
