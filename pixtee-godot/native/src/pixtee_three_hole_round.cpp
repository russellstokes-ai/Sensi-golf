#include "pixtee_three_hole_round.hpp"

#include <algorithm>
#include <stdexcept>

namespace pixtee {
namespace {
using Ids = std::array<std::uint8_t, PixteeThreeHoleRound::kPreviewHoles>;

void check_manifest(const Ids& ids, const Ids& pars) {
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (ids[i] == 0u || ids[i] >= 100u)
            throw std::invalid_argument("three-hole course IDs must be 1..99");
        if (pars[i] == 0u || pars[i] > 12u)
            throw std::invalid_argument("three-hole pars must be 1..12");
        for (std::size_t j = 0; j < i; ++j)
            if (ids[j] == ids[i])
                throw std::invalid_argument("three-hole preview requires three DISTINCT authored courses");
    }
}

std::array<std::uint8_t, 18> make_order(const Ids& ids, const Ids& pars) {
    check_manifest(ids, pars);
    // The original recovered engine strictly requires an 18-entry plan.
    // The padding is NEVER exposed by this preview wrapper and does not
    // mean the extra 15 holes exist as authored production course content.
    std::array<std::uint8_t, 18> order{};
    order.fill(ids[0]);
    std::copy(ids.begin(), ids.end(), order.begin());
    return order;
}

std::array<std::uint8_t, 100> make_pars(const Ids& ids, const Ids& pars) {
    check_manifest(ids, pars);
    std::array<std::uint8_t, 100> output{};
    for (std::size_t i = 0; i < ids.size(); ++i)
        output[ids[i]] = pars[i];
    return output;
}
}  // namespace

PixteeThreeHoleRound::PixteeThreeHoleRound(
    Ids ids, Ids pars, std::uint16_t seed0, std::uint16_t seed1)
    : ids_(ids), round_(make_order(ids, pars), make_pars(ids, pars),
                        seed0, seed1) {}

void PixteeThreeHoleRound::register_course(
    std::uint8_t id, const sensigolf::ClassicCourseResources& course) {
    if (std::find(ids_.begin(), ids_.end(), id) == ids_.end())
        throw std::invalid_argument("unlisted resource is not an approved preview hole");
    if (round_.has_hole() || holes_completed() != 0u)
        throw std::logic_error("preview course content cannot change after play starts");
    round_.register_course(id, course);
}

std::optional<sensigolf::ClassicHolePlanEntry>
PixteeThreeHoleRound::next_hole() const {
    if (complete()) return std::nullopt;
    const auto requested = round_.resource_request();
    if (!requested || requested->round_index >= ids_.size() ||
        requested->resource_id != ids_[requested->round_index])
        throw std::logic_error("native original round's requested course differs from preview manifest");
    return requested;
}

void PixteeThreeHoleRound::load_next_hole() {
    if (complete())
        throw std::logic_error("three-hole preview is already complete");
    if (round_.has_hole())
        throw std::logic_error("cannot replace an active preview hole");
    (void)next_hole();  // checked original request
    round_.load_next_hole();
}

void PixteeThreeHoleRound::commit_scored_hole() {
    if (complete())
        throw std::logic_error("three-hole preview is already complete");
    if (!round_.has_hole())
        throw std::logic_error("no active hole to score");
    round_.commit_scored_hole(); // delegates to original/native source counters
    if (holes_completed() > kPreviewHoles)
        throw std::logic_error("preview scored beyond three holes");
}

void PixteeThreeHoleRound::set_aim_raw(std::uint16_t h) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.set_aim_raw(h);
}
void PixteeThreeHoleRound::nudge_aim_raw(std::int32_t n) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.nudge_aim_raw(n);
}
void PixteeThreeHoleRound::set_club(std::uint16_t i) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.set_club(i);
}
void PixteeThreeHoleRound::cycle_club(int delta) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.cycle_club(delta);
}
void PixteeThreeHoleRound::meter_click_raw(std::uint16_t sample) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.meter_click_raw(sample);
}
void PixteeThreeHoleRound::start_original_welly() {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.start_original_welly();
}
bool PixteeThreeHoleRound::tick_original_welly(bool pressed) {
    if (complete()) throw std::logic_error("three-hole preview completed");
    return round_.tick_original_welly(pressed);
}
void PixteeThreeHoleRound::step_tick() {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.step_tick();
}
void PixteeThreeHoleRound::acknowledge_hazard() {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.acknowledge_hazard();
}
void PixteeThreeHoleRound::acknowledge_special_green() {
    if (complete()) throw std::logic_error("three-hole preview completed");
    round_.acknowledge_special_green();
}
const sensigolf::ClassicHoleSession& PixteeThreeHoleRound::hole() const {
    return round_.hole();
}
const sensigolf::ClassicRoundState&
PixteeThreeHoleRound::underlying_original_round() const noexcept {
    return round_.round_state();
}
bool PixteeThreeHoleRound::complete() const noexcept {
    return round_.round_state().holes_completed >= kPreviewHoles;
}
std::uint16_t PixteeThreeHoleRound::holes_completed() const noexcept {
    return round_.round_state().holes_completed;
}
std::string PixteeThreeHoleRound::serialize_stable_replay() const {
    return round_.serialize_stable_replay();
}
}  // namespace pixtee
