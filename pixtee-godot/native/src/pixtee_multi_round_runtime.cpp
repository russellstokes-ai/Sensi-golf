#include "pixtee_multi_round_runtime.hpp"
#include <stdexcept>
namespace pixtee {
PixteeMultiRoundRuntime::PixteeMultiRoundRuntime(
    std::uint8_t n,std::array<std::uint8_t,18> order,
    std::array<std::uint8_t,100> pars,
    std::array<sensigolf::recovered::OriginalPrng16,4> seeds) {
    if(n==0 || n>4)
        throw std::invalid_argument("original game permits 1..4 players");
    players_.reserve(n);
    for(std::size_t i=0;i<n;++i) {
        players_.push_back(std::make_unique<PixteeRoundRuntime>(
            order,pars,seeds[i].seed0,seeds[i].seed1));
    }
}
void PixteeMultiRoundRuntime::register_course(
    std::uint8_t id,const sensigolf::ClassicCourseResources& course) {
    for(auto& p:players_)p->register_course(id,course);
}
void PixteeMultiRoundRuntime::load_next_hole_for_everyone() {
    if(turn_.has_pending_turn())
        throw std::logic_error("cannot load holes during turn handoff");
    for(const auto& p:players_)
        if(p->has_hole())
            throw std::logic_error("each previous hole must be committed first");
    // Caller has explicitly provided the next course to every runtime.
    for(auto& p:players_)p->load_next_hole();
    current_owner_.reset();
}
void PixteeMultiRoundRuntime::propose_original_turn(
    const OriginalTurnRequest& req) {
    if(req.ordinary.count!=players_.size())
        throw std::invalid_argument(
            "original player-choice count differs from loaded participants");
    for(const auto& p:players_)
        if(!p->has_hole())
            throw std::logic_error("all participants need an active hole");
    if(current_owner_) {
        const auto& h=players_.at(*current_owner_)->hole();
        if(h.phase()==sensigolf::HoleSessionPhase::ShotActive)
            throw std::logic_error(
                "cannot transfer original turn during unfinished ball flight");
    }
    turn_.select_next(req);
}
std::uint8_t PixteeMultiRoundRuntime::acknowledge_original_turn() {
    const auto value=turn_.pending_turn();
    if(value.active_player>=players_.size())
        throw std::logic_error("original turn choice outside current match");
    const auto out=turn_.acknowledge_owner_switch();
    current_owner_=out.active_player;
    return *current_owner_;
}
std::uint8_t PixteeMultiRoundRuntime::active_owner() const {
    if(!current_owner_ || turn_.has_pending_turn())
        throw std::logic_error("player turn owner has not been acknowledged");
    return *current_owner_;
}
PixteeRoundRuntime& PixteeMultiRoundRuntime::active_round(){
    return *players_.at(active_owner());
}
const PixteeRoundRuntime& PixteeMultiRoundRuntime::player_round(
    std::size_t i) const{return *players_.at(i);}
PixteeRoundRuntime& PixteeMultiRoundRuntime::player_round(
    std::size_t i){return *players_.at(i);}
void PixteeMultiRoundRuntime::reset_owner() noexcept {
    current_owner_.reset();
    turn_.reset();
}
} // namespace pixtee
