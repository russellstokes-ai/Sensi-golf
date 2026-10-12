#include "sensigolf/recovered_mapped_terminal.hpp"

#include "sensigolf/classic_terrain.hpp"
#include "sensigolf/recovered_course_lookup.hpp"
#include "sensigolf/recovered_interactions.hpp"

namespace sensigolf::recovered {

OriginalMappedTerminalResult original_mapped_terminal_contact(
    const OriginalMappedTerminalInput& input) {
    const auto subcell=lookup_course_subcell(
        input.descriptor_bank,input.descriptor_size,
        input.selector_bank,input.selector_size,
        input.tile_index,input.x_subcell,input.y_subcell);
    const auto& descriptor=classic_terrain_descriptor(subcell.descriptor_index);
    OriginalMappedTerminalResult out{};
    out.descriptor_index=subcell.descriptor_index;
    out.landing_code=descriptor.landing_code;
    out.player_flags=input.player_flags;
    out.contact_flags=input.contact_flags;
    out.vertical_force=input.vertical_force;
    out.horizontal_force=input.horizontal_force;
    out.ball_height=input.ball_height;

    // The original 0x40A65E checks whether vertical force is nonzero before
    // calling the map lookup; return source-inactive but still report the
    // independently computed native lookup. No effects are applied.
    if(input.vertical_force==0 || out.landing_code==8u)
        return out;

    const auto special=original_special_terminal_terrain(
        out.landing_code,input.club_index,input.player_flags,
        input.vertical_force,input.horizontal_force,input.ball_height);
    if(special.handled) {
        out.resolved=true;
        out.player_flags=special.player_state_flags;
        out.vertical_force=special.vertical_force;
        out.horizontal_force=special.horizontal_force;
        out.ball_height=special.ball_height;
        out.continues_flight=special.continues_flight;
        out.event_code=special.event_code;
        return out;
    }
    // Original 0x40A730 maps every non-8 ordinary landing code to
    // 0x40A79D contact latch + 0x40A7BD rebound. Here code 35 above
    // signed height-hi 200 enters this fallback as well.
    const auto common=original_terminal_rebound_with_latch(
        input.contact_flags,input.vertical_force,input.horizontal_force);
    out.resolved=true;
    out.contact_flags=common.player_flags;
    out.vertical_force=common.rebound.vertical_force;
    out.horizontal_force=common.rebound.horizontal_force;
    out.continues_flight=common.rebound.continues_flight;
    out.event_code=common.event_zero_requested ? 0 : -1;
    return out;
}


OriginalPutterAdmissionResult original_putter_terrain_admission(
    const OriginalMappedTerminalInput& input,
    std::uint8_t original_bounce_marker) {
    OriginalPutterAdmissionResult out{};
    // 0x40A575 CMP signed WORD[ball + 0x16], 20; 0x40A57C clears
    // ONLY contact flag bit zero when the signed high WORD is >= 20.
    const auto high=static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(input.ball_height)>>16);
    const auto signed_high=high<=0x7fffu
        ? static_cast<std::int32_t>(high)
        : static_cast<std::int32_t>(high)-0x10000;
    out.contact_flags=static_cast<std::uint16_t>(
        input.contact_flags & (signed_high>=20 ? 0xfffeu : 0xffffu));

    // Original 0x40A581/0x40A588: non-putters OR byte marker == 1
    // skip the early second MAPI lookup and enter 0x40A603.
    if(input.club_index!=12u || original_bounce_marker==1u)
        return out;

    const auto cell=lookup_course_subcell(
        input.descriptor_bank,input.descriptor_size,
        input.selector_bank,input.selector_size,
        input.tile_index,input.x_subcell,input.y_subcell);
    out.map_lookup_performed=true;
    out.descriptor_index=cell.descriptor_index;
    out.landing_code=classic_terrain_descriptor(
        cell.descriptor_index).landing_code;

    const auto code=out.landing_code;
    if(static_cast<std::int16_t>(code)<8)
        out.route=OriginalPutterAdmissionRoute::RollingSurface;
    else if(code==8u)
        out.route=OriginalPutterAdmissionRoute::CupCode8;
    else if(code==9u || code==10u)
        out.route=OriginalPutterAdmissionRoute::SpecialGreenCode9Or10;
    else if(code==35u && signed_high<=200)
        out.route=OriginalPutterAdmissionRoute::Code35LowHeight;
    else
        out.route=OriginalPutterAdmissionRoute::HighCodeEvent1;
    return out;
}

} // namespace sensigolf::recovered
