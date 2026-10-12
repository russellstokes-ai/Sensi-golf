#pragma once
#include <array>
#include <cstdint>

namespace pixtee {
// Original Windows v1.014 candidate-evaluation scoring ONLY:
// 0x40C9FD..0x40CA1E. [record+0xA] chooses a penalty-table WORD;
// [record+0xC] receives (AX distance + penalty) modulo 2^16.
// Green mode exactly 1 bypasses penalty. This routine does NOT predict
// the trajectory, contact code, distance or penalty table itself.
constexpr std::size_t kOriginalCpuSafePenaltySlots=32;
using OriginalCpuPenaltyTable=
    std::array<std::uint16_t,kOriginalCpuSafePenaltySlots>;
std::uint16_t original_cpu_quality_score(
    std::uint16_t distance_word,
    std::uint16_t source_contact_code,
    std::uint16_t original_green_mode,
    const OriginalCpuPenaltyTable& source_penalties);
} // namespace pixtee
