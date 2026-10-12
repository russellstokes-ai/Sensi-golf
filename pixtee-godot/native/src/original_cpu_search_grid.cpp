#include "original_cpu_search_grid.hpp"
#include <stdexcept>
namespace pixtee {
OriginalCpuScanGrid original_cpu_scan_grid(
    std::uint16_t mode,std::uint8_t band,
    const std::array<std::uint16_t,4>& w) {
    if(mode<4 || mode>9 || band>3)
        throw std::invalid_argument("source table indices need mode 4..9 and band 0..3");
    if(w[0]==0) // actual x86 would raise divide-error, do not emulate a crash
        throw std::invalid_argument("original CPU scan candidate divisor zero");
    const auto quotient=static_cast<std::uint16_t>(154u/w[0]);
    const auto remainder=static_cast<std::uint16_t>(154u%w[0]);
    return {
        w[0],w[1],w[2],w[3],
        quotient,
        static_cast<std::uint16_t>(154u-remainder)
    };
}

OriginalCpuScanGrid original_cpu_scan_grid_from_original_table(
    std::uint16_t mode, std::uint8_t band) {
    if(mode<4 || mode>9 || band>3)
        throw std::invalid_argument("original CPU table index outside recovered 24-row grid");
    // Numerical parameters extracted from hash-verified GOLFWIN.EXE at
    // 0x41FA11 + (mode-4)*8 + band*48. No original game bytes included.
    // This table is research-only until source-derived data rights reviewed.
    static constexpr std::uint16_t kSourceRows[4][6][4]={
        {{32,512,32,2}, {25,525,42,2}, {25,525,42,2}, {22,517,46,3}, {22,517,46,3}, {20,520,52,3}},
        {{20,520,52,4}, {17,507,58,4}, {64,1024,32,2}, {50,1050,42,2}, {50,1050,42,2}, {45,1035,46,2}},
        {{45,1035,46,2}, {40,1040,52,2}, {40,1040,52,3}, {35,1015,58,3}, {64,2048,64,2}, {50,2100,84,2}},
        {{50,2100,84,2}, {45,2070,92,2}, {45,2070,92,2}, {40,2080,104,2}, {40,2080,104,3}, {35,2030,116,3}}
    };
    const auto &w=kSourceRows[band][mode-4];
    return original_cpu_scan_grid(mode,band,{w[0],w[1],w[2],w[3]});
}
}

