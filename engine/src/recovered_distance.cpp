#include "sensigolf/recovered_distance.hpp"

#include <cstdint>

namespace sensigolf::recovered {
namespace {

std::uint32_t arithmetic_shift_right_one(std::uint32_t bits) noexcept {
    return (bits >> 1) | (bits & 0x80000000u);
}

std::uint64_t integer_sqrt(std::uint64_t value) noexcept {
    std::uint64_t result = 0;
    std::uint64_t bit = std::uint64_t{1} << 62;

    while (bit > value) {
        bit >>= 2;
    }

    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
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

    const auto integer_x = static_cast<std::int64_t>(x >> 16);
    const auto integer_y = static_cast<std::int64_t>(y >> 16);
    const auto dx = integer_x - static_cast<std::int64_t>(hole_x);
    const auto dy = integer_y - static_cast<std::int64_t>(hole_y);

    const auto squared =
        static_cast<std::uint64_t>(dx * dx + dy * dy);
    const auto root = integer_sqrt(squared);
    return static_cast<std::uint32_t>((root * 6u) / 10u);
}

} // namespace sensigolf::recovered
