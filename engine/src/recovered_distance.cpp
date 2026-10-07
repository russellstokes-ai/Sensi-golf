#include "sensigolf/recovered_distance.hpp"

#include <cstdint>

namespace sensigolf::recovered {
namespace {

std::uint32_t arithmetic_shift_right_one(std::uint32_t value) noexcept {
    return (value >> 1u) | (value & 0x80000000u);
}

std::int64_t signed_wrapped_delta(
    std::uint32_t integer_component,
    std::uint16_t target) noexcept {
    const auto wrapped =
        integer_component - static_cast<std::uint32_t>(target);
    if ((wrapped & 0x80000000u) != 0u) {
        return static_cast<std::int64_t>(wrapped) - 0x100000000LL;
    }
    return static_cast<std::int64_t>(wrapped);
}

std::uint64_t integer_sqrt(std::uint64_t value) noexcept {
    // Restoring integer square root. This avoids floating-point rounding and
    // therefore preserves the zero-tolerance parity contract.
    std::uint64_t result = 0;
    std::uint64_t bit = std::uint64_t{1} << 62u;

    while (bit > value) {
        bit >>= 2u;
    }

    while (bit != 0u) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1u) + bit;
        } else {
            result >>= 1u;
        }
        bit >>= 2u;
    }

    return result;
}

std::uint64_t magnitude(std::int64_t value) noexcept {
    return value < 0
        ? static_cast<std::uint64_t>(-value)
        : static_cast<std::uint64_t>(value);
}

} // namespace

std::uint32_t distance_to_hole(
    std::int32_t x_raw,
    std::int32_t y_raw,
    std::uint16_t hole_x,
    std::uint16_t hole_y,
    bool green_mode) noexcept {
    auto x = static_cast<std::uint32_t>(x_raw);
    auto y = static_cast<std::uint32_t>(y_raw);

    if (green_mode) {
        x = arithmetic_shift_right_one(x);
        y = arithmetic_shift_right_one(y);
    }

    const auto integer_x = x >> 16u;
    const auto integer_y = y >> 16u;

    const auto dx = signed_wrapped_delta(integer_x, hole_x);
    const auto dy = signed_wrapped_delta(integer_y, hole_y);
    const auto abs_dx = magnitude(dx);
    const auto abs_dy = magnitude(dy);

    const auto squared =
        abs_dx * abs_dx + abs_dy * abs_dy;
    const auto root = integer_sqrt(squared);

    return static_cast<std::uint32_t>((root * 6u) / 10u);
}

} // namespace sensigolf::recovered
