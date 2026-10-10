#include "sensigolf/classic_mobile_controls.hpp"

#include <cstdint>
#include <stdexcept>

namespace sensigolf {

void ClassicMobileControls::sync_hole(const ClassicHoleSession& hole) {
    hole_ready_ = hole.phase() == HoleSessionPhase::ReadyForShot;
    // No controls may leak into a live shot, hazard or scored-hole state.
    if (!hole_ready_) {
        cancel_meter();
    }
}

void ClassicMobileControls::reset() noexcept {
    hole_ready_ = false;
    aim_ = 0;
    club_ = 0;
    cancel_meter();
}

bool ClassicMobileControls::enabled() const noexcept {
    return hole_ready_;
}

ClassicMeterStage ClassicMobileControls::stage() const noexcept {
    return stage_;
}

std::uint16_t ClassicMobileControls::aim_raw() const noexcept {
    return aim_;
}

std::uint16_t ClassicMobileControls::club_index() const noexcept {
    return club_;
}

void ClassicMobileControls::ensure_configurable() const {
    if (!hole_ready_ || stage_ != ClassicMeterStage::Idle) {
        throw std::logic_error(
            "aim/club changes require an idle, playable original hole");
    }
}

void ClassicMobileControls::set_aim_raw(std::uint16_t aim) {
    ensure_configurable();
    if (aim >= 4096u) {
        throw std::invalid_argument("classic aim is exactly 0..4095");
    }
    aim_ = aim;
}

void ClassicMobileControls::nudge_aim(std::int32_t delta_units) {
    ensure_configurable();
    const auto value = static_cast<std::int64_t>(aim_) + delta_units;
    aim_ = static_cast<std::uint16_t>((value % 4096 + 4096) % 4096);
}

void ClassicMobileControls::set_club(std::uint16_t club_index) {
    ensure_configurable();
    if (club_index > 12u) {
        throw std::invalid_argument("classic club index is exactly 0..12");
    }
    club_ = club_index;
}

void ClassicMobileControls::cycle_club(int delta) {
    ensure_configurable();
    const auto value = static_cast<std::int64_t>(club_) + delta;
    club_ = static_cast<std::uint16_t>((value % 13 + 13) % 13);
}

void ClassicMobileControls::meter_click(
    std::uint16_t displayed_raw_reading) {
    if (!hole_ready_) {
        throw std::logic_error("cannot click Welly meter outside playable hole");
    }
    if (displayed_raw_reading > 105u) {
        throw std::invalid_argument("original raw meter must be 0..105");
    }
    switch (stage_) {
    case ClassicMeterStage::Idle:
        stage_ = ClassicMeterStage::Power;
        return;
    case ClassicMeterStage::Power:
        captured_power_ = displayed_raw_reading;
        stage_ = ClassicMeterStage::Accuracy;
        return;
    case ClassicMeterStage::Accuracy:
        pending_ = ClassicShotRequest{
            aim_, captured_power_, displayed_raw_reading, club_};
        stage_ = ClassicMeterStage::ShotQueued;
        return;
    case ClassicMeterStage::ShotQueued:
        throw std::logic_error("shot already queued for the original engine");
    }
}

void ClassicMobileControls::cancel_meter() noexcept {
    stage_ = ClassicMeterStage::Idle;
    captured_power_ = 0;
    pending_.reset();
}

std::optional<ClassicShotRequest>
ClassicMobileControls::queued_shot() const noexcept {
    return pending_;
}

bool ClassicMobileControls::dispatch_to(ClassicHoleSession& hole) {
    if (!pending_) {
        return false;
    }
    if (!hole_ready_ || hole.phase() != HoleSessionPhase::ReadyForShot) {
        throw std::logic_error("cannot dispatch while hole is not ready");
    }
    hole.begin_shot(*pending_);
    // Only a successful engine submission consumes the input transaction.
    hole_ready_ = false;
    cancel_meter();
    return true;
}

} // namespace sensigolf
