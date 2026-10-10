#include "sensigolf/recovered_prng.hpp"

#include <cstdint>

namespace sensigolf::recovered {
namespace {

std::uint16_t rol16(std::uint16_t value, unsigned bits) noexcept {
    bits &= 15u;
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(value << bits)
        | static_cast<std::uint16_t>(value >> ((16u - bits) & 15u)));
}

std::uint16_t ror16(std::uint16_t value, unsigned bits) noexcept {
    bits &= 15u;
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(value >> bits)
        | static_cast<std::uint16_t>(value << ((16u - bits) & 15u)));
}

} // namespace

std::uint16_t OriginalPrng16::next(std::uint16_t max_inclusive) noexcept {
    auto value = rol16(seed0, 3);
    if ((value & 0x8000u) != 0) {
        value = static_cast<std::uint16_t>(value ^ seed1);
        seed1 = ror16(seed1, 1);
    }
    seed0 = value;

    const auto range =
        static_cast<std::uint16_t>(max_inclusive + std::uint16_t{1});
    if (range == 0) {
        return value;
    }

    const auto product =
        static_cast<std::uint32_t>(value) * static_cast<std::uint32_t>(range);
    return static_cast<std::uint16_t>(product >> 16);
}

} // namespace sensigolf::recovered
