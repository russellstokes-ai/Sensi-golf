#pragma once
#include <cstdint>

namespace pixtee {
// Original v1.014 x86 computer shot-error adjustment 0x40C7DD..0x40C84F.
// Random samples are supplied by the original game's PRNG caller; this
// reproduces ONLY the deterministic effect of those returned samples.
struct OriginalCpuShotVariation {
    std::uint16_t captured_power=0;
    std::uint16_t difficulty_variation=0;
    std::int32_t swing_adjuster=0;
    std::uint8_t random_calls=0;
};
OriginalCpuShotVariation original_cpu_shot_variation(
    bool bypass_error, std::uint16_t captured_power,
    std::uint16_t difficulty_variation,
    std::uint16_t random_100,
    std::uint16_t random_power,
    std::uint16_t random_direction) noexcept;
}
