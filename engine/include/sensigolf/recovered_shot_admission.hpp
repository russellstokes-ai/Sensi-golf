#pragma once
#include <cstdint>

namespace sensigolf::recovered {

// Strict source-scoped original Windows v1.014 player dispatch, from
// 0x40A2B0 up to the first external continuation. The original state
// fields are preserved by numeric names until their FULL semantics are
// verified. This does not run the preceding game-tick helpers.
enum class OriginalShotAdmissionRoute : std::uint8_t {
    PlayerBit15Wait = 0,  // source 0x40A7E0
    PlayerBit2Set = 1,    // source 0x40AACF
    PlayerBit1Clear = 2,  // source 0x40AA78 without CPU
    ActiveTerrainTick = 3,// source 0x40A2F7
    CpuTurnDelegate = 4,  // source 0x40C630 callback, then 0x40AA78
};
struct OriginalShotAdmissionState {
    std::uint16_t player_flags = 0; // [ESI + 0]
    std::uint16_t player_mode_5a = 0;// [ESI + 0x5A]
    std::uint16_t player_mode_5c = 0;// [ESI + 0x5C]
};
struct OriginalShotAdmissionResult {
    OriginalShotAdmissionRoute route =
        OriginalShotAdmissionRoute::PlayerBit1Clear;
    OriginalShotAdmissionState state{};
    bool cpu_delegate_requested = false;
};
// Source x86 preserves unsigned JB comparison of mode_5a < 4 and
// exact equality with mode_5c == 2 (not a threshold comparison).
OriginalShotAdmissionResult original_shot_tick_admission(
    OriginalShotAdmissionState state) noexcept;

} // namespace sensigolf::recovered
