#include "pixtee_round_runtime.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <limits>
#include "sensigolf/recovered_flight.hpp"

namespace pixtee {
using namespace sensigolf;

PixteeRoundRuntime::PixteeRoundRuntime(
    std::array<std::uint8_t,18> order,
    std::array<std::uint8_t,100> pars,
    std::uint16_t seed0, std::uint16_t seed1)
    : pars_(pars), plan_(order,pars), initial_seed_{seed0,seed1},
      game_(std::make_unique<ClassicGameSession>(plan_,0,initial_seed_)) {}

void PixteeRoundRuntime::register_course(
    std::uint8_t id,const ClassicCourseResources& course) {
    if(id>=100) throw std::invalid_argument("original-style resource ID must be 0..99");
    courses_.insert_or_assign(id,course);
}
bool PixteeRoundRuntime::course_registered(std::uint8_t id) const noexcept {
    return courses_.find(id)!=courses_.end();
}
void PixteeRoundRuntime::load_next_hole() {
    const auto next=game_->resource_request();
    if(!next) throw std::logic_error("original-rule round already completed");
    if(game_->has_active_hole()) throw std::logic_error("hole already active");
    const auto found=courses_.find(next->resource_id);
    if(found==courses_.end()) throw std::logic_error("authored course bundle not registered");
    game_->load_current_hole(next->resource_id,found->second);
    game_->active_hole().set_authored_wind(wind_);
    controls_.reset();
    controls_.sync_hole(game_->active_hole());
    pending_.reset();
    original_welly_active_=false;
    original_welly_first_press_=false;
    original_welly_rejected_=false;
    original_welly_.reset();
    touch_meter_={};
    record_command('L');
}
void PixteeRoundRuntime::set_authored_wind(std::int32_t direction,std::int32_t speed,bool enabled) {
    if(direction<0 || direction>4095 || speed<0 || speed>150)throw std::invalid_argument("wind parameters outside authored range");
    if(!has_hole() || round_state().holes_completed || touch_meter_.stage || original_welly_active_ || controls_.stage()!=sensigolf::ClassicMeterStage::Idle)
        throw std::logic_error("wind requires fresh ready round");
    sensigolf::AuthoredWindState value{enabled && speed>0,static_cast<std::uint16_t>(direction),static_cast<std::uint16_t>(enabled?speed:0),96};
    game_->active_hole().set_authored_wind(value);wind_=value;
    record_command('D',{direction,speed,enabled?1:0});
    if(value.enabled)replay_supported_=false;
}
void PixteeRoundRuntime::set_aim_raw(std::uint16_t raw) {
    if(original_welly_active_ || touch_meter_.stage!=0) throw std::logic_error("aim locked during original Welly");
    controls_.set_aim_raw(raw);record_command('A',{raw});
}
void PixteeRoundRuntime::nudge_aim_raw(std::int32_t delta) {
    if(original_welly_active_ || touch_meter_.stage!=0) throw std::logic_error("aim locked during original Welly");
    controls_.nudge_aim(delta);record_command('N',{delta});
}
void PixteeRoundRuntime::set_original_player_mode_5a(std::uint16_t source_mode) {
    if(!game_->has_active_hole())
        throw std::logic_error("original player mode requires a loaded hole");
    game_->active_hole().set_original_player_mode_5a(source_mode);record_command('M',{source_mode});
    // The replay journal does not yet store original player role. Human
    // role enables PRNG-dependent original near-cup lip collision; never
    // claim replay determinism until role and PRNG state are journalled.
    replay_supported_=false;
}

void PixteeRoundRuntime::set_flag_removed_for_putting(bool removed) {
    if(!game_->has_active_hole() || touch_meter_.stage || original_welly_active())
        throw std::logic_error("flag attendance requires ready input");
    game_->active_hole().set_flag_removed_for_putting(removed);
    replay_supported_=false;
    record_command('J',{removed?1:0});
}
void PixteeRoundRuntime::set_verified_flag_object(bool present) {
    if(!game_->has_active_hole())
        throw std::logic_error("no loaded hole for verified flag object");
    game_->active_hole().set_verified_flag_object(present);record_command('F',{present?1:0});
    if(present) {
        // The stable replay format does NOT serialize authored flag
        // object presence; don't claim deterministic restoration until
        // the flag manifest is recorded and independently replayed.
        replay_supported_=false;
    }
}

void PixteeRoundRuntime::set_authored_tree_objects(const std::vector<sensigolf::AuthoredTreeVolume>& trees) {
    if(!game_->has_active_hole())throw std::logic_error("no loaded hole for authored trees");
    game_->active_hole().set_authored_tree_objects(trees);
    std::vector<std::int64_t> args;
    for(const auto& t:trees) for(auto value:{static_cast<std::int32_t>(t.id),t.x,t.y,t.trunk_radius,t.trunk_height,t.canopy_radius,t.canopy_bottom,t.canopy_top})args.push_back(value);
    record_command('T',std::move(args));
    if(!trees.empty())replay_supported_=false;
}
void PixteeRoundRuntime::set_soft_ground_enabled(bool enabled) {
    if(!has_hole())throw std::logic_error("soft ground requires a loaded hole");
    game_->active_hole().set_soft_ground_enabled(enabled);record_command('Q',{enabled?1:0});
    if(enabled)replay_supported_=false; // Stable legacy journal lacks authoring flags; full native session records Q.
}
void PixteeRoundRuntime::set_authored_birds_enabled(bool enabled) {
    game_->active_hole().set_authored_birds_enabled(enabled);record_command('B',{enabled?1:0});
    replay_supported_=false;
}
void PixteeRoundRuntime::set_club(std::uint16_t index) {
    if(original_welly_active_ || touch_meter_.stage!=0) throw std::logic_error("club locked during original Welly");
    controls_.set_club(index);record_command('C',{index});
}
void PixteeRoundRuntime::cycle_club(int delta) {
    if(original_welly_active_ || touch_meter_.stage!=0) throw std::logic_error("club locked during original Welly");
    controls_.cycle_club(delta);record_command('Y',{delta});
}
void PixteeRoundRuntime::meter_click_raw(std::uint16_t sample) {
    if(!game_->has_active_hole()) throw std::logic_error("no active native hole");
    if(original_welly_active_ || touch_meter_.stage!=0) throw std::logic_error("cannot mix original meter with manual samples");
    controls_.meter_click(sample);record_command('I',{sample}); // Recovered atomic three-press sequence.
    if(controls_.stage()!=ClassicMeterStage::ShotQueued) return;
    const auto shot=*controls_.queued_shot();
    if(!controls_.dispatch_to(game_->active_hole()))
        throw std::runtime_error("original core rejected queued shot");
    if(game_->active_hole().phase()==HoleSessionPhase::ReadyForShot) {
        // Original v1.014 charged a stroke even with no flight. The replay
        // journal can preserve this completed atomic input and verify it by
        // executing the recovered core again at restoration.
        record_charged_no_flight(shot);
        return;
    }
    pending_=shot;
}

void PixteeRoundRuntime::start_touch_meter() {
    if(original_welly_active_ || touch_meter_.stage || !has_hole() ||
       !controls_.enabled() || controls_.stage()!=ClassicMeterStage::Idle)
        throw std::logic_error("touch meter requires an idle playable hole");
    const auto window=recovered::original_accuracy_window(club_index(),hole().current_surface().profile_slot);
    touch_meter_.start(window.lower,window.upper);record_command('U');
}
bool PixteeRoundRuntime::tick_touch_meter(bool pressed) {
    if(!touch_meter_.stage)throw std::logic_error("touch meter not started");
    const auto accuracy=touch_meter_.sample();
    const bool complete=touch_meter_.step(pressed);
    record_command('V',{pressed?1:0});
    if(!complete)return false;
    // Suppress nested journal entries: replay V dispatches exactly these same inputs.
    const bool recording=recording_;recording_=false;
    try {
        meter_click_raw(0);meter_click_raw(static_cast<std::uint16_t>(touch_meter_.power));
        meter_click_raw(static_cast<std::uint16_t>(accuracy));
    }catch(...) {recording_=recording;throw;}
    recording_=recording;return true;
}
void PixteeRoundRuntime::start_original_welly() {
    if(original_welly_active_ || touch_meter_.stage!=0 || !game_->has_active_hole() ||
       !controls_.enabled() || controls_.stage()!=ClassicMeterStage::Idle)
        throw std::logic_error("original Welly needs an idle playable hole");
    original_welly_.start_from_original_ui_entry();
    original_welly_active_=true;
    original_welly_first_press_=false;
    original_welly_rejected_=false;
    record_command('W');
}
bool PixteeRoundRuntime::tick_original_welly(bool pressed) {
    if(!original_welly_active_)
        throw std::logic_error("original Welly was not started");
    const auto before=original_welly_.state();
    original_welly_.step(pressed,controls_.club_index());record_command('P',{pressed?1:0});
    const auto& after=original_welly_.state();
    if(after.capture_flag) original_welly_first_press_=true;
    if(after.stage!=0) return false;
    original_welly_active_=false;
    // Exact original meter producer was tested against actual v1.014 x86.
    // No stroke is created by incomplete/timeout input. The full original
    // rejection/retry event sequence is not yet recovered; do not invent it.
    const bool second_press=pressed && before.stage==2 &&
                             before.capture_flag!=0;
    if(!original_welly_first_press_ || !second_press ||
       after.counter>105 || after.captured_power>105) {
        original_welly_rejected_=true;
        return false;
    }
    try {
        controls_.meter_click(0); // adapter's first press starts meter
        controls_.meter_click(after.captured_power);
        controls_.meter_click(after.counter);
        const auto request=*controls_.queued_shot();
        if(!controls_.dispatch_to(game_->active_hole()))
            throw std::logic_error("classic original shot rejected");
        if(game_->active_hole().phase()==HoleSessionPhase::ReadyForShot) {
            // Genuine original profile rejection: no flight but one paid
            // stroke. Save the input as a complete, replayable stroke rather
            // than inventing a free retry.
            original_welly_rejected_=true;
            record_charged_no_flight(request);
            return false;
        }
        pending_=request;
        return true;
    } catch(const std::out_of_range&) {
        // Programming error or an unrepresentable raw meter input, not
        // the source-verified profile rejection. Do not silently swallow.
        controls_.cancel_meter();
        original_welly_rejected_=true;
        throw;
    } catch(...) {
        controls_.cancel_meter();
        original_welly_rejected_=true;
        throw;
    }
}
bool PixteeRoundRuntime::original_welly_active() const noexcept {
    return original_welly_active_;
}
bool PixteeRoundRuntime::original_welly_rejected() const noexcept {
    return original_welly_rejected_;
}
const OriginalWellyTickState&
PixteeRoundRuntime::original_welly_state() const noexcept {
    return original_welly_.state();
}

void PixteeRoundRuntime::cancel_meter() noexcept {
    original_welly_active_=false;
    touch_meter_.stage=0;
    controls_.cancel_meter();record_command('K');
}

void PixteeRoundRuntime::record_charged_no_flight(
    const ClassicShotRequest& request) {
    const auto& h=game_->active_hole();
    if(h.phase()!=HoleSessionPhase::ReadyForShot || h.strokes()==0)
        throw std::logic_error("original paid no-flight attempt has no stable boundary");

    // An input taken at exactly zero hole distance needs the original next
    // logical scoring tick. Until that edge case has a stable boundary, keep
    // the stricter replay gate instead of saving a premature result.
    if(h.distance_to_hole()==0) {
        replay_supported_=false;
    } else if(replay_supported_) {
        try {
            journal_.append_after_shot(h,request);
        } catch(...) {
            // The live paid stroke cannot be rolled back after journal failure.
            replay_supported_=false;
            throw;
        }
    }
    pending_.reset();
    controls_.sync_hole(h);
}

void PixteeRoundRuntime::maybe_record_stable_shot() {
    if(!pending_ || !game_->has_active_hole()) return;
    const auto& h=game_->active_hole();
    const auto phase=h.phase();
    // A resting shot at zero recovered distance is not scored until the
    // next native logical tick; don't record a premature ReadyForShot.
    if(phase!=HoleSessionPhase::HoleScored &&
       !(phase==HoleSessionPhase::ReadyForShot && h.distance_to_hole()>0))
        return;
    if(replay_supported_)
        journal_.append_after_shot(h,*pending_);
    pending_.reset();
}
void PixteeRoundRuntime::step_tick() {
    if(!game_->has_active_hole()) return;
    game_->active_hole().step();record_command('S');
    controls_.sync_hole(game_->active_hole());
    maybe_record_stable_shot();
}
void PixteeRoundRuntime::acknowledge_hazard() {
    if(!game_->has_active_hole()) throw std::logic_error("no native hole");
    game_->active_hole().acknowledge_hazard_recovery();record_command('H');
    controls_.sync_hole(game_->active_hole());
    maybe_record_stable_shot();
}
void PixteeRoundRuntime::acknowledge_special_green() {
    if(!game_->has_active_hole()) throw std::logic_error("no native hole");
    game_->active_hole().acknowledge_special_green_stop();record_command('G');
    // Original code 9/10 post-pause continuation is not proven; do not
    // generate a superficially valid "exact replay" from an unsupported path.
    replay_supported_=false;
    controls_.sync_hole(game_->active_hole());
    maybe_record_stable_shot();
}
void PixteeRoundRuntime::commit_scored_hole() {
    if(!game_->has_active_hole()) throw std::logic_error("no native hole");
    maybe_record_stable_shot();
    if(pending_) throw std::logic_error("incomplete original shot cannot be committed");
    game_->commit_scored_hole();record_command('E'); // Original cumulative counters / turns.
    controls_.reset();
}
std::string PixteeRoundRuntime::serialize_stable_replay() const {
    if(pending_ || !replay_supported_)
        throw std::logic_error("cannot claim a verified stable replay of this shot");
    return journal_.serialize();
}
void PixteeRoundRuntime::restore_replay(const std::string& checkpoint) {
    const auto parsed=ClassicReplayJournal::parse(checkpoint);
    auto restored=std::make_unique<ClassicGameSession>(plan_,0,initial_seed_);
    parsed.replay_into(*restored,[this](std::uint8_t id) {
        auto it=courses_.find(id);
        if(it==courses_.end())
            throw std::logic_error("replay requires the same registered authored course");
        return it->second;
    });
    // Commit successful only after the entire reference replay has passed.
    game_=std::move(restored);
    journal_=parsed;
    replay_supported_=true;
    pending_.reset();
    controls_.reset();
    original_welly_active_=false;
    original_welly_first_press_=false;
    original_welly_rejected_=false;
    original_welly_.reset();
    touch_meter_={};
    if(game_->has_active_hole())
        controls_.sync_hole(game_->active_hole());
}
const ClassicRoundState& PixteeRoundRuntime::round_state() const noexcept {
    return game_->round_state();
}
std::optional<ClassicHolePlanEntry>
PixteeRoundRuntime::resource_request() const {return game_->resource_request();}
bool PixteeRoundRuntime::has_hole() const noexcept {
    return game_->has_active_hole();
}
const ClassicHoleSession& PixteeRoundRuntime::hole() const {
    return game_->active_hole();
}
std::optional<ClassicGreenRegion>
PixteeRoundRuntime::active_green_region() const {
    if(!game_->has_active_hole()) return std::nullopt;
    const auto metadata=game_->active_hole().hole_metadata();
    if(!metadata) return std::nullopt;
    const auto found=courses_.find(static_cast<std::uint8_t>(metadata->resource_id));
    if(found==courses_.end()) return std::nullopt;
    return found->second.green_region();
}

std::vector<NativeGreenArrowCell>
PixteeRoundRuntime::verified_native_green_arrows(
    std::uint16_t green_detail_stride) const {
    // This is a pure READ of original MAPS, never a new gameplay slope engine.
    // Godot must use returned field directly; it cannot infer it from artwork.
    if(green_detail_stride==0 || green_detail_stride>64)
        throw std::invalid_argument("green-detail sample spacing must be 1..64");
    if(!game_->has_active_hole()) return {};
    const auto metadata=game_->active_hole().hole_metadata();
    if(!metadata) return {};
    const auto found=courses_.find(static_cast<std::uint8_t>(metadata->resource_id));
    if(found==courses_.end()) return {};
    const auto& source=found->second;
    const auto origin=source.green_region();
    if(!origin || !source.has_green_map()) return {};
    const auto xmax=std::min<std::uint32_t>(
        0x100u,static_cast<std::uint32_t>(source.green_map_width())*16u);
    const auto ymax=std::min<std::uint32_t>(
        0xD0u,static_cast<std::uint32_t>(source.green_map_height())*8u);
    std::vector<NativeGreenArrowCell> arrows;
    for(std::uint32_t y=green_detail_stride/2u;y<ymax;
        y+=green_detail_stride) {
        for(std::uint32_t x=green_detail_stride/2u;x<xmax;
            x+=green_detail_stride) {
            // MAPS extends beyond the original scene admission rectangle.
            // Outside it the ball uses MAPM, so a MAPS arrow would advertise
            // a slope force that does not apply at that world position.
            if(!origin->contains_course(
                    static_cast<std::int16_t>(origin->origin_x+x/2u),
                    static_cast<std::int16_t>(origin->origin_y+y/2u)))continue;
            const auto surface=source.resolve_integer_position(
                static_cast<std::int16_t>(x),static_cast<std::int16_t>(y),true);
            if(surface.landing_code!=1u || surface.slope_magnitude==0u)
                continue;
            if(surface.slope_direction>4095u ||
               surface.slope_direction%256u!=0u ||
               surface.slope_magnitude>15u) {
                throw std::logic_error(
                    "authored green MAPS returned invalid original slope sectors");
            }
            // v1.014 originals use local coords at 2x course unit resolution:
            // x_world = first MAPM marker origin + MAPS detailed x / 2.
            const std::uint32_t world_x=origin->origin_x + x/2u;
            const std::uint32_t world_y=origin->origin_y + y/2u;
            if(world_x>=source.map_width()*16u ||
               world_y>=source.map_height()*8u)
                throw std::logic_error("original MAPS arrow outside course world");
            arrows.push_back({
                static_cast<std::int16_t>(world_x),
                static_cast<std::int16_t>(world_y),
                surface.slope_direction,surface.slope_magnitude,
                surface.landing_code
            });
            if(arrows.size()>2048u)
                throw std::logic_error(
                    "green arrow field exceeds bounded render limit");
        }
    }
    return arrows;
}
ClassicMeterStage PixteeRoundRuntime::meter_stage() const noexcept {
    return controls_.stage();
}
bool PixteeRoundRuntime::controls_enabled() const noexcept {
    return controls_.enabled();
}
std::uint16_t PixteeRoundRuntime::aim_raw() const noexcept {
    return controls_.aim_raw();
}
std::uint16_t PixteeRoundRuntime::club_index() const noexcept {
    return controls_.club_index();
}
std::size_t PixteeRoundRuntime::saved_shots() const noexcept {
    return journal_.frames().size();
}
bool PixteeRoundRuntime::replay_supported() const noexcept {
    return replay_supported_;
}
} // namespace pixtee
