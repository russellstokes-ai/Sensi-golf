#pragma once
#include <cstdint>
#include <vector>
#include "original_cpu_search_grid.hpp"
namespace pixtee {
// Source v1.014 0x40C71B..0x40C77F scanner: original heading candidate
// SCHEDULE, not the complete CPU candidate generation/score evaluation.
struct OriginalCpuHeadingSweep {
    std::vector<std::uint16_t> candidate_headings{};
    std::uint16_t final_ball_heading=0;
    std::uint16_t final_player_heading=0;
    std::uint16_t final_scan_counter=0;
};
OriginalCpuHeadingSweep original_cpu_heading_sweep(
    const OriginalCpuScanGrid& source_grid,std::uint16_t ball_heading);
OriginalCpuHeadingSweep original_cpu_heading_sweep_from_original_table(
    std::uint16_t mode,std::uint8_t band,std::uint16_t ball_heading);
}
