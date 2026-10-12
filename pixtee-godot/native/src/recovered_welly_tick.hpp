#pragma once
#include <cstdint>
namespace pixtee {
// Isolated source-derived Windows v1.014 WHACK/Welly meter subroutines.
// Covers raw per-invocation counter, saved power, directional angle and
// putter-specific step settings only. Input debounce, display pixels and
// scheduler-to-wallclock mapping require separate original-game traces.
struct OriginalWellyTickState {
    std::uint16_t stage=1; // original 0x41d58c; 1 rising, 2 falling, 0 ended
    std::uint16_t counter=0; // 0x41d592
    std::uint16_t angle=0; // 0x41d594
    std::uint32_t accumulator=0x00010000u; // 0x41d596 in controlled test
    std::uint16_t capture_flag=0; // 0x41d59a
    std::uint16_t saved_angle=0; // 0x41d59c
    std::uint16_t captured_power=0; // 0x41d59e
    std::uint16_t accuracy_counter=0; // 0x41d5a0
    std::uint16_t step_setting=5; // 0x41d5a2
};
class OriginalWellyTick {
public:
    void reset() noexcept;
    // Authentic outer original 0x407620..0x407686 start handler. Source
    // stage2 becomes native isolated rising stage1; start accumulator 8
    // differs from the 0x00010000 isolated oracle precondition.
    void start_from_original_ui_entry() noexcept;
    void step(bool button_pressed, std::uint16_t original_club_index) noexcept;
    const OriginalWellyTickState& state() const noexcept {return state_;}
private:
    void complete() noexcept;
    OriginalWellyTickState state_{};
};
} // namespace pixtee
