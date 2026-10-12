#include "original_cpu_candidate.hpp"
#include <stdexcept>
namespace pixtee {
int original_cpu_candidate_index(
    const std::vector<OriginalComputerCandidate>& candidates) {
    if(candidates.empty() || candidates.size()>110)
        throw std::invalid_argument("original candidate count outside fixed CPU search window");
    // Original 0x40C78B..0x40C7C4: seed from FIRST non-negative
    // primary-status candidate. On subsequent iterations the original
    // compares the previous best AX metric with each candidate's +0xC
    // signed WORD; strictly smaller values win. Secondary status must
    // also be nonnegative for replacements. Ties preserve the first.
    int chosen=-1;
    std::int16_t best_metric=0;
    for(std::size_t i=0;i<candidates.size();++i) {
        const auto& c=candidates[i];
        if(chosen<0) {
            if(c.status_primary<0) continue;
            chosen=static_cast<int>(i);
            best_metric=c.candidate_metric;
            continue;
        }
        if(c.status_primary<0 || c.status_secondary<0) continue;
        if(c.candidate_metric<best_metric) {
            best_metric=c.candidate_metric;
            chosen=static_cast<int>(i);
        }
    }
    return chosen;
}
}