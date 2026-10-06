#include "sensigolf/recovered_interactions.hpp"

#include <cstdint>

namespace sensigolf::recovered {
namespace {

void add_half_turn_and_biased_random(
    FlightState& state,
    OriginalPrng16& prng) noexcept {
    const auto random = prng.next(0x0200u);
    const auto delta = static_cast<std::uint16_t>(random - 0x0200u);
    auto low = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.direction) + 0x0800u);
    low = static_cast<std::uint16_t>(low + delta);
    state.direction = static_cast<std::uint16_t>(low & kDirectionMask);
}

} // namespace

void apply_code9_low_height_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept {
    add_half_turn_and_biased_random(state, prng);
}

bool apply_near_hole_lip_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept {
    const auto original_h = static_cast<std::uint32_t>(state.horizontal_force);
    state.vertical_force = static_cast<std::int32_t>(original_h);
    state.horizontal_force = static_cast<std::int32_t>(original_h >> 1);

    const auto random = prng.next(0x0800u);
    const auto delta16 = static_cast<std::uint16_t>(random - 0x0400u);
    const auto signed_delta = static_cast<std::int16_t>(delta16);
    const auto direction = static_cast<std::int64_t>(state.direction)
        + static_cast<std::int64_t>(signed_delta);
    state.direction = static_cast<std::uint16_t>(direction) & kDirectionMask;
    return true;
}

void apply_flag_coordinate_deflection(
    FlightState& state,
    OriginalPrng16& prng) noexcept {
    add_half_turn_and_biased_random(state, prng);

    // The original uses SHR rather than SAR here; keep the unsigned bit-level
    // operation and 32-bit wrap exactly.
    const auto original = static_cast<std::uint32_t>(state.vertical_force);
    const auto boosted = original + (original >> 2);
    state.vertical_force = static_cast<std::int32_t>(boosted);
}

} // namespace sensigolf::recovered
