#include "sensigolf/recovered_alt_terrain.hpp"
#include "sensigolf/recovered_interactions.hpp"

namespace sensigolf::recovered {
OriginalAlternateTerrainResult original_alternate_terrain_contact(
    std::uint16_t landing_code,std::int32_t vertical_force,
    std::int32_t horizontal_force) noexcept {
    OriginalAlternateTerrainResult out{};
    out.vertical_force=vertical_force;
    out.horizontal_force=horizontal_force;
    // Original 0x40CC31/37: equality code35 and code60 means the
    // shared ZERO return at 0x40CC71 WITHOUT mutating either force.
    if(landing_code==35u || landing_code==60u) return out;
    // Original 0x40CC3D: only unsigned [8..10] returns AX=0xffff.
    if(landing_code>=8u && landing_code<=10u) {
        out.route=OriginalAlternateTerrainRoute::ReturnMinusOne;
        return out;
    }
    // Original 0x40CC4E..0x40CC6A: wrapping NEG, arithmetic two SAR,
    // add to H; branch on signed JG. Reuse already verified original
    // 0x40A7BD common rebound machine arithmetic.
    const auto effect=original_terminal_rebound(vertical_force,
                                               horizontal_force);
    out.vertical_force=effect.vertical_force;
    out.horizontal_force=effect.horizontal_force;
    if(effect.continues_flight)
        out.route=OriginalAlternateTerrainRoute::ContinueGround;
    return out;
}
} // namespace sensigolf::recovered
