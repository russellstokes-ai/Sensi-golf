#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace pixtee {
// Source Windows 1.014 CPU evaluator 0x40C93E..0x40C968.
// A record is precisely 14 bytes. This is only the scoring scratch
// INITIALIZATION, not a recovered shot quality metric or AI policy.
constexpr std::size_t kOriginalCpuScoreRecordBytes=14u;
constexpr std::size_t kOriginalCpuScoreSafeRecords=110u;
struct OriginalCpuScoreInitialization {
    std::size_t cleared_records=0;
    std::uint32_t reset_swing_adjuster=0;
};
// Source [0x41FA0F] and [0x41FAD3] are inputs. Reject loops that
// would exceed the verified bounded mobile scratch memory, rather
// than copy the original's unchecked REP STOSB into unrelated memory.
OriginalCpuScoreInitialization original_cpu_clear_scoring_records(
    std::uint16_t original_span,std::uint16_t original_step,
    std::vector<std::uint8_t>& scoring_bytes);
// Source 0x40CA5A..0x40CA8E/CAC4 first-valid-candidate gate.
// Two signed status WORDs in each original 14-byte record must be >=0.
// Negative records advance EBX/EDX and signed 16-bit scan CX by stride.
// This does NOT score a shot or choose a club.
struct OriginalCpuScoreFirstEligible {
    bool found=false;
    std::size_t original_index=0;
    std::uint16_t scanned_cx=0;
    std::uint16_t scanned_bx=0;
    std::uint16_t scanned_dx=0;
};
OriginalCpuScoreFirstEligible original_cpu_first_eligible_score(
    std::uint16_t original_span,std::uint16_t original_step,
    const std::vector<std::uint8_t>& scoring_bytes);
} // namespace pixtee
