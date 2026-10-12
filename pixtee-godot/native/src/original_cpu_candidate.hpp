#pragma once
#include <cstdint>
#include <vector>
namespace pixtee {
struct OriginalComputerCandidate {
    std::int16_t status_primary=0;
    std::int16_t status_secondary=0;
    std::int16_t candidate_metric=0;
};
// Isolated original 0x40C78B..0x40C7C4 from v1.014 CPU planner.
// Returns -1 if the original instruction path branches to retry 0x40C883.
// Does NOT claim a complete AI or original course decision policy.
int original_cpu_candidate_index(
    const std::vector<OriginalComputerCandidate>& candidates);
}
