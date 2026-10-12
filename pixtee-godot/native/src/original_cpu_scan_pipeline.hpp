#pragma once
#include <cstdint>
#include <vector>
#include "original_cpu_candidate.hpp"
#include "original_cpu_difficulty.hpp"
#include "original_cpu_heading_sweep.hpp"
#include "original_cpu_shot_variation.hpp"

namespace pixtee {
// Composes separately source-authenticated original candidate *schedule*
// and candidate ranking, without inventing how the original game evaluates
// each shot or how it chooses a club/power.
struct OriginalCpuScanPlan {
    OriginalComputerSearchParameters difficulty{};
    OriginalCpuHeadingSweep sweep{};
    std::uint16_t mode=4;
    std::uint8_t course_band=0;
};
OriginalCpuScanPlan original_cpu_plan_scan(
    std::uint16_t source_mode,std::uint8_t course_band,
    std::uint16_t ball_heading,bool special_flag);

struct OriginalCpuRankedScan {
    int original_candidate_index=-1;
    std::uint16_t original_candidate_heading=0;
    bool requires_original_retry=false;
};
// The caller must actually compute the original per-candidate metric.
// No synthetic metrics or guessed ranking/power are supplied here.
OriginalCpuRankedScan original_cpu_rank_scanned_candidates(
    const OriginalCpuScanPlan& plan,
    const std::vector<OriginalComputerCandidate>& source_evaluated);

struct OriginalCpuSampledSwing {
    OriginalCpuRankedScan ranked{};
    OriginalCpuShotVariation variation{};
};
// Original PRNG output samples supplied explicitly and used only when a
// separately verified source swing power and difficulty variance is known.
OriginalCpuSampledSwing original_cpu_resolve_sampled_swing(
    const OriginalCpuRankedScan& ranked,
    bool bypass_error,std::uint16_t captured_power,
    std::uint16_t source_variation,
    std::uint16_t random_100,std::uint16_t random_power,
    std::uint16_t random_direction);
} // namespace pixtee
