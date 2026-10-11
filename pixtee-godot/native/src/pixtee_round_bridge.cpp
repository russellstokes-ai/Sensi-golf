#include "pixtee_round_bridge.hpp"
#include <array>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include "godot_cpp/core/class_db.hpp"

namespace godot {
namespace {
std::vector<std::uint8_t> native_bytes(const PackedByteArray& a) {
    std::vector<std::uint8_t> out;
    out.reserve(static_cast<std::size_t>(a.size()));
    for(int i=0;i<a.size();++i) out.push_back(a[i]);
    return out;
}
}
void PixteeRoundBridge::_bind_methods() {
    ClassDB::bind_method(D_METHOD("begin_round","order","pars","seed0","seed1"),
                         &PixteeRoundBridge::begin_round);
    ClassDB::bind_method(D_METHOD("register_course_resource","resource_id","mapm","spt","descriptors","selectors","maps"),
                         &PixteeRoundBridge::register_course_resource);
    ClassDB::bind_method(D_METHOD("load_next_hole"),&PixteeRoundBridge::load_next_hole);
    ClassDB::bind_method(D_METHOD("set_aim_raw","heading"),&PixteeRoundBridge::set_aim_raw);
    ClassDB::bind_method(D_METHOD("nudge_aim_raw","delta"),&PixteeRoundBridge::nudge_aim_raw);
    ClassDB::bind_method(D_METHOD("set_club_index","club"),&PixteeRoundBridge::set_club_index);
    ClassDB::bind_method(D_METHOD("cycle_club","delta"),&PixteeRoundBridge::cycle_club);
    ClassDB::bind_method(D_METHOD("meter_click_raw","sample"),&PixteeRoundBridge::meter_click_raw);
    ClassDB::bind_method(D_METHOD("cancel_meter"),&PixteeRoundBridge::cancel_meter);
    ClassDB::bind_method(D_METHOD("acknowledge_hazard"),&PixteeRoundBridge::acknowledge_hazard);
    ClassDB::bind_method(D_METHOD("acknowledge_special_green"),&PixteeRoundBridge::acknowledge_special_green);
    ClassDB::bind_method(D_METHOD("commit_scored_hole"),&PixteeRoundBridge::commit_scored_hole);
    ClassDB::bind_method(D_METHOD("step_tick"),&PixteeRoundBridge::step_tick);
    ClassDB::bind_method(D_METHOD("step_ticks","count"),&PixteeRoundBridge::step_ticks);
    ClassDB::bind_method(D_METHOD("save_stable_replay"),&PixteeRoundBridge::save_stable_replay);
    ClassDB::bind_method(D_METHOD("restore_replay","payload"),&PixteeRoundBridge::restore_replay);
    ClassDB::bind_method(D_METHOD("snapshot"),&PixteeRoundBridge::snapshot);
    ClassDB::bind_method(D_METHOD("requested_course"),&PixteeRoundBridge::requested_course);
    ClassDB::bind_method(D_METHOD("last_error"),&PixteeRoundBridge::last_error);
}
bool PixteeRoundBridge::begin_round(
    const PackedInt32Array& order,const PackedInt32Array& pars,
    int seed0,int seed1) {
    if(order.size()!=18 || pars.size()!=100 || seed0<0 || seed0>65535 ||
       seed1<0 || seed1>65535) {
        last_error_="18 order slots, 100 par slots and two 16-bit seeds are required";
        return false;
    }
    try {
        std::array<std::uint8_t,18> ids{};
        std::array<std::uint8_t,100> par_by_id{};
        for(int i=0;i<18;++i) {
            if(order[i]<0 || order[i]>=100)
                throw std::invalid_argument("resource ID outside 0..99");
            ids[static_cast<std::size_t>(i)]=static_cast<std::uint8_t>(order[i]);
        }
        for(int i=0;i<100;++i) {
            if(pars[i]<0 || pars[i]>9)
                throw std::invalid_argument("par outside original supported 0..9 table");
            par_by_id[static_cast<std::size_t>(i)]=static_cast<std::uint8_t>(pars[i]);
        }
        auto created=std::make_unique<pixtee::PixteeRoundRuntime>(
            ids,par_by_id,static_cast<std::uint16_t>(seed0),
            static_cast<std::uint16_t>(seed1));
        round_=std::move(created);
        last_error_="";
        return true;
    } catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
bool PixteeRoundBridge::register_course_resource(
    int id,const PackedByteArray& mapm,const PackedByteArray& spt,
    const PackedByteArray& descriptors,const PackedByteArray& selectors,
    const PackedByteArray& maps) {
    if(!round_ || id<0 || id>99) return false;
    try {
        sensigolf::ClassicCourseResources course(
            native_bytes(mapm),native_bytes(spt),native_bytes(descriptors),
            native_bytes(selectors),native_bytes(maps));
        round_->register_course(static_cast<std::uint8_t>(id),course);
        last_error_="";
        return true;
    } catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
#define PIXTEE_WRAP_BOOL(METHOD,CODE) \
bool PixteeRoundBridge::METHOD { \
    if(!round_) {last_error_="Native round is not initialized";return false;} \
    try {CODE;last_error_="";return true;} \
    catch(const std::exception& e) {last_error_=String(e.what());return false;} \
}
PIXTEE_WRAP_BOOL(load_next_hole(),round_->load_next_hole())
PIXTEE_WRAP_BOOL(set_aim_raw(int h),round_->set_aim_raw(static_cast<std::uint16_t>(h)))
PIXTEE_WRAP_BOOL(nudge_aim_raw(int d),round_->nudge_aim_raw(d))
PIXTEE_WRAP_BOOL(set_club_index(int i),round_->set_club(static_cast<std::uint16_t>(i)))
PIXTEE_WRAP_BOOL(cycle_club(int d),round_->cycle_club(d))
PIXTEE_WRAP_BOOL(meter_click_raw(int s),round_->meter_click_raw(static_cast<std::uint16_t>(s)))
PIXTEE_WRAP_BOOL(acknowledge_hazard(),round_->acknowledge_hazard())
PIXTEE_WRAP_BOOL(acknowledge_special_green(),round_->acknowledge_special_green())
PIXTEE_WRAP_BOOL(commit_scored_hole(),round_->commit_scored_hole())
#undef PIXTEE_WRAP_BOOL
void PixteeRoundBridge::cancel_meter() {if(round_)round_->cancel_meter();}
void PixteeRoundBridge::step_tick() {
    if(!round_) return;
    try {round_->step_tick();last_error_="";}
    catch(const std::exception& e) {last_error_=String(e.what());}
}
bool PixteeRoundBridge::step_ticks(int count) {
    if(!round_ || count<0 || count>4096) return false;
    try {
        for(int i=0;i<count;++i) round_->step_tick();
        last_error_="";
        return true;
    } catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
String PixteeRoundBridge::save_stable_replay() {
    if(!round_) return "";
    try {last_error_="";return String(round_->serialize_stable_replay().c_str());}
    catch(const std::exception& e) {last_error_=String(e.what());return "";}
}
bool PixteeRoundBridge::restore_replay(const String& payload) {
    if(!round_) return false;
    try {
        const auto utf8=payload.utf8();
        const std::string encoded(utf8.get_data(),static_cast<std::size_t>(utf8.length()));
        round_->restore_replay(encoded);
        last_error_="";
        return true;
    } catch(const std::exception& e) {last_error_=String(e.what());return false;}
}
Dictionary PixteeRoundBridge::requested_course() const {
    Dictionary d;
    if(!round_) return d;
    try {
        const auto next=round_->resource_request();
        if(next) {
            d["index"]=static_cast<int>(next->round_index);
            d["resource_id"]=static_cast<int>(next->resource_id);
            d["par"]=static_cast<int>(next->par);
            d["registered"]=round_->course_registered(next->resource_id);
        }
    } catch(const std::exception&) {}
    return d;
}
Dictionary PixteeRoundBridge::snapshot() const {
    Dictionary d;
    d["initialized"]=round_!=nullptr;
    d["error"]=last_error_;
    if(!round_) return d;
    try {
        const auto& score=round_->round_state();
        d["round_complete"]=score.round_complete;
        d["holes_completed"]=static_cast<int>(score.holes_completed);
        d["current_hole_index"]=static_cast<int>(score.current_hole_index);
        d["total_strokes"]=static_cast<int>(score.total_strokes);
        d["cumulative_par"]=static_cast<int>(score.cumulative_par);
        d["relative_to_par"]=static_cast<int>(score.relative_to_par);
        d["saved_shots"]=static_cast<int64_t>(round_->saved_shots());
        d["replay_supported"]=round_->replay_supported();
        d["meter_stage"]=static_cast<int>(round_->meter_stage());
        d["can_shoot"]=round_->controls_enabled();
        d["club_index"]=static_cast<int>(round_->club_index());
        d["aim_raw"]=static_cast<int>(round_->aim_raw());
        d["has_hole"]=round_->has_hole();
        d["physics_authority"]="Recovered Sensible Golf ClassicGameSession";
        if(round_->has_hole()) {
            const auto& h=round_->hole();
            const auto& ball=h.ball_state();
            const bool active=h.phase()==sensigolf::HoleSessionPhase::ShotActive;
            d["hole_phase"]=static_cast<int>(h.phase());
            d["strokes"]=static_cast<int>(h.strokes());
            d["ball_x_raw"]=static_cast<int64_t>(active?ball.x_raw:h.ball_x_raw());
            d["ball_y_raw"]=static_cast<int64_t>(active?ball.y_raw:h.ball_y_raw());
            d["ball_z_raw"]=static_cast<int64_t>(ball.z_raw);
            d["ball_velocity_x"]=static_cast<int64_t>(ball.vx_raw);
            d["ball_velocity_y"]=static_cast<int64_t>(ball.vy_raw);
            d["ball_velocity_z"]=static_cast<int64_t>(ball.vz_raw);
            d["shot_tick"]=static_cast<int64_t>(ball.tick);
            d["green_mode"]=h.green_mode();
            d["distance_to_hole"]=static_cast<int64_t>(h.distance_to_hole());
            d["hazard_pause_ticks"]=static_cast<int>(h.hazard_pause_remaining());
            d["special_green_pause_ticks"]=static_cast<int>(h.special_green_pause_remaining());
            const auto surface=h.current_surface();
            d["terrain_descriptor"]=static_cast<int>(surface.descriptor_index);
            d["surface_code"]=static_cast<int>(surface.landing_code);
            d["lie_profile"]=static_cast<int>(surface.profile_slot);
            d["slope_direction"]=static_cast<int>(surface.slope_direction);
            d["slope_magnitude"]=static_cast<int>(surface.slope_magnitude);
        }
    } catch(const std::exception& e) {d["snapshot_error"]=String(e.what());}
    return d;
}
} // namespace godot
