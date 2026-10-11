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

OriginalStopContact original_terminal_stop_contact(
    std::uint16_t landing_code, std::int16_t height_high_word) noexcept {
    // Original v1.014 compares signed WORD[ball + 0x16] with 0xC8,
    // then selects the code-35 branch BEFORE the code-50 branch.
    if (height_high_word <= 0xC8 && landing_code == 0x23)
        return OriginalStopContact::Code35;
    if (landing_code == 0x32)
        return OriginalStopContact::Code50;
    return OriginalStopContact::Other;
}

OriginalTerminalRebound original_terminal_rebound(
    std::int32_t vertical_force, std::int32_t horizontal_force) noexcept {
    // x86 original 0x40A7BD: NEG EBX (32-bit wrapping), SAR 1,
    // write vertical, SAR 1 again, ADD horizontal, JG after ADD.
    // Avoid implementation-defined signed shift, signed overflow and
    // truncation-to-zero (x86 SAR rounds negative numbers DOWN).
    const auto neg_bits=std::uint32_t{0}-static_cast<std::uint32_t>(vertical_force);
    const auto signed_neg=neg_bits<=0x7fffffffu
        ? static_cast<std::int64_t>(neg_bits)
        : static_cast<std::int64_t>(neg_bits)-0x100000000LL;
    const auto half=(signed_neg-(signed_neg<0 ? 1LL : 0LL))/2;
    const auto quarter=(half-(half<0 ? 1LL : 0LL))/2;
    const auto mathematical_sum=static_cast<std::int64_t>(horizontal_force)+quarter;
    if(mathematical_sum<=0)
        return {0,0,false};
    // Preserve 32-bit wrapping resulting ADD even for signed overflow.
    const auto wrapped=static_cast<std::uint32_t>(horizontal_force)
        +static_cast<std::uint32_t>(quarter);
    const auto signed_result=wrapped<=0x7fffffffu
        ? static_cast<std::int64_t>(wrapped)
        : static_cast<std::int64_t>(wrapped)-0x100000000LL;
    return {static_cast<std::int32_t>(half),
            static_cast<std::int32_t>(signed_result),true};
}

OriginalTerminalLatchRebound original_terminal_rebound_with_latch(
    std::uint16_t player_flags,
    std::int32_t vertical_force, std::int32_t horizontal_force) noexcept {
    const bool first_event=(player_flags & 1u)==0;
    // Authentic 0x40A7A5 sets the bit only when it was previously clear.
    const auto flags=static_cast<std::uint16_t>(player_flags | 1u);
    return {original_terminal_rebound(vertical_force,horizontal_force),
            flags,first_event};
}

OriginalSpecialTerminalTerrainResult original_special_terminal_terrain(
    std::uint16_t landing_code, std::uint16_t club_index,
    std::uint16_t player_state_flags, std::int32_t vertical_force,
    std::int32_t horizontal_force, std::int32_t ball_height) noexcept {
    OriginalSpecialTerminalTerrainResult out{
        false,false,player_state_flags,vertical_force,
        horizontal_force,ball_height,0
    };
    const auto height_hi=static_cast<std::uint32_t>(ball_height)>>16;
    const auto signed_hi=height_hi>=0x8000u
        ? static_cast<std::int32_t>(height_hi)-0x10000
        : static_cast<std::int32_t>(height_hi);
    if(landing_code==35u && signed_hi<=200) {
        // Original low-height code-35 branch 0x40A6E3..0x40A72B,
        // after unrelated rendering / audio calls (not replicated here).
        out.handled=true;
        out.vertical_force=0;
        out.horizontal_force=0;
        out.ball_height=0;
        out.player_state_flags=
            static_cast<std::uint16_t>(player_state_flags|0x8000u);
        out.event_code=11;
        return out;
    }
    if(landing_code==50u) {
        // Original 0x40A6AB event-code-1 route. The putter (club 12)
        // exits by marking bit 15; all other clubs enter the original
        // shared rebound calculation without the code-0 latch.
        out.handled=true;
        out.event_code=1;
        if(club_index==12u) {
            out.player_state_flags=
                static_cast<std::uint16_t>(player_state_flags|0x8000u);
            return out;
        }
        const auto bounced=original_terminal_rebound(
            vertical_force,horizontal_force);
        out.vertical_force=bounced.vertical_force;
        out.horizontal_force=bounced.horizontal_force;
        out.continues_flight=bounced.continues_flight;
    }
    return out;
}


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
    if(on_green_map || state.height>0xA0000)
        return false;
    // Original v1.014 0x40A50B..0x40A538:
    // DX = WORD[ball + 2], NOT WORD[ball + 6]. This is the high X
    // word. The source compares X to flag X, and (X - height_hi/2)
    // to flag Y. Ball Y is NOT referenced in this branch.
    // Preserve x86 signed SAR and 16-bit wrap rather than assuming
    // physically intuitive 2D coordinates.
    const auto x=static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.x)>>16);
    const auto hi=static_cast<std::int16_t>(
        static_cast<std::uint32_t>(state.height)>>16);
    const auto height_half=static_cast<std::int16_t>(hi >> 1);
    const auto adjusted=static_cast<std::int16_t>(
        static_cast<std::uint16_t>(x)
        - static_cast<std::uint16_t>(height_half));
    return x==flag_x && adjusted==flag_y;
}

bool original_near_cup_lip_contact(
    bool zero_distance, std::uint16_t player_state,
    const FlightState& state) noexcept {
    // Original 0x40A456..0x40A4A8: separate from code-8 capture.
    return zero_distance && state.height<=0x32 &&
           static_cast<std::int16_t>(player_state)<4 &&
           state.horizontal_force>=0xD000;
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
