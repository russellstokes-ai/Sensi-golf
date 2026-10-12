#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>
#include "pixtee_multi_round_runtime.hpp"
namespace {
void be16(std::vector<std::uint8_t>& v,std::size_t p,std::uint16_t x){
    v.at(p)=static_cast<std::uint8_t>(x>>8);
    v.at(p+1)=static_cast<std::uint8_t>(x);
}
sensigolf::ClassicCourseResources fixture() {
    std::vector<std::uint8_t> map(0x60+4*128*2,0),spt(50,0);
    std::vector<std::uint8_t> desc(8,0),sel(8,0);
    be16(map,0x54,4);be16(map,0x56,128);
    be16(spt,46,64); // away from zero cup distance
    be16(desc,0,7);
    return {std::move(map),std::move(spt),std::move(desc),std::move(sel)};
}
}
int main(){
    std::array<std::uint8_t,18> order{};
    std::array<std::uint8_t,100> pars{};
    for(unsigned i=0;i<18;i++){order[i]=static_cast<std::uint8_t>(i+1);pars[i+1]=4;}
    std::array<sensigolf::recovered::OriginalPrng16,4> seeds{{
        {1,2},{3,4},{5,6},{7,8}
    }};
    pixtee::PixteeMultiRoundRuntime game(2,order,pars,seeds);
    for(unsigned i=1;i<=18;i++)game.register_course(i,fixture());
    game.load_next_hole_for_everyone();
    assert(!game.has_active_owner());
    pixtee::OriginalTurnRequest req{};
    req.ordinary.count=2;
    req.ordinary.priority_metric={3,7,0,0};
    game.propose_original_turn(req);
    assert(game.turn_switch_pending());
    bool blocked=false;
    try{game.active_round();}catch(const std::logic_error&){blocked=true;}
    assert(blocked);
    assert(game.acknowledge_original_turn()==1);
    assert(game.active_owner()==1);
    auto& one=game.active_round();
    one.set_club(12);
    one.meter_click_raw(0);
    one.meter_click_raw(30);
    one.meter_click_raw(105); // source paid rejected/no-flight
    assert(one.hole().strokes()==1);
    assert(game.player_round(0).hole().strokes()==0);
    assert(game.player_round(1).hole().recovered_counters().player_52==1);
    assert(game.player_round(0).hole().recovered_counters().player_52==0);
    assert(game.player_round(1).saved_shots()==1);
    assert(game.player_round(0).saved_shots()==0);
    req.ordinary.priority_metric={10,7,0,0};
    game.propose_original_turn(req);
    assert(game.acknowledge_original_turn()==0);
    auto& zero=game.active_round();
    zero.set_club(12);
    zero.meter_click_raw(0);
    zero.meter_click_raw(30);
    zero.meter_click_raw(105);
    assert(zero.hole().strokes()==1);
    assert(game.player_round(1).hole().strokes()==1);
    assert(game.player_round(0).serialize_stable_replay()!=
           game.player_round(1).serialize_stable_replay() ||
           game.player_round(0).hole().recovered_counters().player_52==
           game.player_round(1).hole().recovered_counters().player_52);
    return 0;
}
