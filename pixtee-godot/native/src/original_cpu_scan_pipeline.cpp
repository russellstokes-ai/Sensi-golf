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
} // namespace pixtee
