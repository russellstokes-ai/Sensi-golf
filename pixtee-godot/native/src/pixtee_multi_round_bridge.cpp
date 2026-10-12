#include "pixtee_multi_round_bridge.hpp"
#include <array>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>
#include "godot_cpp/core/class_db.hpp"

namespace godot {
namespace {
std::vector<std::uint8_t> as_bytes(const PackedByteArray& v) {
    std::vector<std::uint8_t> r;
    r.reserve(static_cast<std::size_t>(v.size()));
    for(int i=0;i<v.size();++i)r.push_back(v[i]);
    return r;
}
}
void PixteeMultiRoundBridge::_bind_methods() {
    ClassDB::bind_method(D_METHOD("begin_multi_round",
        "players","order","pars","seed0_by_player","seed1_by_player"),
        &PixteeMultiRoundBridge::begin_multi_round);
    ClassDB::bind_method(D_METHOD("register_course_resource",
        "resource_id","mapm","spt","descriptors","selectors","maps"),
        &PixteeMultiRoundBridge::register_course_resource);
    ClassDB::bind_method(D_METHOD("load_next_hole_for_everyone"),
        &PixteeMultiRoundBridge::load_next_hole_for_everyone);
    ClassDB::bind_method(D_METHOD("propose_original_turn",
        "forced_holder","priorities","eliminated","identities"),
        &PixteeMultiRoundBridge::propose_original_turn);
    ClassDB::bind_method(D_METHOD("propose_original_competition_turn",
        "mode","active_side","distance0","distance1","side0_flag",
        "side1_flag","slot_bytes","previous_slots"),
        &PixteeMultiRoundBridge::propose_original_competition_turn);
    ClassDB::bind_method(D_METHOD("acknowledge_original_turn"),
        &PixteeMultiRoundBridge::acknowledge_original_turn);
    ClassDB::bind_method(D_METHOD("set_club_index","club"),
        &PixteeMultiRoundBridge::set_club_index);
    ClassDB::bind_method(D_METHOD("meter_click_raw","sample"),
        &PixteeMultiRoundBridge::meter_click_raw);
    ClassDB::bind_method(D_METHOD("start_original_welly"),
        &PixteeMultiRoundBridge::start_original_welly);
    ClassDB::bind_method(D_METHOD("tick_original_welly","pressed"),
        &PixteeMultiRoundBridge::tick_original_welly);
    ClassDB::bind_method(D_METHOD("step_tick"),
        &PixteeMultiRoundBridge::step_tick);
    ClassDB::bind_method(D_METHOD("acknowledge_hazard"),
        &PixteeMultiRoundBridge::acknowledge_hazard);
    ClassDB::bind_method(D_METHOD("acknowledge_special_green"),
        &PixteeMultiRoundBridge::acknowledge_special_green);
    ClassDB::bind_method(D_METHOD("commit_scored_hole"),
        &PixteeMultiRoundBridge::commit_scored_hole);
    ClassDB::bind_method(D_METHOD("snapshot"),
        &PixteeMultiRoundBridge::snapshot);
    ClassDB::bind_method(D_METHOD("last_error"),
        &PixteeMultiRoundBridge::last_error);
}
bool PixteeMultiRoundBridge::begin_multi_round(
    int n,const PackedInt32Array& order,
    const PackedInt32Array& pars,const PackedInt32Array& seed0,
    const PackedInt32Array& seed1) {
    if(n<1 || n>4 || order.size()!=18 || pars.size()!=100 ||
       seed0.size()!=n || seed1.size()!=n) {
        last_error_="original multiplayer needs 1..4 players, 18 hole order, 100 pars and per-player PRNG seeds";
        return false;
    }
    try {
        std::array<std::uint8_t,18> ids{};
        std::array<std::uint8_t,100> table{};
        std::array<sensigolf::recovered::OriginalPrng16,4> seeds{};
        for(int i=0;i<18;++i) {
            if(order[i]<0 || order[i]>99)
                throw std::invalid_argument("original resource ID outside 0..99");
            ids[i]=static_cast<std::uint8_t>(order[i]);
        }
        for(int i=0;i<100;++i) {
            if(pars[i]<0 || pars[i]>255)
                throw std::invalid_argument("original par byte outside 0..255");
            table[i]=static_cast<std::uint8_t>(pars[i]);
        }
        for(int i=0;i<n;++i) {
            if(seed0[i]<0 || seed0[i]>65535 ||
               seed1[i]<0 || seed1[i]>65535)
                throw std::invalid_argument("original PRNG seed outside 16 bits");
            seeds[i]={static_cast<std::uint16_t>(seed0[i]),
                      static_cast<std::uint16_t>(seed1[i])};
        }
        auto candidate=std::make_unique<pixtee::PixteeMultiRoundRuntime>(
            static_cast<std::uint8_t>(n),ids,table,seeds);
        game_=std::move(candidate);
        last_error_="";
        return true;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
bool PixteeMultiRoundBridge::register_course_resource(
    int id,const PackedByteArray& mapm,const PackedByteArray& spt,
    const PackedByteArray& desc,const PackedByteArray& selector,
    const PackedByteArray& maps) {
    if(!game_ || id<0 || id>99) {
        last_error_="multiplayer round unavailable or course ID invalid";
        return false;
    }
    try {
        sensigolf::ClassicCourseResources course(
            as_bytes(mapm),as_bytes(spt),as_bytes(desc),
            as_bytes(selector),as_bytes(maps));
        game_->register_course(static_cast<std::uint8_t>(id),course);
        last_error_="";
        return true;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
bool PixteeMultiRoundBridge::load_next_hole_for_everyone() {
    if(!game_) {last_error_="multiplayer round not initialized";return false;}
    try {
        game_->load_next_hole_for_everyone();
        last_error_="";
        return true;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
bool PixteeMultiRoundBridge::propose_original_turn(
    int forced,const PackedInt32Array& priority,
    const PackedInt32Array& eliminated,const PackedInt32Array& identities) {
    if(!game_){last_error_="multiplayer round not initialized";return false;}
    const int count=static_cast<int>(game_->participants());
    if(forced<0 || forced>count || priority.size()!=count ||
       eliminated.size()!=count || identities.size()!=count) {
        last_error_="original player choice requires valid player records";
        return false;
    }
    try {
        pixtee::OriginalTurnRequest req{};
        req.ordinary.count=static_cast<std::uint8_t>(count);
        req.ordinary.forced_holder=static_cast<std::uint8_t>(forced);
        for(int i=0;i<count;++i) {
            if(priority[i]<0 || eliminated[i]<0 || eliminated[i]>65535 ||
                identities[i]<0 || identities[i]>255)
                throw std::invalid_argument("original player values outside source ranges");
            req.ordinary.priority_metric[i]=static_cast<std::uint32_t>(priority[i]);
            req.ordinary.eliminated[i]=static_cast<std::uint16_t>(eliminated[i]);
            req.ordinary.identity[i]=static_cast<std::uint8_t>(identities[i]);
        }
        game_->propose_original_turn(req);
        last_error_="";
        return true;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
bool PixteeMultiRoundBridge::propose_original_competition_turn(
    int mode,int side,int d0,int d1,int f0,int f1,
    const PackedInt32Array& slots,const PackedInt32Array& previous) {
    if(!game_){last_error_="multiplayer round not initialized";return false;}
    if((mode!=4&&mode!=6)||side<0||side>65535||d0<0||d1<0||
       f0<0||f0>65535||f1<0||f1>65535||slots.size()!=4 ||
       previous.size()!=2) {
        last_error_="original competition requires mode4/6 and complete source state";
        return false;
    }
    try {
        pixtee::OriginalTurnRequest req{};
        req.original_mode=static_cast<std::uint8_t>(mode);
        req.ordinary.count=static_cast<std::uint8_t>(game_->participants());
        req.competition.active_side=static_cast<std::uint16_t>(side);
        req.competition.ball_distance={
          static_cast<std::uint32_t>(d0),static_cast<std::uint32_t>(d1)};
        req.competition.side0_flag=static_cast<std::uint16_t>(f0);
        req.competition.side1_flag=static_cast<std::uint16_t>(f1);
        for(int i=0;i<4;++i) {
            if(slots[i]<0 || slots[i]>255)
                throw std::invalid_argument("original competition slot byte invalid");
            req.competition.slot_bytes[i]=static_cast<std::uint8_t>(slots[i]);
        }
        for(int i=0;i<2;++i) {
            if(previous[i]<0 || previous[i]>65535)
                throw std::invalid_argument("original previous slot invalid");
            req.competition.previous_slot[i]=static_cast<std::uint16_t>(previous[i]);
        }
        game_->propose_original_turn(req);
        last_error_="";
        return true;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
int PixteeMultiRoundBridge::acknowledge_original_turn() {
    if(!game_){last_error_="multiplayer round not initialized";return -1;}
    try {
        const auto player=game_->acknowledge_original_turn();
        last_error_="";
        return static_cast<int>(player);
    }catch(const std::exception& e){last_error_=String(e.what());return -1;}
}
#define PIXTEE_MULTI_BOOL(NAME,CALL) \
bool PixteeMultiRoundBridge::NAME { \
    if(!game_){last_error_="multiplayer round not initialized";return false;} \
    try {CALL;last_error_="";return true;} \
    catch(const std::exception& e){last_error_=String(e.what());return false;} \
}
PIXTEE_MULTI_BOOL(set_club_index(int c),
    game_->active_round().set_club(static_cast<std::uint16_t>(c)))
PIXTEE_MULTI_BOOL(meter_click_raw(int s),
    game_->active_round().meter_click_raw(static_cast<std::uint16_t>(s)))
PIXTEE_MULTI_BOOL(start_original_welly(),
    game_->active_round().start_original_welly())
PIXTEE_MULTI_BOOL(acknowledge_hazard(),
    game_->active_round().acknowledge_hazard())
PIXTEE_MULTI_BOOL(acknowledge_special_green(),
    game_->active_round().acknowledge_special_green())
PIXTEE_MULTI_BOOL(commit_scored_hole(),
    game_->active_round().commit_scored_hole())
#undef PIXTEE_MULTI_BOOL
bool PixteeMultiRoundBridge::tick_original_welly(bool pressed) {
    if(!game_){last_error_="multiplayer round not initialized";return false;}
    try {
        const bool accepted=game_->active_round().tick_original_welly(pressed);
        last_error_="";
        return accepted;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
void PixteeMultiRoundBridge::step_tick() {
    if(!game_)return;
    try{game_->active_round().step_tick();last_error_="";}
    catch(const std::exception& e){last_error_=String(e.what());}
}
Dictionary PixteeMultiRoundBridge::snapshot() const {
    Dictionary out;
    out["initialized"]=game_!=nullptr;
    out["error"]=last_error_;
    if(!game_)return out;
    out["participants"]=static_cast<int>(game_->participants());
    out["turn_switch_pending"]=game_->turn_switch_pending();
    out["owner_assigned"]=game_->has_active_owner();
    PackedInt32Array holes,strokes,totals,saved,phases;
    try {
        out["active_owner"]=game_->has_active_owner() &&
            !game_->turn_switch_pending()
            ? static_cast<int>(game_->active_owner()) : -1;
        for(std::size_t i=0;i<game_->participants();++i) {
            const auto& player=game_->player_round(i);
            const auto& score=player.round_state();
            holes.push_back(static_cast<int>(score.holes_completed));
            totals.push_back(static_cast<int>(score.total_strokes));
            saved.push_back(static_cast<int>(player.saved_shots()));
            strokes.push_back(player.has_hole()
                ? static_cast<int>(player.hole().strokes()) : -1);
            phases.push_back(player.has_hole()
                ? static_cast<int>(player.hole().phase()) : -1);
        }
    }catch(const std::exception& e){
        out["snapshot_error"]=String(e.what());
    }
    out["holes_completed"]=holes;
    out["total_strokes"]=totals;
    out["saved_shots"]=saved;
    out["hole_strokes"]=strokes;
    out["hole_phases"]=phases;
    out["complete_original_tournament_logic"]=false;
    return out;
}
} // namespace godot
