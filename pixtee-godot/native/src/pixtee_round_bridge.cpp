#include "pixtee_round_bridge.hpp"
#include "sensigolf/recovered_flight.hpp"
#include "original_next_player.hpp"
#include "original_cpu_difficulty.hpp"
#include "original_cpu_search_grid.hpp"
#include "original_cpu_heading_sweep.hpp"
#include "original_cpu_shot_variation.hpp"
#include "original_turn_score_adjust.hpp"
#include "original_competition_turn.hpp"
#include "original_cpu_candidate.hpp"
#include "original_cpu_trial_force.hpp"
#include "original_cpu_scan_pipeline.hpp"
#include "original_cpu_quality_score.hpp"
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
    ClassDB::bind_method(
        D_METHOD("original_cpu_reference_penalties"),
        &PixteeRoundBridge::original_cpu_reference_penalties);
    ClassDB::bind_method(
        D_METHOD("original_cpu_quality_score","distance",
                 "original_contact_code","green_mode","source_penalties"),
        &PixteeRoundBridge::original_cpu_quality_score);
    ClassDB::bind_method(
        D_METHOD("original_cpu_scored_record_choice",
                 "mode","course_band","initial_heading","span","step",
                 "source_records","genuine_evaluations"),
        &PixteeRoundBridge::original_cpu_scored_record_choice);
    ClassDB::bind_method(
        D_METHOD("original_cpu_candidate_choice","primary_status",
                 "secondary_status","candidate_metrics"),
        &PixteeRoundBridge::original_cpu_candidate_choice);
    ClassDB::bind_method(
        D_METHOD("original_competition_turn_choice",
                 "mode","active_side","ball0_distance","ball1_distance",
                 "side0_flag","side1_flag","slot_bytes","previous_slots"),
        &PixteeRoundBridge::original_competition_turn_choice);
    ClassDB::bind_method(
        D_METHOD("original_turn_score_adjust","player_count",
                 "strokes","totals","completed","ball_distances"),
        &PixteeRoundBridge::original_turn_score_adjust);
    ClassDB::bind_method(
        D_METHOD("original_cpu_trial_force",
                 "player_scale","vertical_base","horizontal_base","captured_power"),
        &PixteeRoundBridge::original_cpu_trial_force);
    ClassDB::bind_method(
        D_METHOD("original_cpu_shot_variation",
                 "bypass","power","variation",
                 "random100","random_power","random_direction"),
        &PixteeRoundBridge::original_cpu_shot_variation);
    ClassDB::bind_method(
        D_METHOD("original_cpu_heading_sweep","original_mode","course_band","initial_heading"),
        &PixteeRoundBridge::original_cpu_heading_sweep);
    ClassDB::bind_method(
        D_METHOD("original_cpu_scan_grid","original_mode","course_band"),
        &PixteeRoundBridge::original_cpu_scan_grid);
    ClassDB::bind_method(
        D_METHOD("original_cpu_search_parameters","original_mode","special_flag"),
        &PixteeRoundBridge::original_cpu_search_parameters);
    ClassDB::bind_method(
        D_METHOD("original_next_player_choice","player_count",
                 "forced_holder","priority","eliminated","identities"),
        &PixteeRoundBridge::original_next_player_choice);
    ClassDB::bind_method(D_METHOD("begin_round","order","pars","seed0","seed1"),
                         &PixteeRoundBridge::begin_round);
    ClassDB::bind_method(D_METHOD("register_course_resource","resource_id","mapm","spt","descriptors","selectors","maps"),
                         &PixteeRoundBridge::register_course_resource);
    ClassDB::bind_method(D_METHOD("load_next_hole"),&PixteeRoundBridge::load_next_hole);
    ClassDB::bind_method(D_METHOD("set_aim_raw","heading"),&PixteeRoundBridge::set_aim_raw);
    ClassDB::bind_method(D_METHOD("nudge_aim_raw","delta"),&PixteeRoundBridge::nudge_aim_raw);
    ClassDB::bind_method(D_METHOD("set_club_index","club"),&PixteeRoundBridge::set_club_index);
    ClassDB::bind_method(D_METHOD("cycle_club","delta"),&PixteeRoundBridge::cycle_club);
    ClassDB::bind_method(D_METHOD("set_verified_flag_object","present"),
                         &PixteeRoundBridge::set_verified_flag_object);
    ClassDB::bind_method(D_METHOD("set_flag_removed_for_putting","removed"),&PixteeRoundBridge::set_flag_removed_for_putting);
    ClassDB::bind_method(D_METHOD("set_authored_wind","direction","speed_mph_x10","enabled"),&PixteeRoundBridge::set_authored_wind);
    ClassDB::bind_method(D_METHOD("save_native_session"),&PixteeRoundBridge::save_native_session);
    ClassDB::bind_method(D_METHOD("restore_native_session","payload"),&PixteeRoundBridge::restore_native_session);
    ClassDB::bind_method(D_METHOD("set_soft_ground_enabled","enabled"),&PixteeRoundBridge::set_soft_ground_enabled);
    ClassDB::bind_method(D_METHOD("set_authored_birds_enabled","enabled"),&PixteeRoundBridge::set_authored_birds_enabled);
    ClassDB::bind_method(D_METHOD("set_original_player_mode_5a","mode"),
                         &PixteeRoundBridge::set_original_player_mode_5a);
    ClassDB::bind_method(D_METHOD("meter_click_raw","sample"),&PixteeRoundBridge::meter_click_raw);
    ClassDB::bind_method(D_METHOD("cancel_meter"),&PixteeRoundBridge::cancel_meter);
    ClassDB::bind_method(D_METHOD("start_touch_meter"),&PixteeRoundBridge::start_touch_meter);
    ClassDB::bind_method(D_METHOD("tick_touch_meter","pressed"),&PixteeRoundBridge::tick_touch_meter);
    ClassDB::bind_method(D_METHOD("start_original_welly"),&PixteeRoundBridge::start_original_welly);
    ClassDB::bind_method(D_METHOD("tick_original_welly","pressed"),&PixteeRoundBridge::tick_original_welly);
    ClassDB::bind_method(D_METHOD("acknowledge_hazard"),&PixteeRoundBridge::acknowledge_hazard);
    ClassDB::bind_method(D_METHOD("acknowledge_special_green"),&PixteeRoundBridge::acknowledge_special_green);
    ClassDB::bind_method(D_METHOD("commit_scored_hole"),&PixteeRoundBridge::commit_scored_hole);
    ClassDB::bind_method(D_METHOD("step_tick"),&PixteeRoundBridge::step_tick);
    ClassDB::bind_method(D_METHOD("drain_presentation_events"),&PixteeRoundBridge::drain_presentation_events);
    ClassDB::bind_method(D_METHOD("step_ticks","count"),&PixteeRoundBridge::step_ticks);
    ClassDB::bind_method(D_METHOD("save_stable_replay"),&PixteeRoundBridge::save_stable_replay);
    ClassDB::bind_method(D_METHOD("restore_replay","payload"),&PixteeRoundBridge::restore_replay);
    ClassDB::bind_method(D_METHOD("snapshot"),&PixteeRoundBridge::snapshot);
    ClassDB::bind_method(D_METHOD("set_authored_tree_objects","raw_volumes"),&PixteeRoundBridge::set_authored_tree_objects);
    ClassDB::bind_method(D_METHOD("verified_native_green_arrow_cells","green_detail_stride"),&PixteeRoundBridge::verified_native_green_arrow_cells);
    ClassDB::bind_method(D_METHOD("requested_course"),&PixteeRoundBridge::requested_course);
    ClassDB::bind_method(D_METHOD("last_error"),&PixteeRoundBridge::last_error);
}
int PixteeRoundBridge::original_cpu_candidate_choice(
    const PackedInt32Array& primary,
    const PackedInt32Array& secondary,
    const PackedInt32Array& metric) {
    if(primary.size()<1 || primary.size()>110 ||
       primary.size()!=secondary.size() || primary.size()!=metric.size()) {
        last_error_="original CPU candidate arrays must share length 1..110";
        return -1;
    }
    try {
        std::vector<pixtee::OriginalComputerCandidate> candidates;
        candidates.reserve(static_cast<std::size_t>(primary.size()));
        for(int i=0;i<primary.size();++i) {
            for(auto value: {primary[i],secondary[i],metric[i]}) {
                if(value < -32768 || value > 32767)
                    throw std::invalid_argument("original candidate word outside signed 16-bit range");
            }
            candidates.push_back({
                static_cast<std::int16_t>(primary[i]),
                static_cast<std::int16_t>(secondary[i]),
                static_cast<std::int16_t>(metric[i])});
        }
        const int choice=pixtee::original_cpu_candidate_index(candidates);
        last_error_="";
        return choice;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        return -1;
    }
}
Dictionary PixteeRoundBridge::original_competition_turn_choice(
    int mode, int active, int dist0, int dist1,
    int f0, int f1,
    const PackedInt32Array& slots,
    const PackedInt32Array& previous) {
    Dictionary d;
    if(mode<0 || mode>255 || active<0 || active>65535 ||
       dist0<0 || dist1<0 ||
       f0<0 || f0>1 || f1<0 || f1>65535 ||
       slots.size()<4 || previous.size()<2) {
        last_error_="invalid original mode4/6 source state";
        d["verified"]=false;
        return d;
    }
    pixtee::OriginalCompetitionTurnInput in{};
    in.mode=static_cast<std::uint8_t>(mode);
    in.active_side=static_cast<std::uint16_t>(active);
    in.ball_distance={static_cast<std::uint32_t>(dist0),
                      static_cast<std::uint32_t>(dist1)};
    in.side0_flag=static_cast<std::uint16_t>(f0);
    in.side1_flag=static_cast<std::uint16_t>(f1);
    for(int i=0;i<4;++i) {
        if(slots[i]<0 || slots[i]>3) {
            last_error_="source original game requires player slot 0..3";
            d["verified"]=false;
            return d;
        }
        in.slot_bytes[i]=static_cast<std::uint8_t>(slots[i]);
    }
    for(int i=0;i<2;++i) {
        if(previous[i]<0 || previous[i]>65535) {
            last_error_="original previous slot must be unsigned WORD";
            d["verified"]=false;
            return d;
        }
        in.previous_slot[i]=static_cast<std::uint16_t>(previous[i]);
    }
    try {
        const auto out=pixtee::original_competition_turn_select(in);
        d["verified"]=true;
        d["selected_slot"]=out.selected_slot;
        d["source_slot"]=out.source_slot;
        d["ball_array"]=out.ball_array;
        d["active_side"]=static_cast<int>(out.active_side);
        PackedInt32Array slots_after;
        for(int i=0;i<2;++i)slots_after.append(static_cast<int>(out.previous_slot[i]));
        d["previous_slots"]=slots_after;
        d["all_competition_rules_recovered"]=false;
        last_error_="";
        return d;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        d["verified"]=false;
        return d;
    }
}
Dictionary PixteeRoundBridge::original_turn_score_adjust(
    int count, const PackedInt32Array& strokes,
    const PackedInt32Array& totals,
    const PackedInt32Array& completed,
    const PackedInt32Array& distances) {
    Dictionary d;
    if(count<1 || count>4 ||
       strokes.size()<count || totals.size()<count ||
       completed.size()<count || distances.size()<count) {
        last_error_="original multiplayer mode1 requires 1..4 valid player score arrays";
        d["verified"]=false;
        return d;
    }
    pixtee::OriginalTurnScores raw{};
    raw.count=static_cast<std::uint8_t>(count);
    for(int i=0;i<count;++i){
        for(const auto val : {strokes[i],totals[i],completed[i]}) {
            if(val<0 || val>65535) {
                last_error_="original multiplayer score counters require word range 0..65535";
                d["verified"]=false;
                return d;
            }
        }
        if(distances[i]<0) {
            last_error_="original multiplayer distance requires nonnegative word range";
            d["verified"]=false;
            return d;
        }
        raw.strokes[i]=static_cast<std::uint16_t>(strokes[i]);
        raw.total_strokes[i]=static_cast<std::uint16_t>(totals[i]);
        raw.completed_marker[i]=static_cast<std::uint16_t>(completed[i]);
        raw.ball_distance[i]=static_cast<std::uint32_t>(distances[i]);
    }
    const auto result=pixtee::original_turn_score_adjust(raw);
    PackedInt32Array out_strokes;
    PackedInt32Array out_total;
    for(int i=0;i<count;++i) {
        out_strokes.append(static_cast<int>(result.result.strokes[i]));
        out_total.append(static_cast<int>(result.result.total_strokes[i]));
    }
    d["verified"]=true;
    d["corrected_index"]=result.corrected_index;
    d["strokes"]=out_strokes;
    d["totals"]=out_total;
    d["entire_multiplayer_mode_implemented"]=false;
    last_error_="";
    return d;
}
Dictionary PixteeRoundBridge::original_cpu_trial_force(
    int player_scale,int vertical_base,int horizontal_base,int captured_power) {
    Dictionary d;
    if(captured_power<0 || captured_power>65535) {
        last_error_="original CPU trial captured power must fit source word";
        d["verified"]=false;
        return d;
    }
    const auto output=pixtee::original_cpu_trial_force({
        static_cast<std::int32_t>(player_scale),
        static_cast<std::int32_t>(vertical_base),
        static_cast<std::int32_t>(horizontal_base),
        static_cast<std::uint16_t>(captured_power)});
    d["verified"]=true;
    d["vertical_force"]=static_cast<int64_t>(output.vertical_force);
    d["horizontal_force"]=static_cast<int64_t>(output.horizontal_force);
    d["complete_cpu_shot_planner"]=false;
    last_error_="";
    return d;
}
PackedInt32Array PixteeRoundBridge::original_cpu_reference_penalties() const {
    PackedInt32Array out;
    for(const auto value:pixtee::original_cpu_reference_penalties())
        out.push_back(static_cast<int>(value));
    return out;
}
Dictionary PixteeRoundBridge::original_cpu_quality_score(
    int distance,int code,int green,const PackedInt32Array& penalties) {
    Dictionary result;
    result["verified"]=false;
    result["complete_cpu_shot_planner"]=false;
    if(distance<0 || distance>65535 || code<0 || code>=32 ||
       green<0 || green>65535 ||
       penalties.size()!=static_cast<int>(pixtee::kOriginalCpuSafePenaltySlots)) {
        last_error_="original CPU score needs 16-bit distance/green and bounded source contact/table";
        return result;
    }
    try {
        pixtee::OriginalCpuPenaltyTable source_table{};
        for(int i=0;i<penalties.size();++i) {
            if(penalties[i]<0 || penalties[i]>65535)
                throw std::invalid_argument(
                    "original CPU source penalty table WORD outside 0..65535");
            source_table[static_cast<std::size_t>(i)]=
                static_cast<std::uint16_t>(penalties[i]);
        }
        const auto raw=pixtee::original_cpu_quality_score(
            static_cast<std::uint16_t>(distance),
            static_cast<std::uint16_t>(code),
            static_cast<std::uint16_t>(green),source_table);
        result["verified"]=true;
        result["score_word"]=static_cast<int>(raw);
        result["requires_original_trajectory_evaluator"]=true;
        result["requires_original_penalty_table"]=true;
        last_error_="";
    }catch(const std::exception& e) {
        last_error_=String(e.what());
    }
    return result;
}
Dictionary PixteeRoundBridge::original_cpu_scored_record_choice(
    int mode,int band,int initial_heading,int span,int step,
    const PackedByteArray& scored,bool original_evaluations) {
    Dictionary result;
    result["verified"]=false;
    result["complete_cpu_shot_planner"]=false;
    if(mode<4 || mode>9 || band<0 || band>3 ||
       initial_heading<0 || initial_heading>4095 ||
       span<=0 || span>32767 || step<=0 || step>32767) {
        last_error_="original CPU signed score record inputs outside source range";
        return result;
    }
    try {
        const auto plan=pixtee::original_cpu_plan_scan(
            static_cast<std::uint16_t>(mode),static_cast<std::uint8_t>(band),
            static_cast<std::uint16_t>(initial_heading),false);
        const auto winner=pixtee::original_cpu_choose_scored_scan(
            plan,static_cast<std::uint16_t>(span),
            static_cast<std::uint16_t>(step),
            native_bytes(scored),original_evaluations);
        result["verified"]=true;
        result["found"]=winner.found;
        result["original_index"]=winner.found ?
            static_cast<int>(winner.index) : -1;
        result["heading"]=winner.found ?
            static_cast<int>(winner.heading) : -1;
        result["original_score_word"]=static_cast<int>(winner.score_word);
        result["original_evaluator_supplied"]=original_evaluations;
        last_error_="";
    }catch(const std::exception& e) {
        last_error_=String(e.what());
    }
    return result;
}
Dictionary PixteeRoundBridge::original_cpu_shot_variation(
    bool bypass, int power, int variation,
    int random100, int random_power, int random_direction) {
    Dictionary d;
    for(const auto v : {power,variation,random100,random_power,random_direction}) {
        if(v<0 || v>65535) {
            last_error_="source CPU error inputs require raw original 16-bit values";
            d["verified"]=false;
            return d;
        }
    }
    const auto shot=pixtee::original_cpu_shot_variation(
        bypass,static_cast<std::uint16_t>(power),
        static_cast<std::uint16_t>(variation),
        static_cast<std::uint16_t>(random100),
        static_cast<std::uint16_t>(random_power),
        static_cast<std::uint16_t>(random_direction));
    d["verified"]=true;
    d["captured_power"]=static_cast<int>(shot.captured_power);
    d["difficulty_variation"]=static_cast<int>(shot.difficulty_variation);
    d["swing_adjuster"]=static_cast<int>(shot.swing_adjuster);
    d["random_calls"]=static_cast<int>(shot.random_calls);
    d["original_rng_implemented"]=false; // random INPUTS provided; no synthetic RNG
    d["complete_cpu_shot_planner"]=false;
    last_error_="";
    return d;
}
Dictionary PixteeRoundBridge::original_cpu_heading_sweep(
    int mode,int band,int initial_heading) {
    Dictionary result;
    if(mode<4 || mode>9 || band<0 || band>3 || initial_heading<0 || initial_heading>4095) {
        last_error_="original CPU heading sweep requires mode 4..9, band 0..3, heading 0..4095";
        result["verified"]=false;
        return result;
    }
    try {
        const auto source=pixtee::original_cpu_heading_sweep_from_original_table(
            static_cast<std::uint16_t>(mode),
            static_cast<std::uint8_t>(band),
            static_cast<std::uint16_t>(initial_heading));
        PackedInt32Array headings;
        for(const auto h:source.candidate_headings)
            headings.append(static_cast<int>(h));
        result["verified"]=true;
        result["headings"]=headings;
        result["final_ball_heading"]=static_cast<int>(source.final_ball_heading);
        result["final_player_heading"]=static_cast<int>(source.final_player_heading);
        result["final_scan_counter"]=static_cast<int>(source.final_scan_counter);
        result["candidate_quality_evaluation_recovered"]=false;
        last_error_="";
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        result["verified"]=false;
    }
    return result;
}
Dictionary PixteeRoundBridge::original_cpu_scan_grid(int mode,int band) {
    Dictionary result;
    if(mode<4 || mode>9 || band<0 || band>3) {
        last_error_="original CPU scan grid requires source-proven mode4..9 and band0..3";
        result["verified"]=false;
        return result;
    }
    try {
        const auto source=pixtee::original_cpu_scan_grid_from_original_table(
            static_cast<std::uint16_t>(mode),static_cast<std::uint8_t>(band));
        result["verified"]=true;
        result["candidate_count"]=static_cast<int>(source.table_word_0);
        result["first_angular_extent"]=static_cast<int>(source.table_word_1);
        result["second_angular_extent"]=static_cast<int>(source.table_word_2);
        result["source_fourth_word"]=static_cast<int>(source.table_word_3);
        result["spacing"]=static_cast<int>(source.spacing);
        result["initial_remainder"]=static_cast<int>(source.initial_remainder);
        result["full_computer_opponent_recovered"]=false;
        last_error_="";
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        result["verified"]=false;
    }
    return result;
}
Dictionary PixteeRoundBridge::original_cpu_search_parameters(
    int mode, bool special_flag) {
    Dictionary result;
    if(mode<4 || mode>15) {
        last_error_="original CPU player mode must be in source-tested range 4..15";
        result["verified"]=false;
        return result;
    }
    try {
        const auto raw=pixtee::original_cpu_search_parameters(
            static_cast<std::uint16_t>(mode),special_flag);
        result["verified"]=true;
        result["candidate_stride"]=static_cast<int>(raw.candidate_stride);
        result["variation_budget"]=static_cast<int>(raw.variation_budget);
        result["search_budget"]=static_cast<int>(raw.search_budget);
        result["whole_ai_recovered"]=false; // NEVER overclaim
        last_error_="";
        return result;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        result["verified"]=false;
        return result;
    }
}
int PixteeRoundBridge::original_next_player_choice(
    int count, int forced, const PackedInt32Array& priority,
    const PackedInt32Array& eliminated, const PackedInt32Array& identities) {
    if(count<1 || count>4 || forced<0 || forced>count ||
       priority.size()<count || eliminated.size()<count ||
       identities.size()<count) {
        last_error_="original next-player selection needs 1..4 valid players";
        return -1;
    }
    try {
        pixtee::OriginalPlayerChoiceInput input{};
        input.count=static_cast<std::uint8_t>(count);
        input.forced_holder=static_cast<std::uint8_t>(forced);
        for(int i=0;i<count;++i) {
            if(priority[i]<0 || eliminated[i]<0 || eliminated[i]>65535 ||
               identities[i]<0 || identities[i]>255)
                throw std::invalid_argument("original player inputs outside register range");
            input.priority_metric[i]=static_cast<std::uint32_t>(priority[i]);
            input.eliminated[i]=static_cast<std::uint16_t>(eliminated[i]);
            input.identity[i]=static_cast<std::uint8_t>(identities[i]);
        }
        const auto selected=pixtee::original_next_player_index(input);
        last_error_="";
        return static_cast<int>(selected);
    } catch(const std::exception& e) {
        last_error_=String(e.what());
        return -1;
    }
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
PIXTEE_WRAP_BOOL(set_authored_birds_enabled(bool enabled),round_->set_authored_birds_enabled(enabled))
PIXTEE_WRAP_BOOL(load_next_hole(),round_->load_next_hole())
PIXTEE_WRAP_BOOL(set_soft_ground_enabled(bool enabled),round_->set_soft_ground_enabled(enabled))
PIXTEE_WRAP_BOOL(start_touch_meter(),round_->start_touch_meter())
PIXTEE_WRAP_BOOL(start_original_welly(),round_->start_original_welly())
PIXTEE_WRAP_BOOL(set_aim_raw(int h),round_->set_aim_raw(static_cast<std::uint16_t>(h)))
PIXTEE_WRAP_BOOL(nudge_aim_raw(int d),round_->nudge_aim_raw(d))
PIXTEE_WRAP_BOOL(set_club_index(int i),round_->set_club(static_cast<std::uint16_t>(i)))
PIXTEE_WRAP_BOOL(set_verified_flag_object(bool present),round_->set_verified_flag_object(present))
PIXTEE_WRAP_BOOL(set_flag_removed_for_putting(bool removed),round_->set_flag_removed_for_putting(removed))
PIXTEE_WRAP_BOOL(set_authored_wind(int direction,int speed_mph_x10,bool enabled),round_->set_authored_wind(direction,speed_mph_x10,enabled))
bool PixteeRoundBridge::set_original_player_mode_5a(int mode) {
    if(!round_) {last_error_="Native round is not initialized";return false;}
    if(mode<0 || mode>15) {
        last_error_="original player mode must be 0..15";
        return false;
    }
    try {
        round_->set_original_player_mode_5a(static_cast<std::uint16_t>(mode));
        last_error_="";
        return true;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
PIXTEE_WRAP_BOOL(cycle_club(int d),round_->cycle_club(d))
PIXTEE_WRAP_BOOL(meter_click_raw(int s),round_->meter_click_raw(static_cast<std::uint16_t>(s)))
PIXTEE_WRAP_BOOL(acknowledge_hazard(),round_->acknowledge_hazard())
PIXTEE_WRAP_BOOL(acknowledge_special_green(),round_->acknowledge_special_green())
PIXTEE_WRAP_BOOL(commit_scored_hole(),round_->commit_scored_hole())
#undef PIXTEE_WRAP_BOOL
bool PixteeRoundBridge::tick_touch_meter(bool pressed) {
    if(!round_)return false;
    try {const auto accepted=round_->tick_touch_meter(pressed);last_error_="";return accepted;}
    catch(const std::exception& e){last_error_=String(e.what());return false;}
}
bool PixteeRoundBridge::tick_original_welly(bool pressed) {
    if(!round_) {last_error_="Native round not initialized";return false;}
    try {
        const auto accepted=round_->tick_original_welly(pressed);
        last_error_="";
        return accepted;
    }catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
void PixteeRoundBridge::cancel_meter() {if(round_)round_->cancel_meter();}
bool PixteeRoundBridge::set_authored_tree_objects(const PackedInt32Array& raw) {
    if(!round_)return false;
    try {
        if(raw.size()%8 || raw.size()>128*8)throw std::invalid_argument("tree records require eight raw fields");
        std::vector<sensigolf::AuthoredTreeVolume> trees;
        for(int i=0;i<raw.size();i+=8) {
            if(raw[i]<1)throw std::invalid_argument("tree IDs must be positive");
            trees.push_back({static_cast<std::uint32_t>(raw[i]),raw[i+1],raw[i+2],raw[i+3],raw[i+4],raw[i+5],raw[i+6],raw[i+7]});
        }
        round_->set_authored_tree_objects(trees);
        last_error_="";return true;
    }catch(const std::exception& e){last_error_=String(e.what());return false;}
}
void PixteeRoundBridge::queue_presentation_event(const char* type,double x,double y,
                                                std::uint64_t tick) {
    Dictionary event;
    event["confirmed_native"]=true;
    event["event_id"]=String(std::to_string(++presentation_serial_).c_str());
    event["event_type"]=type;
    event["world_x"]=x; event["world_y"]=y;
    event["native_tick"]=static_cast<int64_t>(tick);
    presentation_events_.append(event);
}
Array PixteeRoundBridge::drain_presentation_events() {
    Array events=presentation_events_;
    presentation_events_=Array();
    return events;
}
void PixteeRoundBridge::step_tick() {
    if(!round_) return;
    try {
        if(!round_->has_hole()) {round_->step_tick();return;}
        const auto before_phase=round_->hole().phase();
        const auto before_tick=round_->hole().ball_state().tick;
        const auto before_vz=round_->hole().ball_state().vz_raw;
        const auto before_z=round_->hole().ball_state().z_raw;
        const bool before_cup=round_->hole().pending_cup_capture_event().has_value();
        const bool before_world_oob=round_->hole().pending_world_oob_event().has_value();
        round_->step_tick();
        const auto& hole=round_->hole();
        const auto& ball=hole.ball_state();
        auto world_position=[&](std::int64_t x,std::int64_t y,bool green) {
            double wx=static_cast<double>(x)/65536.,wy=static_cast<double>(y)/65536.;
            if(green) {
                const auto origin=round_->active_green_region();
                if(!origin) throw std::logic_error("native contact lacks green origin");
                wx=wx*.5+origin->origin_x;wy=wy*.5+origin->origin_y;
            }
            return std::pair<double,double>{wx,wy};
        };
        if(before_phase==sensigolf::HoleSessionPhase::ShotActive && ball.tick>before_tick) {
            const auto& contact=hole.presentation_contact();
            const auto p=world_position(contact.x_raw,contact.y_raw,contact.green_mode);
            if(before_tick==0) queue_presentation_event("club_contact",p.first,p.second,ball.tick);
            if(contact.impact || contact.hazard) {
                // Report actual native rebounds, never proximity or a visual arc.
                if(contact.impact && !contact.hazard && before_vz<0 && ball.vz_raw>0 && contact.landing_code!=5 && contact.landing_code!=6 && contact.landing_code!=7) {
                    queue_presentation_event("ball_bounce",p.first,p.second,ball.tick);
                    Dictionary rebound=presentation_events_[presentation_events_.size()-1];
                    rebound["rebound_vz_raw"]=static_cast<int64_t>(ball.vz_raw);
                }
                const char* type=nullptr;
                switch(contact.landing_code) {
                case 2:case 3:case 4:type="fairway_contact";break;
                case 5:case 6:type="rough_contact";break;
                case 7:type="bunker_contact";break;
                case 35:
                    // Native descriptor distinguishes water from mapped OOB;
                    // this authored map has descriptor 6 for water only.
                    if(hole.last_hazard_kind()==sensigolf::ClassicNamedHazard::Water)
                        type="water_contact";
                    break;
                }
                if(type) queue_presentation_event(type,p.first,p.second,ball.tick);
            }
            if(contact.bird_id) {
                queue_presentation_event("authored_bird_contact",p.first,p.second,ball.tick);
                Dictionary event=presentation_events_[presentation_events_.size()-1];
                event["bird_id"]=static_cast<int64_t>(contact.bird_id);
                event["bird_member"]=static_cast<int64_t>(contact.bird_member);
                event["world_z"]=static_cast<double>(before_z)/65536.;
            }
            if(contact.flag) queue_presentation_event("verified_flag_contact",p.first,p.second,ball.tick);
            if(contact.tree_id) {
                queue_presentation_event(contact.tree_id>=100 && contact.tree_id<104 ? "authored_board_contact" : "authored_tree_contact",p.first,p.second,ball.tick);
                Dictionary event=presentation_events_[presentation_events_.size()-1];
                event["tree_id"]=static_cast<int64_t>(contact.tree_id);
                event["contact_part"]=contact.canopy?"canopy":"trunk";
                event["collision_authority"]="owner-authorized C++ tree volumes";
            }
            if(contact.lip) queue_presentation_event("cup_lip",p.first,p.second,ball.tick);
        }
        const auto p=world_position(ball.x_raw,ball.y_raw,hole.green_mode());
        if(!before_world_oob && hole.pending_world_oob_event())
            queue_presentation_event("world_oob",p.first,p.second,ball.tick);
        if(!before_cup && hole.pending_cup_capture_event())
            queue_presentation_event("cup_capture",p.first,p.second,ball.tick);
        last_error_="";
    }
    catch(const std::exception& e) {last_error_=String(e.what());}
}
bool PixteeRoundBridge::step_ticks(int count) {
    if(!round_ || count<0 || count>4096) return false;
    try {
        for(int i=0;i<count;++i) {
            step_tick();
            if(!last_error_.is_empty()) return false;
        }
        last_error_="";
        return true;
    } catch(const std::exception& e) {
        last_error_=String(e.what());
        return false;
    }
}
String PixteeRoundBridge::save_native_session() {
    if(!round_)return "";
    try {last_error_="";return String(round_->serialize_native_session().c_str());}
    catch(const std::exception& e){last_error_=String(e.what());return "";}
}
bool PixteeRoundBridge::restore_native_session(const String& payload) {
    if(!round_)return false;
    try {round_->restore_native_session(payload.utf8().get_data());presentation_events_.clear();presentation_serial_=0;last_error_="";return true;}
    catch(const std::exception& e){last_error_=String(e.what());return false;}
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
Array PixteeRoundBridge::verified_native_green_arrow_cells(int stride) const {
    Array out;
    if(!round_ || stride<1 || stride>64) return out;
    try {
        for(const auto& sample:round_->verified_native_green_arrows(
                static_cast<std::uint16_t>(stride))) {
            Dictionary cell;
            cell["world_x"]=static_cast<int>(sample.world_x);
            cell["world_y"]=static_cast<int>(sample.world_y);
            cell["direction_raw"]=static_cast<int>(sample.slope_direction_raw);
            cell["magnitude"]=static_cast<int>(sample.slope_magnitude);
            cell["terrain_code"]=static_cast<int>(sample.landing_code);
            cell["verified_native"]=true;
            out.append(cell);
        }
    } catch(const std::exception&) {
        // Don't render invented arrow vectors after an invalid authored map.
        out.clear();
    }
    return out;
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
        d["can_shoot"]=round_->controls_enabled() && !round_->original_welly_active() && !round_->touch_meter().stage;
        const auto& touch=round_->touch_meter();
        d["touch_meter_active"]=touch.stage!=0;d["touch_meter_phase"]=touch.stage;
        d["touch_meter_position"]=touch.position;d["touch_meter_power"]=touch.power;
        d["touch_meter_red_half"]=touch.red_half;d["touch_meter_yellow"]=touch.yellow;
        d["original_welly_active"]=round_->original_welly_active();
        d["original_welly_rejected"]=round_->original_welly_rejected();
        const auto& meter=round_->original_welly_state();
        d["welly_phase"]=static_cast<int>(meter.stage);
        d["welly_power_captured"]=meter.capture_flag!=0;
        // The recovered falling callback decrements BEFORE accepting a press.
        d["welly_press_accuracy"]=meter.stage==2 ? static_cast<int>(meter.counter)-1 : 63;
        d["welly_counter"]=static_cast<int>(meter.counter);
        d["welly_saved_power"]=static_cast<int>(meter.captured_power);
        d["welly_angle"]=static_cast<int>(meter.angle);
        d["welly_saved_angle"]=static_cast<int>(meter.saved_angle);
        d["welly_accuracy_counter"]=static_cast<int>(meter.accuracy_counter);
        d["welly_step_setting"]=static_cast<int>(meter.step_setting);
        d["club_index"]=static_cast<int>(round_->club_index());
        d["aim_raw"]=static_cast<int>(round_->aim_raw());
        d["has_hole"]=round_->has_hole();
        d["wind_enabled"]=round_->authored_wind().enabled;
        d["wind_toward_raw"]=round_->authored_wind().toward_raw;
        d["wind_speed_mph_x10"]=round_->authored_wind().speed_mph_x10;
        d["wind_model_version"]=1;
        d["physics_authority"]="Recovered Sensible Golf ClassicGameSession";
        if(round_->has_hole()) {
            // Expose original MAPM green marker for 2x local-to-world visual
            // mapping. No changes to source native ball coordinates.
            if(const auto origin=round_->active_green_region()) {
                d["green_origin_x"]=static_cast<int>(origin->origin_x);
                d["green_origin_y"]=static_cast<int>(origin->origin_y);
            }
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
            d["flag_object_verified"]=h.flag_object_verified();
            d["original_player_mode_assigned"]=h.original_player_mode_5a().has_value();
            d["original_player_mode_5a"]=h.original_player_mode_5a().has_value()
                ? static_cast<int>(h.original_player_mode_5a().value()) : -1;
            d["near_cup_lip_activated_last_tick"]=h.original_lip_deflection_last_tick();
            d["distance_to_hole"]=static_cast<int64_t>(h.distance_to_hole());
            d["hazard_pause_ticks"]=static_cast<int>(h.hazard_pause_remaining());
            d["special_green_pause_ticks"]=static_cast<int>(h.special_green_pause_remaining());
            d["hazard_kind"]=static_cast<int>(h.last_hazard_kind());
            const auto world_oob=h.pending_world_oob_event();
            d["world_oob_event_pending"]=world_oob.has_value();
            if(world_oob) {
                d["world_oob_message_id"]=static_cast<int64_t>(world_oob->dword_420548);
                d["world_oob_display_code"]=static_cast<int64_t>(world_oob->dword_420544);
                d["world_oob_source_resource"]=static_cast<int64_t>(world_oob->source_esi);
            }
            const auto special_event=h.pending_special_green_event();
            d["special_green_event_pending"]=special_event.has_value();
            if(special_event) {
                d["special_green_message_id"]=static_cast<int64_t>(special_event->dword_420548);
                d["special_green_display_code"]=static_cast<int64_t>(special_event->dword_420544);
                d["special_green_source_resource"]=static_cast<int64_t>(special_event->source_esi);
            }
            const auto cup_event=h.pending_cup_capture_event();
            d["cup_event_pending"]=cup_event.has_value();
            if(cup_event) {
                d["cup_message_id"]=static_cast<int64_t>(cup_event->dword_420548);
                d["cup_display_code"]=static_cast<int64_t>(cup_event->dword_420544);
                d["cup_source_resource"]=static_cast<int64_t>(cup_event->source_esi);
            }
            // A genuine off-map ball has no valid MAPI descriptor. The
            // original executable reports AX=-1 instead of fetching a
            // nonexistent tile. Do not turn a valid OOB penalty into
            // a Godot snapshot_error by calling current_surface().
            const bool original_world_oob= h.last_hazard_kind()==
                sensigolf::ClassicNamedHazard::OutsideWorld &&
                h.phase()==sensigolf::HoleSessionPhase::HazardStopped;
            const auto bird=h.authored_bird();
            const auto flock=sensigolf::authored_bird_sample(h.authored_clock());
            d["bird_active"]=bird.active || (bird.id!=0 && flock.active);d["bird_id"]=static_cast<int64_t>(bird.id);
            d["bird_x_raw"]=bird.point.x;d["bird_y_raw"]=bird.point.y;d["bird_z_raw"]=bird.point.z;
            d["flag_removed_for_putting"]=h.flag_removed_for_putting();
            d["bird_hit_visible"]=bird.active && h.authored_bird_hit();
            d["bird_flock_active"]=bird.id!=0 && flock.active;
            d["bird_group_size"]=static_cast<int64_t>(sensigolf::authored_bird_group_size(bird.id));
            d["bird_hit_member"]=static_cast<int64_t>(bird.member);
            d["bird_flock_x_raw"]=flock.point.x;d["bird_flock_y_raw"]=flock.point.y;d["bird_flock_z_raw"]=flock.point.z;
            d["bird_hit"]=h.authored_bird_hit();d["bird_hit_age"]=static_cast<int64_t>(h.bird_hit_age());
            d["authored_clock"]=static_cast<int64_t>(h.authored_clock());
            d["terrain_available"]=!original_world_oob;
            if(!original_world_oob) {
            const auto surface=h.current_surface();
            d["terrain_descriptor"]=static_cast<int>(surface.descriptor_index);
            d["surface_code"]=static_cast<int>(surface.landing_code);
            d["lie_profile"]=static_cast<int>(surface.profile_slot);
            const auto window=sensigolf::recovered::original_accuracy_window(round_->club_index(),surface.profile_slot);
            d["welly_window_low"]=window.lower;
            d["welly_window_high"]=window.upper;
            d["welly_window_center"]=window.center;
            d["slope_direction"]=static_cast<int>(surface.slope_direction);
            d["slope_magnitude"]=static_cast<int>(surface.slope_magnitude);
            } // only if a valid original world MAPI position exists
        }
    } catch(const std::exception& e) {d["snapshot_error"]=String(e.what());}
    return d;
}
} // namespace godot
