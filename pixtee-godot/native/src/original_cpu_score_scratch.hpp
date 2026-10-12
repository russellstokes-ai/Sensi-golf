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
} // namespace pixtee
