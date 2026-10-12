#pragma once
#include <cstdint>
namespace pixtee {
// Exact original v1.014 x86 0x40CCA4..0x40CCC1, used by CPU
// trajectory trial 0x40C93E as well as original playing AI.
// A computed initial trial force is NOT yet a complete trial flight.
struct OriginalCpuTrialForceInput {
    std::int32_t player_shot_scale=0; // source [ESI+0x32]
    std::int32_t vertical_base=0;     // source [ESI+0x2A]
    std::int32_t horizontal_base=0;   // source [ESI+0x2E]
    std::uint16_t captured_power=0;  // source [0x41D59E]
};
struct OriginalCpuTrialForce {
    std::int32_t vertical_force=0;    // [EDI+0x8]
    std::int32_t horizontal_force=0;  // [EDI+0xC]
};
OriginalCpuTrialForce original_cpu_trial_force(
    OriginalCpuTrialForceInput input) noexcept;
} // namespace pixtee
