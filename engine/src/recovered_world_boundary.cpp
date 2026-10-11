#include "sensigolf/recovered_world_boundary.hpp"
#include <cstdint>

namespace sensigolf::recovered {
OriginalWorldBoundaryResult original_world_boundary_check(
    const OriginalWorldBoundaryInput& input) noexcept {
    OriginalWorldBoundaryResult out{};
    out.player_flags=input.player_flags;
    out.strokes_on_hole=input.strokes_on_hole;
    out.strokes_total=input.strokes_total;
    out.ball_pause=input.ball_pause;
    out.game_gate=input.game_gate;
    // Source 0x409535 MOVSX of signed 16-bit coordinate before bounds
    // check, followed by signed JGE against x_extent+0x100 and
    // y_extent+0xD0. The configured course dimensions do not overflow
    // signed 32-bit extents.
    const auto xhi=static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(input.ball_x)>>16);
    const auto yhi=static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(input.ball_y)>>16);
    const auto x=xhi<=0x7fffu
        ? static_cast<std::int32_t>(xhi)
        : static_cast<std::int32_t>(xhi)-65536;
    const auto y=yhi<=0x7fffu
        ? static_cast<std::int32_t>(yhi)
        : static_cast<std::int32_t>(yhi)-65536;
    const auto xmax=static_cast<std::int64_t>(input.x_extent)+0x100;
    const auto ymax=static_cast<std::int64_t>(input.y_extent)+0xD0;
    out.outside_world=x<0 || y<0 || x>=xmax || y>=ymax;
    out.source_map_ax=out.outside_world ? static_cast<std::int16_t>(-1) : 0;
    if(!out.outside_world)return out;

    // Original 0x40A318 JS -> 0x40A327..0x40A34C, only when
    // current green mode !=1. Both player stroke counters increment;
    // the source sets bit2 and finally writes literal flags=4.
    // Renderer callback 0x40A22B and 0x40CD00 downstream are separate.
    out.strokes_on_hole=static_cast<std::uint16_t>(
        input.strokes_on_hole+1u);
    out.strokes_total=static_cast<std::uint16_t>(
        input.strokes_total+1u);
    out.player_flags=4;
    out.ball_pause=100;
    out.game_gate=1;
    out.requires_out_of_bounds_dispatch=true;
    return out;
}
OriginalWorldOobRestoreResult original_world_oob_restore(
    const OriginalWorldOobRestoreInput& input) noexcept {
    OriginalWorldOobRestoreResult out{};
    out.relocated=recover_hazard_position(input.coordinates);
    // Original 0x40AADA checks signed WORD mode against 4 (JLE).
    const auto mode=input.player_mode_5a;
    const auto signed_mode=mode<=0x7fffu
        ? static_cast<std::int32_t>(mode)
        : static_cast<std::int32_t>(mode)-65536;
    out.player_mode_62=signed_mode>4 ? static_cast<std::uint16_t>(1)
                                   : input.player_mode_62;
    out.player_mode_5c=0; // original 0x40A8AB
    return out;
}

} // namespace sensigolf::recovered
