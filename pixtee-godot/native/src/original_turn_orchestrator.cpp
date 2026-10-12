#include "original_turn_orchestrator.hpp"
namespace pixtee {
OriginalTurnDecision original_decide_next_turn(
    const OriginalTurnRequest& req) {
    if(req.ordinary.count<1 || req.ordinary.count>4)
        throw std::invalid_argument("original turn owner requires 1..4 players");
    OriginalTurnDecision out{};
    if(req.original_mode==4 || req.original_mode==6) {
        auto p=req.competition;
        p.mode=req.original_mode;
        auto selected=original_competition_turn_select(p);
        if(selected.selected_slot<0 ||
           selected.selected_slot>=req.ordinary.count ||
           selected.source_slot<0 ||
           selected.source_slot>=req.ordinary.count)
            throw std::logic_error(
                "source competition turn selected outside registered player slots");
        out.used_competition_controller=true;
        out.active_player=static_cast<std::uint8_t>(selected.selected_slot);
        out.active_ball_array=selected.ball_array;
        out.source_player=selected.source_slot;
        out.next_side=selected.active_side;
        return out;
    }
    out.active_player=original_next_player_index(req.ordinary);
    return out;
}
void OriginalTurnOrchestrator::select_next(const OriginalTurnRequest& req) {
    if(pending_)
        throw std::logic_error(
            "original turn owner not yet acknowledged");
    next_=original_decide_next_turn(req);
    pending_=true;
}
const OriginalTurnDecision& OriginalTurnOrchestrator::pending_turn() const {
    if(!pending_)
        throw std::logic_error("no original turn switch pending");
    return next_;
}
OriginalTurnDecision OriginalTurnOrchestrator::acknowledge_owner_switch() {
    if(!pending_)
        throw std::logic_error("cannot acknowledge absent original turn");
    pending_=false;
    return next_;
}
void OriginalTurnOrchestrator::reset() noexcept {
    pending_=false;
    next_={};
}
} // namespace pixtee
