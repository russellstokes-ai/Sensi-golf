#pragma once

#include <cstdint>
#include <optional>

#include "sensigolf/classic_hole_session.hpp"

namespace sensigolf {

// Host-independent input state machine for Android touch/gamepad controls.
// NO original physics, meter timing, or shot outcomes are implemented here.
// UI provides the raw 0..105 meter reading at the two capture clicks.
enum class ClassicMeterStage : std::uint8_t {
    Idle = 0,
    Power,
    Accuracy,
    ShotQueued,
};

class ClassicMobileControls final {
public:
    void sync_hole(const ClassicHoleSession& hole);
    void reset() noexcept;

    bool enabled() const noexcept;
    ClassicMeterStage stage() const noexcept;

    std::uint16_t aim_raw() const noexcept;
    std::uint16_t club_index() const noexcept;
    void set_aim_raw(std::uint16_t aim);
    void nudge_aim(std::int32_t delta_units);
    void set_club(std::uint16_t club_index);
    void cycle_club(int delta);

    // Click 1 starts power; click 2 captures power; click 3 captures accuracy.
    // Readings must come from the recovered original meter scheduler.
    // No invented power/accuracy curve is applied by this adapter.
    void meter_click(std::uint16_t displayed_raw_reading);
    void cancel_meter() noexcept;

    std::optional<ClassicShotRequest> queued_shot() const noexcept;
    // Dispatch is atomic from the input UI's perspective: a failed engine
    // begin_shot never marks the UI shot as successfully sent.
    bool dispatch_to(ClassicHoleSession& hole);

private:
    void ensure_configurable() const;

    bool hole_ready_ = false;
    std::uint16_t aim_ = 0;
    std::uint16_t club_ = 0;
    std::uint16_t captured_power_ = 0;
    ClassicMeterStage stage_ = ClassicMeterStage::Idle;
    std::optional<ClassicShotRequest> pending_{};
};

} // namespace sensigolf
