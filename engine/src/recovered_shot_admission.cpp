#include "sensigolf/recovered_shot_admission.hpp"

namespace sensigolf::recovered {

OriginalShotAdmissionResult original_shot_tick_admission(
    OriginalShotAdmissionState state) noexcept {
    OriginalShotAdmissionResult out{};
    out.state=state;
    // SHA-verified original 0x40A2B3..0x40A2CE, in exact priority.
    if((state.player_flags & 0x8000u)!=0u) {
        out.route=OriginalShotAdmissionRoute::PlayerBit15Wait;
        return out;
    }
    if((state.player_flags & 0x0004u)!=0u) {
        out.route=OriginalShotAdmissionRoute::PlayerBit2Set;
        return out;
    }
    if((state.player_flags & 0x0002u)==0u) {
        out.route=OriginalShotAdmissionRoute::PlayerBit1Clear;
        return out;
    }
    // 0x40A2D4 uses JB, UNSIGNED word mode comparator.
    // 0x40A2DB tests source mode_5c exactly against 2.
    if(state.player_mode_5a<4u || state.player_mode_5c==2u) {
        out.route=OriginalShotAdmissionRoute::ActiveTerrainTick;
        return out;
    }
    // Source 0x40A2E2..0x40A2F2; the callback's effects are NOT
    // modeled here, only the exact hand-off and direct source writes.
    out.state.player_mode_5c=2u;
    out.cpu_delegate_requested=true;
    out.state.player_flags=0u;
    out.route=OriginalShotAdmissionRoute::CpuTurnDelegate;
    return out;
}

} // namespace sensigolf::recovered
