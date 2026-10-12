#pragma once
#include <cstdint>
#include <vector>
#include "original_cpu_candidate.hpp"
#include "original_cpu_trial_force.hpp"
#include "original_cpu_simulated_contact.hpp"
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
// The exact original 0x40CCA4 trial initial velocity with one original
// candidate's heading. Caller supplies authoritative original player
// scale/base fields and captured power. This does NOT simulate/score a
// trajectory or choose the club. Never substitute a guessed evaluator.
struct OriginalCpuTrial {
    std::uint16_t heading=0;
    OriginalCpuTrialForce force{};
};
OriginalCpuTrial original_cpu_candidate_trial(
    const OriginalCpuScanPlan& plan,std::size_t candidate_index,
    OriginalCpuTrialForceInput original_player_state);

// Composes two independently source-x86 verified original CPU trial
// stages when the caller can certify original branch 0x40CB6C was reached
// (non-putter, no earlier special terrain/map termination). Uncertified
// early branches cannot be executed using this isolated contact helper.
struct OriginalCpuTrialPrecontact {
    OriginalCpuTrial trial{};
    OriginalCpuContactResult update{};
};
OriginalCpuTrialPrecontact original_cpu_trial_precontact(
    const OriginalCpuScanPlan& plan,std::size_t candidate_index,
    OriginalCpuTrialForceInput source_forces,
    std::int32_t source_height,std::int32_t source_hole_distance,
    bool original_airborne_branch_verified);

} // namespace pixtee
