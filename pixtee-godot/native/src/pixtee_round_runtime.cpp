#include "pixtee_round_runtime.hpp"
#include <stdexcept>
#include <utility>

namespace pixtee {
using namespace sensigolf;

PixteeRoundRuntime::PixteeRoundRuntime(
    std::array<std::uint8_t,18> order,
    std::array<std::uint8_t,100> pars,
    std::uint16_t seed0, std::uint16_t seed1)
    : plan_(order,pars), initial_seed_{seed0,seed1},
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
    controls_.reset();
    controls_.sync_hole(game_->active_hole());
    pending_.reset();
}
void PixteeRoundRuntime::set_aim_raw(std::uint16_t raw) {
    controls_.set_aim_raw(raw);
}
void PixteeRoundRuntime::nudge_aim_raw(std::int32_t delta) {
    controls_.nudge_aim(delta);
}
void PixteeRoundRuntime::set_club(std::uint16_t index) {
    controls_.set_club(index);
}
void PixteeRoundRuntime::cycle_club(int delta) {
    controls_.cycle_club(delta);
}
void PixteeRoundRuntime::meter_click_raw(std::uint16_t sample) {
    if(!game_->has_active_hole()) throw std::logic_error("no active native hole");
    controls_.meter_click(sample); // Recovered atomic three-press sequence.
    if(controls_.stage()!=ClassicMeterStage::ShotQueued) return;
    const auto shot=*controls_.queued_shot();
    if(!controls_.dispatch_to(game_->active_hole()))
        throw std::runtime_error("original core rejected queued shot");
    pending_=shot;
}
void PixteeRoundRuntime::cancel_meter() noexcept {controls_.cancel_meter();}

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
    game_->active_hole().step();
    controls_.sync_hole(game_->active_hole());
    maybe_record_stable_shot();
}
void PixteeRoundRuntime::acknowledge_hazard() {
    if(!game_->has_active_hole()) throw std::logic_error("no native hole");
    game_->active_hole().acknowledge_hazard_recovery();
    controls_.sync_hole(game_->active_hole());
    maybe_record_stable_shot();
}
void PixteeRoundRuntime::acknowledge_special_green() {
    if(!game_->has_active_hole()) throw std::logic_error("no native hole");
    game_->active_hole().acknowledge_special_green_stop();
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
    game_->commit_scored_hole(); // Original cumulative counters / turns.
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
