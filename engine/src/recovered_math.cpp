#include "sensigolf/recovered_math.hpp"

#include <cassert>
#include <cstdint>

#include "sensigolf/generated/trig_q14.hpp"

namespace sensigolf::recovered {
namespace {

std::int64_t arithmetic_shift_right(std::int64_t value, unsigned bits) noexcept {
    if (value >= 0) {
        return value >> bits;
    }

    const auto magnitude = static_cast<std::uint64_t>(-(value + 1)) + 1u;
    const auto rounded = (magnitude + ((std::uint64_t{1} << bits) - 1u)) >> bits;
    return -static_cast<std::int64_t>(rounded);
}

} // namespace

std::int16_t sin_q14(std::uint32_t direction) noexcept {
    return generated::kSinQ14[direction & kDirectionMask];
}

std::int16_t cos_q14(std::uint32_t direction) noexcept {
    return generated::kSinQ14[(direction + 1024u) & kDirectionMask];
}

std::uint32_t wrap_direction(std::int64_t direction) noexcept {
    return static_cast<std::uint32_t>(direction) & kDirectionMask;
}

std::int32_t project_q14(std::int32_t force, std::int16_t trig_value) noexcept {
    const auto product =
        static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig_value);
    return static_cast<std::int32_t>(arithmetic_shift_right(product, 14));
}

std::pair<std::int32_t, std::int32_t>
project_horizontal(std::int32_t force, std::uint32_t direction) noexcept {
    return {
        project_q14(force, sin_q14(direction)),
        project_q14(force, cos_q14(direction)),
    };
}

LaunchState make_launch_state(
    std::size_t club_index,
    std::uint16_t drop_power,
    std::uint32_t direction,
    std::int32_t swing_adjuster) noexcept {
    assert(club_index < kClubPhysics.size());
    const auto& club = kClubPhysics[club_index];

    const auto power_component =
        static_cast<std::int64_t>(club.power_scale) * drop_power;

    return LaunchState{
        static_cast<std::int32_t>(
            static_cast<std::int64_t>(club.loaded_vertical_base()) + power_component),
        static_cast<std::int32_t>(
            static_cast<std::int64_t>(club.loaded_horizontal_base()) + power_component),
        wrap_direction(
            static_cast<std::int64_t>(direction) -
            static_cast<std::int64_t>(swing_adjuster) * 2),
    };
}

void apply_airborne_vertical_tick(RecoveredBallState& ball) noexcept {
    ball.vertical_force -= kGravityPerTick;
    ball.height += ball.vertical_force;
    if (ball.height < 0) {
        ball.height = 0;
    }
}

void apply_roll_drag(RecoveredBallState& ball, bool green_mode) noexcept {
    const auto drag = green_mode ? kGreenRollDrag : kNormalRollDrag;
    ball.horizontal_force -= drag;
    if (ball.horizontal_force < 0) {
        ball.horizontal_force = 0;
    }
}

void apply_bounce(RecoveredBallState& ball) noexcept {
    // x86 sequence:
    //   eax = V; neg eax; sar eax,1; V=eax; sar eax,1; H+=eax
    auto rebound = -static_cast<std::int64_t>(ball.vertical_force);
    rebound = arithmetic_shift_right(rebound, 1);
    ball.vertical_force = static_cast<std::int32_t>(rebound);

    const auto transfer = arithmetic_shift_right(rebound, 1);
    const auto horizontal =
        static_cast<std::int64_t>(ball.horizontal_force) + transfer;

    if (horizontal <= 0) {
        ball.horizontal_force = 0;
        ball.vertical_force = 0;
    } else {
        ball.horizontal_force = static_cast<std::int32_t>(horizontal);
    }
}

void apply_horizontal_tick(RecoveredBallState& ball) noexcept {
    const auto [dx, dy] = project_horizontal(ball.horizontal_force, ball.direction);
    ball.x += dx;
    ball.y += dy;
}

} // namespace sensigolf::recovered
