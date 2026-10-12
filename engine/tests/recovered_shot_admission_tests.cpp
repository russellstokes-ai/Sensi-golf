#include <cassert>
#include <cstdint>
#include "sensigolf/recovered_shot_admission.hpp"
using namespace sensigolf::recovered;
int main() {
    auto gate=original_shot_tick_admission({0x8006,0xffff,0});
    assert(gate.route==OriginalShotAdmissionRoute::PlayerBit15Wait);
    assert(!gate.cpu_delegate_requested && gate.state.player_flags==0x8006);
    gate=original_shot_tick_admission({4,0xffff,0});
    assert(gate.route==OriginalShotAdmissionRoute::PlayerBit2Set);
    gate=original_shot_tick_admission({0,0xffff,0});
    assert(gate.route==OriginalShotAdmissionRoute::PlayerBit1Clear);
    gate=original_shot_tick_admission({2,3,7});
    assert(gate.route==OriginalShotAdmissionRoute::ActiveTerrainTick);
    assert(gate.state.player_mode_5c==7);
    gate=original_shot_tick_admission({2,4,2});
    assert(gate.route==OriginalShotAdmissionRoute::ActiveTerrainTick);
    gate=original_shot_tick_admission({2,4,1});
    assert(gate.route==OriginalShotAdmissionRoute::CpuTurnDelegate);
    assert(gate.cpu_delegate_requested);
    assert(gate.state.player_mode_5c==2 && gate.state.player_flags==0);
    // The original JAE/JB branch interprets 0xFFFF as unsigned mode.
    gate=original_shot_tick_admission({2,0xffff,0xffff});
    assert(gate.route==OriginalShotAdmissionRoute::CpuTurnDelegate);
    assert(gate.state.player_flags==0);
    gate=original_shot_tick_admission({0xffff,0,0});
    assert(gate.route==OriginalShotAdmissionRoute::PlayerBit15Wait);
}
