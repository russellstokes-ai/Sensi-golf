#include "sensigolf/classic_game_session.hpp"

#include <stdexcept>
#include <utility>

namespace sensigolf {

ClassicGameSession::ClassicGameSession(
    ClassicHolePlan plan,
    std::size_t player_slot,
    std::optional<recovered::OriginalPrng16> initial_prng)
    : plan_(std::move(plan)),
      player_slot_(player_slot),
      initial_prng_(initial_prng),
      prng_(initial_prng) {
    if (player_slot_ >= 4u) {
        throw std::invalid_argument(
            "classic player SPT slot outside recovered 0..3 range");
    }
}

const ClassicRoundState&
ClassicGameSession::round_state() const noexcept {
    return round_.state();
}

std::optional<ClassicHolePlanEntry>
ClassicGameSession::resource_request() const {
    return round_.current_hole_request(plan_);
}

bool ClassicGameSession::has_prng_state() const noexcept {
    return prng_.has_value();
}

recovered::OriginalPrng16& ClassicGameSession::prng_state() {
    if (!prng_) {
        throw std::logic_error(
            "classic game session has no captured original PRNG state");
    }
    return *prng_;
}

const recovered::OriginalPrng16& ClassicGameSession::prng_state() const {
    if (!prng_) {
        throw std::logic_error(
            "classic game session has no captured original PRNG state");
    }
    return *prng_;
}

bool ClassicGameSession::has_active_hole() const noexcept {
    return hole_ != nullptr;
}

ClassicHoleSession& ClassicGameSession::active_hole() {
    if (!hole_) {
        throw std::logic_error("classic game session has no loaded hole");
    }
    return *hole_;
}

const ClassicHoleSession& ClassicGameSession::active_hole() const {
    if (!hole_) {
        throw std::logic_error("classic game session has no loaded hole");
    }
    return *hole_;
}

void ClassicGameSession::load_current_hole(
    std::uint8_t resource_id,
    ClassicCourseResources course) {
    if (hole_) {
        throw std::logic_error(
            "classic game session already has an active hole");
    }

    const auto request = resource_request();
    if (!request) {
        throw std::logic_error(
            "classic round is complete; no further hole can be loaded");
    }
    if (resource_id != request->resource_id) {
        throw std::logic_error(
            "loaded classic resource does not match current original hole request");
    }

    course_ = std::make_unique<ClassicCourseResources>(
        std::move(course));
    try {
        hole_ = std::make_unique<ClassicHoleSession>(
            *course_,
            player_slot_,
            ClassicHoleMetadata{
                request->round_index,
                request->par,
                request->resource_id},
            prng_ ? &*prng_ : nullptr);
    } catch (...) {
        course_.reset();
        throw;
    }
}

void ClassicGameSession::commit_scored_hole() {
    if (!hole_) {
        throw std::logic_error(
            "classic game session has no active hole to commit");
    }

    // Round validation happens before releasing either resource object so a
    // rejected result leaves the active hole inspectable.
    round_.accept_scored_hole(*hole_, plan_);
    hole_.reset();
    course_.reset();
}

void ClassicGameSession::reset() {
    hole_.reset();
    course_.reset();
    round_.reset();
    prng_ = initial_prng_;
}

} // namespace sensigolf
