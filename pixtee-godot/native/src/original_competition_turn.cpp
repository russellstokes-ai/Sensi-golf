#include "original_competition_turn.hpp"
#include <stdexcept>
namespace pixtee {
OriginalCompetitionTurnSelection original_competition_turn_select(
    const OriginalCompetitionTurnInput& state) {
    OriginalCompetitionTurnSelection result{};
    result.active_side=state.active_side;
    result.previous_slot=state.previous_slot;
    if(state.mode!=4 && state.mode!=6)
        return result; // 0x40C0A8 -> 0x40C1B7 ordinary player chooser
    const auto& slots=state.slot_bytes;
    if(state.side0_flag>1)
        throw std::invalid_argument("original mode pointer slot offset must be 0..1");
    // Original 0x40C098..0x40C110: mode-specific side-selection branch.
    const bool select_first=(state.active_side!=0 && state.ball_distance[0]!=0)
        || state.ball_distance[1]==0;
    if(select_first) {
        result.active_side=0;
        std::uint8_t target=slots[0],src=slots[0];
        if(state.side0_flag!=1) {
            src=slots[1];
            if(target==state.previous_slot[0]) {
                target=slots[1];
                src=slots[0];
            }
        }
        result.previous_slot[0]=target;
        result.selected_slot=target;
        result.source_slot=src;
        result.ball_array=0;
    } else {
        result.active_side=1;
        const auto off=state.side0_flag;
        std::uint8_t target=slots[off],src=slots[off];
        if(state.side1_flag!=1) {
            src=slots[off+1];
            if(target==state.previous_slot[1]) {
                target=slots[off+1];
                src=slots[off];
            }
        }
        result.previous_slot[1]=target;
        result.selected_slot=target;
        result.source_slot=src;
        result.ball_array=1;
    }
    return result;
}
}
