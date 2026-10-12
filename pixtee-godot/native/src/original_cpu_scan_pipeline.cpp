#include "original_cpu_scan_pipeline.hpp"
#include <stdexcept>
namespace pixtee {
OriginalCpuScanPlan original_cpu_plan_scan(
    std::uint16_t mode,std::uint8_t band,
    std::uint16_t heading,bool special) {
    // Original scan table is identified for modes 4..9, four bands.
    // The standalone difficulty helper also handles 10..15, but using
    // those unsupported combinations here would invent a scan table.
    OriginalCpuScanPlan out{};
    out.mode=mode;
    out.course_band=band;
    out.difficulty=original_cpu_search_parameters(mode,special);
    out.sweep=original_cpu_heading_sweep_from_original_table(
        mode,band,heading);
    return out;
}
OriginalCpuRankedScan original_cpu_rank_scanned_candidates(
    const OriginalCpuScanPlan& plan,
    const std::vector<OriginalComputerCandidate>& source_evaluated) {
    if(source_evaluated.size()!=plan.sweep.candidate_headings.size())
        throw std::invalid_argument(
            "original CPU source scan results must cover every generated heading");
    const int choice=original_cpu_candidate_index(source_evaluated);
    OriginalCpuRankedScan out{};
    out.original_candidate_index=choice;
    out.requires_original_retry=choice<0;
    if(choice>=0)
        out.original_candidate_heading=plan.sweep.candidate_headings.at(
            static_cast<std::size_t>(choice));
    return out;
}
OriginalCpuSampledSwing original_cpu_resolve_sampled_swing(
    const OriginalCpuRankedScan& ranked,
    bool bypass,std::uint16_t power,std::uint16_t variation,
    std::uint16_t r100,std::uint16_t rpower,std::uint16_t rdir) {
    if(ranked.requires_original_retry || ranked.original_candidate_index<0)
        throw std::logic_error(
            "original CPU retry route has no completed source shot");
    OriginalCpuSampledSwing out{};
    out.ranked=ranked;
    out.variation=original_cpu_shot_variation(
        bypass,power,variation,r100,rpower,rdir);
    return out;
}
OriginalCpuTrial original_cpu_candidate_trial(
    const OriginalCpuScanPlan& plan,std::size_t index,
    OriginalCpuTrialForceInput values) {
    if(index>=plan.sweep.candidate_headings.size())
        throw std::out_of_range("original CPU trial candidate index not present");
    return {plan.sweep.candidate_headings[index],
            original_cpu_trial_force(values)};
}

OriginalCpuTrialPrecontact original_cpu_trial_precontact(
    const OriginalCpuScanPlan& plan,std::size_t index,
    OriginalCpuTrialForceInput source,
    std::int32_t height,std::int32_t distance,
    bool original_airborne_branch_verified) {
    if(!original_airborne_branch_verified)
        throw std::logic_error(
            "original CPU 0x40CB6C must be observed after course/club checks");
    OriginalCpuTrialPrecontact out{};
    out.trial=original_cpu_candidate_trial(plan,index,source);
    out.update=original_cpu_simulation_contact({
        out.trial.force.vertical_force,
        out.trial.force.horizontal_force,height,distance});
    return out;
}

OriginalCpuTrialRoute original_cpu_trial_route_after_lookup(
    const OriginalCpuScanPlan& plan,std::size_t index,
    OriginalCpuTrialForceInput source,
    std::uint16_t landing,std::uint16_t club,
    std::int32_t height,std::int32_t hole_distance,
    bool source_map_verified) {
    if(!source_map_verified)
        throw std::logic_error(
          "CPU candidate cannot select original 0x40CB40 route before 0x409535 MAPI descriptor lookup");
    OriginalCpuTrialRoute out{};
    out.trial=original_cpu_candidate_trial(plan,index,source);
    out.route=original_cpu_trial_preflight({
        landing,club,out.trial.force.vertical_force,
        out.trial.force.horizontal_force,height});
    if(out.route==OriginalCpuPreflightRoute::Gravity) {
        out.gravity_contact=original_cpu_simulation_contact({
            out.trial.force.vertical_force,
            out.trial.force.horizontal_force,
            height,hole_distance});
    }
    return out;
}
OriginalCpuScoredChoice original_cpu_choose_scored_scan(
    const OriginalCpuScanPlan& plan,std::uint16_t span,
    std::uint16_t step,const std::vector<std::uint8_t>& bytes,
    bool source_verified) {
    if(!source_verified)
        throw std::logic_error(
            "CPU score candidates must come from original trajectory evaluation");
    // Reject original signed overflow or unbounded host parameters. The
    // original scans signed CX; actual source scan order must be a
    // one-to-one match with the 24-row source heading candidate table.
    if(span==0 || span>0x7fffu || step==0 || step>span)
        throw std::invalid_argument("CPU original signed score scan bounds invalid");
    const std::size_t expected=
        (static_cast<std::size_t>(span)+step-1u)/step;
    if(expected!=plan.sweep.candidate_headings.size() ||
       expected>kOriginalCpuScoreSafeRecords)
        throw std::invalid_argument(
            "original scoring record order does not match source heading sweep");
    if(bytes.size()<expected*kOriginalCpuScoreRecordBytes ||
       bytes.size()>kOriginalCpuScoreSafeRecords*kOriginalCpuScoreRecordBytes)
        throw std::out_of_range("original CPU 14-byte score scratch length invalid");

    const auto selected=original_cpu_select_best_scored_record(
        span,step,bytes);
    if(!selected.found)
        return {};
    if(selected.original_index>=expected ||
       selected.offset_bytes!=selected.original_index*kOriginalCpuScoreRecordBytes)
        throw std::logic_error("source CPU score record index outside verified heading order");
    return {true,selected.original_index,
            plan.sweep.candidate_headings[selected.original_index],
            selected.original_score_word};
}

void original_cpu_write_quality_record(
    std::vector<std::uint8_t>& records,std::size_t index,
    std::uint16_t distance,std::uint16_t contact,
    std::uint16_t green,const OriginalCpuPenaltyTable& penalties,
    bool genuine_evaluation) {
    if(!genuine_evaluation)
        throw std::logic_error(
            "CPU score record may not be created without completed source trajectory evaluation");
    if(index>=kOriginalCpuScoreSafeRecords ||
       records.size()>kOriginalCpuScoreSafeRecords*kOriginalCpuScoreRecordBytes)
        throw std::out_of_range(
            "original CPU bounded scratch record index invalid");
    const auto off=index*kOriginalCpuScoreRecordBytes;
    if(off>records.size() || records.size()-off<kOriginalCpuScoreRecordBytes)
        throw std::out_of_range(
            "original CPU scoring record has fewer than 14 bytes");
    // Validate/map the source contact code BEFORE any record mutation.
    const auto score=original_cpu_quality_score(
        distance,contact,green,penalties);
    records[off+12u]=static_cast<std::uint8_t>(score);
    records[off+13u]=static_cast<std::uint8_t>(score>>8);
}

OriginalCpuMotionContinuation original_cpu_continue_trial_after_contact(
    const OriginalCpuContactResult& previous,
    std::int32_t x,std::int32_t y,std::uint16_t direction,
    std::uint16_t green,bool verified) {
    if(!verified)
        throw std::logic_error(
            "CPU source path 0x40CBC7 not proven after original candidate/map update");
    if(previous.route==OriginalCpuContactRoute::ReturnZeroStop)
        return {OriginalCpuContinuationKind::TerminalStop,std::nullopt};
    if(previous.route==OriginalCpuContactRoute::RequestOriginalLipRandom)
        return {OriginalCpuContinuationKind::RequiresOriginalLipPrng,
                std::nullopt};
    if(previous.state.ball_height==0)
        return {OriginalCpuContinuationKind::RequiresOriginalMappedContact,
                std::nullopt};
    const auto moved=original_cpu_simulated_motion({
        x,y,previous.state.horizontal_force,
        previous.state.ball_height,direction,green});
    return {moved.route==OriginalCpuMotionRoute::RequiresMappedContact ?
            OriginalCpuContinuationKind::RequiresOriginalMappedContact :
            OriginalCpuContinuationKind::AppliedOriginalMotion,moved};
}

} // namespace pixtee
