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

Code9Contact original_code9_contact(
    std::uint16_t landing, std::uint16_t club,
    std::int32_t vertical, std::int32_t height) noexcept {
    // Original 0x40A374: club 12 bypasses the airborne sign check.
    if(club!=12 && vertical>=0) return Code9Contact::None;
    if(height>0x140000 || landing!=9) return Code9Contact::None;
    // Original 0x40A3A3: non-putter with >=3 integer units of ball
    // elevation deflects rather than hitting the event-11 stop path.
    if(club!=12 && height>=0x30000) return Code9Contact::Deflect;
    return Code9Contact::SpecialStop;
}

bool original_flag_coordinate_hit(
    const FlightState& state, std::int16_t flag_x,
    std::int16_t flag_y, bool on_green_map) noexcept {
    if(on_green_map || state.height>0xA0000 || state.height<0)
        return false;
    // Original 0x40A51C: SAR CX,1 uses the signed INTEGER height word.
    const auto x=static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.x)>>16);
    const auto y=static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.y)>>16);
    const auto hi=static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.height)>>16);
    const auto adjusted=static_cast<std::int16_t>(
        static_cast<std::uint16_t>(y)-static_cast<std::uint16_t>(hi>>1));
    return x==flag_x && adjusted==flag_y;
}

bool original_near_cup_lip_contact(
    bool zero_distance, std::uint16_t player_state,
    const FlightState& state) noexcept {
    // Original 0x40A456..0x40A4A8: separate from code-8 capture.
    return zero_distance && state.height<=0x32 && state.height>=0 &&
           player_state<4 && state.horizontal_force>=0xD000;
}

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
