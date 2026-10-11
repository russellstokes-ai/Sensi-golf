#include "original_cpu_candidate.hpp"
#include <stdexcept>
namespace pixtee {
int original_cpu_candidate_index(
    const std::vector<OriginalComputerCandidate>& candidates) {
    if(candidates.empty() || candidates.size()>110)
        throw std::invalid_argument("original candidate count outside fixed CPU search window");
    // Source x86 scans status_primary until it first becomes nonnegative.
    // The following apparent 'rank comparison' at 0x40C7B1 compares AX
    // against the very same record word, so ties never replace candidate.
    // Reproducing this observed behaviour is essential; no invented sort.
    for(std::size_t i=0;i<candidates.size();++i)
        if(candidates[i].status_primary>=0)
            return static_cast<int>(i);
    return -1;
}
}
