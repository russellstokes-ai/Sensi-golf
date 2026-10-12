#pragma once
#include <cstdint>

namespace pixtee {
// Original Windows v1.014 CPU candidate simulator 0x40CB40..0x40CB6C
// AFTER the map and landing-code lookup. Unlike 0x40CB6C gravity,
// this gate can early-exit from code9, and routes club12 differently.
// A full trial requires preceding 0x409535 and 0x40B965 machine state.
enum class OriginalCpuPreflightRoute : std::uint8_t {
    Gravity = 0,           // 0x40CB6C
    ContinueMotion = 1,   // 0x40CBCD
    ReturnZero = 2,       // 0x40CC71
    ReturnMinusOne = 3    // 0x40CC49
};
struct OriginalCpuPreflightState {
    std::uint16_t terrain_code = 0; // source BX from original descriptor
    std::uint16_t club = 0;        // source WORD [ESI+0x28]
    std::int32_t vertical_force = 0; // source [EDI+8]
    std::int32_t horizontal_force = 0; // source [EDI+0xC]
    std::int32_t height = 0;       // source [EDI+0x14]
};
OriginalCpuPreflightRoute original_cpu_trial_preflight(
    const OriginalCpuPreflightState& state) noexcept;
} // namespace pixtee
