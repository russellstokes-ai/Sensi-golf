#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
#include "original_turn_orchestrator.hpp"
using namespace pixtee;
int main(){
    OriginalTurnRequest r{};
    r.original_mode=0;
    r.ordinary.count=4;
    r.ordinary.priority_metric={7,9,9,3};
    auto sel=original_decide_next_turn(r);
    assert(sel.active_player==2); // original ties prefer LAST eligible
    assert(!sel.used_competition_controller);
    r.ordinary.eliminated[2]=1;
    sel=original_decide_next_turn(r);
    assert(sel.active_player==1);
    r.ordinary.forced_holder=1; // original target identity = count -1
    sel=original_decide_next_turn(r);
    assert(sel.active_player==3);
    r.ordinary.forced_holder=0;
    r.original_mode=4;
    r.competition.mode=4;
    r.competition.active_side=1;
    r.competition.ball_distance={10,20};
    r.competition.side0_flag=1;
    r.competition.slot_bytes={0,1,2,3};
    sel=original_decide_next_turn(r);
    assert(sel.used_competition_controller);
    assert(sel.active_player==0 && sel.source_player==0);
    assert(sel.next_side==0 && sel.active_ball_array==0);
    assert(!sel.actual_player_state_copied);
    OriginalTurnOrchestrator stage;
    stage.select_next(r);
    assert(stage.has_pending_turn());
    bool refused=false;
    try{stage.select_next(r);}catch(const std::logic_error&){refused=true;}
    assert(refused); // no duplicate turn switches while waiting
    assert(stage.pending_turn().active_player==0);
    auto ack=stage.acknowledge_owner_switch();
    assert(ack.active_player==0 && !stage.has_pending_turn());
    refused=false;
    try{stage.acknowledge_owner_switch();}
    catch(const std::logic_error&){refused=true;}
    assert(refused);
    r.competition.ball_distance={0,0};
    refused=false;
    try{stage.select_next(r);}catch(const std::logic_error&){refused=true;}
    assert(refused);
    assert(!stage.has_pending_turn()); // unhandled original loop not concealed
    r.original_mode=6;
    r.competition.ball_distance={0,2};
    r.competition.active_side=0;
    r.competition.side0_flag=0;
    r.competition.side1_flag=1;
    sel=original_decide_next_turn(r);
    assert(sel.used_competition_controller && sel.active_ball_array==1);
    r.ordinary.count=1;
    refused=false;
    try{original_decide_next_turn(r);}
    catch(const std::logic_error&){refused=true;}
    assert(refused); // no nonexistent player state
    return 0;
}
