#include "original_cpu_heading_sweep.hpp"
#include <stdexcept>
namespace pixtee {
OriginalCpuHeadingSweep original_cpu_heading_sweep(
    const OriginalCpuScanGrid& g,std::uint16_t input) {
    if(g.table_word_0==0 || g.table_word_0>110)
        throw std::invalid_argument("original CPU sweep count outside playable scanner capacity");
    OriginalCpuHeadingSweep out{};
    out.candidate_headings.reserve(g.table_word_0);
    std::uint32_t heading=static_cast<std::uint32_t>(
        input-g.table_word_1)&0xfffu;
    std::uint16_t counter=g.initial_remainder;
    // Original 0x40C723..0x40C776 executes candidate callback
    // at the leftmost heading then advances by angular step per sample.
    for(std::uint16_t i=0;i<g.table_word_0;++i) {
        out.candidate_headings.push_back(static_cast<std::uint16_t>(heading));
        heading=(heading+g.table_word_2)&0xfffu;
        counter=static_cast<std::uint16_t>(counter-g.spacing);
    }
    out.final_ball_heading=static_cast<std::uint16_t>(heading);
    out.final_player_heading=out.final_ball_heading;
    out.final_scan_counter=counter;
    return out;
}
OriginalCpuHeadingSweep original_cpu_heading_sweep_from_original_table(
    std::uint16_t mode,std::uint8_t band,std::uint16_t heading) {
    return original_cpu_heading_sweep(
        original_cpu_scan_grid_from_original_table(mode,band),heading);
}
}
